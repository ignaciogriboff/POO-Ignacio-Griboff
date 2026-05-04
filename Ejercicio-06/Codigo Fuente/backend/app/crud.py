"""
CRUD operations for Kanban.

Card ordering uses card_order.position (0-based, consecutive integers).
"""
from datetime import datetime
from sqlalchemy.orm import Session
from sqlalchemy import select
from app import models, schemas


# ── Helpers ───────────────────────────────────────────────────────────────────

def _reindex_column(db: Session, column_id: int) -> None:
    """Ensure positions in card_order for a column are 0-based consecutive."""
    entries = (
        db.execute(
            select(models.CardOrder)
            .where(models.CardOrder.column_id == column_id)
            .order_by(models.CardOrder.position)
        )
        .scalars()
        .all()
    )
    for i, entry in enumerate(entries):
        entry.position = i
    db.flush()


def _build_column_read(col: models.Column) -> dict:
    """Build a dict matching ColumnRead including ordered cards."""
    order_map: dict[int, int] = {
        co.card_id: co.position for co in col.card_orders
    }
    cards_sorted = sorted(col.cards, key=lambda c: order_map.get(c.id, 9999))
    cards_out = []
    for card in cards_sorted:
        cards_out.append({
            "id": card.id,
            "column_id": card.column_id,
            "title": card.title,
            "description": card.description,
            "position": order_map.get(card.id, 0),
            "created_at": card.created_at,
            "updated_at": card.updated_at,
        })
    return {
        "id": col.id,
        "title": col.title,
        "position": col.position,
        "cards": cards_out,
        "created_at": col.created_at,
        "updated_at": col.updated_at,
    }


# ── Columns ───────────────────────────────────────────────────────────────────

def get_columns(db: Session) -> list[dict]:
    cols = (
        db.execute(select(models.Column).order_by(models.Column.position))
        .scalars()
        .all()
    )
    return [_build_column_read(c) for c in cols]


def create_column(db: Session, data: schemas.ColumnCreate) -> models.Column:
    max_pos = db.execute(
        select(models.Column.position).order_by(models.Column.position.desc())
    ).first()
    pos = (max_pos[0] + 1) if max_pos else 0
    col = models.Column(title=data.title, position=pos, created_at=datetime.utcnow(), updated_at=datetime.utcnow())
    db.add(col)
    db.commit()
    db.refresh(col)
    return col


def update_column(db: Session, col_id: int, data: schemas.ColumnUpdate) -> models.Column | None:
    col = db.get(models.Column, col_id)
    if not col:
        return None
    col.title = data.title
    if data.position is not None:
        col.position = data.position
    col.updated_at = datetime.utcnow()
    db.commit()
    db.refresh(col)
    return col


def delete_column(db: Session, col_id: int) -> bool:
    col = db.get(models.Column, col_id)
    if not col:
        return False
    db.delete(col)
    db.commit()
    return True


# ── Cards ─────────────────────────────────────────────────────────────────────

def create_card(db: Session, data: schemas.CardCreate) -> models.Card | None:
    col = db.get(models.Column, data.column_id)
    if not col:
        return None
    card = models.Card(
        column_id=data.column_id,
        title=data.title,
        description=data.description,
        created_at=datetime.utcnow(),
        updated_at=datetime.utcnow(),
    )
    db.add(card)
    db.flush()  # get card.id

    # Determine next position
    last = db.execute(
        select(models.CardOrder.position)
        .where(models.CardOrder.column_id == data.column_id)
        .order_by(models.CardOrder.position.desc())
    ).first()
    pos = (last[0] + 1) if last else 0

    order = models.CardOrder(column_id=data.column_id, card_id=card.id, position=pos)
    db.add(order)
    db.commit()
    db.refresh(card)
    return card


def get_card_position(db: Session, card_id: int, column_id: int) -> int:
    entry = db.execute(
        select(models.CardOrder.position)
        .where(models.CardOrder.card_id == card_id, models.CardOrder.column_id == column_id)
    ).first()
    return entry[0] if entry else 0


def update_card(db: Session, card_id: int, data: schemas.CardUpdate) -> models.Card | None:
    card = db.get(models.Card, card_id)
    if not card:
        return None
    card.title = data.title
    card.description = data.description
    card.updated_at = datetime.utcnow()
    db.commit()
    db.refresh(card)
    return card


def delete_card(db: Session, card_id: int) -> bool:
    card = db.get(models.Card, card_id)
    if not card:
        return False
    db.delete(card)
    db.commit()
    return True


def move_card(db: Session, card_id: int, data: schemas.CardMove) -> models.Card | None:
    card = db.get(models.Card, card_id)
    if not card:
        return None
    old_column_id = card.column_id

    # Remove from old order
    old_order = db.execute(
        select(models.CardOrder)
        .where(models.CardOrder.card_id == card_id)
    ).scalar_one_or_none()
    if old_order:
        db.delete(old_order)
        db.flush()

    # Move card to new column
    card.column_id = data.to_column_id
    card.updated_at = datetime.utcnow()
    db.flush()

    # Shift existing entries in target column to make room
    to_pos = data.to_position
    entries = (
        db.execute(
            select(models.CardOrder)
            .where(models.CardOrder.column_id == data.to_column_id)
            .order_by(models.CardOrder.position)
        )
        .scalars()
        .all()
    )
    for e in entries:
        if e.position >= to_pos:
            e.position += 1
    db.flush()

    new_order = models.CardOrder(
        column_id=data.to_column_id, card_id=card_id, position=to_pos
    )
    db.add(new_order)
    db.flush()

    # Reindex both columns
    _reindex_column(db, old_column_id)
    _reindex_column(db, data.to_column_id)

    db.commit()
    db.refresh(card)
    return card


def reorder_card(db: Session, column_id: int, data: schemas.CardReorder) -> bool:
    entry = db.execute(
        select(models.CardOrder)
        .where(
            models.CardOrder.column_id == column_id,
            models.CardOrder.card_id == data.card_id,
        )
    ).scalar_one_or_none()
    if not entry:
        return False

    old_pos = entry.position
    to_pos = data.to_position

    entries = (
        db.execute(
            select(models.CardOrder)
            .where(models.CardOrder.column_id == column_id)
            .order_by(models.CardOrder.position)
        )
        .scalars()
        .all()
    )

    # Remove entry, re-insert at new position
    entries_without = [e for e in entries if e.card_id != data.card_id]
    entries_without.insert(to_pos, entry)
    for i, e in enumerate(entries_without):
        e.position = i

    db.commit()
    return True
