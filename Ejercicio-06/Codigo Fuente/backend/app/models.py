from datetime import datetime
from sqlalchemy import Integer, String, Text, DateTime, ForeignKey, UniqueConstraint, Index
from sqlalchemy.orm import Mapped, mapped_column, relationship
from app.database import Base


class Column(Base):
    __tablename__ = "columns"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    title: Mapped[str] = mapped_column(String(255), nullable=False)
    position: Mapped[int] = mapped_column(Integer, nullable=False, default=0)
    created_at: Mapped[datetime] = mapped_column(DateTime, default=datetime.utcnow)
    updated_at: Mapped[datetime] = mapped_column(
        DateTime, default=datetime.utcnow, onupdate=datetime.utcnow
    )

    cards: Mapped[list["Card"]] = relationship(
        "Card", back_populates="column", cascade="all, delete-orphan"
    )
    card_orders: Mapped[list["CardOrder"]] = relationship(
        "CardOrder", back_populates="column", cascade="all, delete-orphan"
    )


class Card(Base):
    __tablename__ = "cards"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    column_id: Mapped[int] = mapped_column(
        Integer, ForeignKey("columns.id", ondelete="CASCADE"), nullable=False
    )
    title: Mapped[str] = mapped_column(String(255), nullable=False)
    description: Mapped[str | None] = mapped_column(Text, nullable=True)
    created_at: Mapped[datetime] = mapped_column(DateTime, default=datetime.utcnow)
    updated_at: Mapped[datetime] = mapped_column(
        DateTime, default=datetime.utcnow, onupdate=datetime.utcnow
    )

    column: Mapped["Column"] = relationship("Column", back_populates="cards")
    order_entry: Mapped["CardOrder | None"] = relationship(
        "CardOrder", back_populates="card", cascade="all, delete-orphan"
    )


class CardOrder(Base):
    __tablename__ = "card_order"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    column_id: Mapped[int] = mapped_column(
        Integer, ForeignKey("columns.id", ondelete="CASCADE"), nullable=False
    )
    card_id: Mapped[int] = mapped_column(
        Integer, ForeignKey("cards.id", ondelete="CASCADE"), nullable=False
    )
    position: Mapped[int] = mapped_column(Integer, nullable=False, default=0)

    __table_args__ = (
        UniqueConstraint("column_id", "card_id", name="uq_column_card"),
        Index("idx_column_position", "column_id", "position"),
    )

    column: Mapped["Column"] = relationship("Column", back_populates="card_orders")
    card: Mapped["Card"] = relationship("Card", back_populates="order_entry")
