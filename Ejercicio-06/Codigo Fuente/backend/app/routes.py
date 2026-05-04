"""
All REST + WebSocket routes for the Kanban API.
"""
import json
from fastapi import APIRouter, Depends, HTTPException, WebSocket, WebSocketDisconnect, status, Query
from sqlalchemy.orm import Session

from app import crud, schemas
from app.auth import verify_basic_auth, check_ws_auth
from app.database import get_db
from app.websocket_manager import manager

router = APIRouter()


# ─────────────────────────────────────────────────────────────────────────────
# Columns
# ─────────────────────────────────────────────────────────────────────────────

@router.get("/columns", response_model=list[schemas.ColumnRead])
def list_columns(
    db: Session = Depends(get_db),
    _: str = Depends(verify_basic_auth),
):
    return crud.get_columns(db)


@router.post("/columns", response_model=schemas.ColumnRead, status_code=status.HTTP_201_CREATED)
async def create_column(
    data: schemas.ColumnCreate,
    db: Session = Depends(get_db),
    _: str = Depends(verify_basic_auth),
):
    col = crud.create_column(db, data)
    col_data = crud._build_column_read(col)
    await manager.broadcast("column_created", col_data)
    return col_data


@router.put("/columns/{col_id}", response_model=schemas.ColumnRead)
async def update_column(
    col_id: int,
    data: schemas.ColumnUpdate,
    db: Session = Depends(get_db),
    _: str = Depends(verify_basic_auth),
):
    col = crud.update_column(db, col_id, data)
    if not col:
        raise HTTPException(status_code=404, detail="Column not found")
    col_data = crud._build_column_read(col)
    await manager.broadcast("column_updated", col_data)
    return col_data


@router.delete("/columns/{col_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_column(
    col_id: int,
    db: Session = Depends(get_db),
    _: str = Depends(verify_basic_auth),
):
    ok = crud.delete_column(db, col_id)
    if not ok:
        raise HTTPException(status_code=404, detail="Column not found")
    await manager.broadcast("column_deleted", {"id": col_id})


# ─────────────────────────────────────────────────────────────────────────────
# Cards
# ─────────────────────────────────────────────────────────────────────────────

@router.post("/cards", response_model=schemas.CardRead, status_code=status.HTTP_201_CREATED)
async def create_card(
    data: schemas.CardCreate,
    db: Session = Depends(get_db),
    _: str = Depends(verify_basic_auth),
):
    card = crud.create_card(db, data)
    if not card:
        raise HTTPException(status_code=404, detail="Column not found")
    pos = crud.get_card_position(db, card.id, card.column_id)
    card_data = {
        "id": card.id,
        "column_id": card.column_id,
        "title": card.title,
        "description": card.description,
        "position": pos,
        "created_at": card.created_at.isoformat(),
        "updated_at": card.updated_at.isoformat(),
    }
    await manager.broadcast("card_created", card_data)
    return {**card_data, "created_at": card.created_at, "updated_at": card.updated_at}


@router.put("/cards/{card_id}", response_model=schemas.CardRead)
async def update_card(
    card_id: int,
    data: schemas.CardUpdate,
    db: Session = Depends(get_db),
    _: str = Depends(verify_basic_auth),
):
    card = crud.update_card(db, card_id, data)
    if not card:
        raise HTTPException(status_code=404, detail="Card not found")
    pos = crud.get_card_position(db, card.id, card.column_id)
    card_data = {
        "id": card.id,
        "column_id": card.column_id,
        "title": card.title,
        "description": card.description,
        "position": pos,
        "created_at": card.created_at.isoformat(),
        "updated_at": card.updated_at.isoformat(),
    }
    await manager.broadcast("card_updated", card_data)
    return {**card_data, "created_at": card.created_at, "updated_at": card.updated_at}


@router.delete("/cards/{card_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_card(
    card_id: int,
    db: Session = Depends(get_db),
    _: str = Depends(verify_basic_auth),
):
    ok = crud.delete_card(db, card_id)
    if not ok:
        raise HTTPException(status_code=404, detail="Card not found")
    await manager.broadcast("card_deleted", {"id": card_id})


@router.post("/cards/{card_id}/move", response_model=schemas.CardRead)
async def move_card(
    card_id: int,
    data: schemas.CardMove,
    db: Session = Depends(get_db),
    _: str = Depends(verify_basic_auth),
):
    card = crud.move_card(db, card_id, data)
    if not card:
        raise HTTPException(status_code=404, detail="Card not found")
    pos = crud.get_card_position(db, card.id, card.column_id)
    card_data = {
        "id": card.id,
        "column_id": card.column_id,
        "title": card.title,
        "description": card.description,
        "position": pos,
        "created_at": card.created_at.isoformat(),
        "updated_at": card.updated_at.isoformat(),
    }
    await manager.broadcast("card_moved", card_data)
    return {**card_data, "created_at": card.created_at, "updated_at": card.updated_at}


@router.post("/columns/{column_id}/reorder", status_code=status.HTTP_200_OK)
async def reorder_card(
    column_id: int,
    data: schemas.CardReorder,
    db: Session = Depends(get_db),
    _: str = Depends(verify_basic_auth),
):
    ok = crud.reorder_card(db, column_id, data)
    if not ok:
        raise HTTPException(status_code=404, detail="Card not found in column")
    await manager.broadcast("card_reordered", {
        "column_id": column_id,
        "card_id": data.card_id,
        "to_position": data.to_position,
    })
    return {"ok": True}


# ─────────────────────────────────────────────────────────────────────────────
# WebSocket
# ─────────────────────────────────────────────────────────────────────────────

@router.websocket("/ws")
async def websocket_endpoint(
    websocket: WebSocket,
    u: str = Query(default=""),
    p: str = Query(default=""),
    db: Session = Depends(get_db),
):
    """
    WebSocket endpoint.
    Authentication via query string: /ws?u=USER&p=PASS
    On connect: sends snapshot event with full board state.
    On auth failure: closes with policy_violation (1008).
    """
    if not check_ws_auth(u, p):
        await websocket.close(code=1008)  # policy violation
        return

    await manager.connect(websocket)
    try:
        # Send snapshot immediately
        columns = crud.get_columns(db)

        def serialize(obj):
            if hasattr(obj, "isoformat"):
                return obj.isoformat()
            return str(obj)

        snapshot_payload = json.loads(json.dumps(columns, default=serialize))
        await websocket.send_text(json.dumps({"type": "snapshot", "data": {"columns": snapshot_payload}}))

        # Keep connection alive; listen for pings/client messages
        while True:
            text = await websocket.receive_text()
            # Clients may send pings; just ignore or echo
            _ = text
    except WebSocketDisconnect:
        pass
    finally:
        await manager.disconnect(websocket)
