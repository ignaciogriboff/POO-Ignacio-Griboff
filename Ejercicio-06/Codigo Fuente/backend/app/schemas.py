from datetime import datetime
from pydantic import BaseModel, field_validator


# ── Card schemas ──────────────────────────────────────────────────────────────

class CardBase(BaseModel):
    title: str
    description: str | None = None

    @field_validator("title")
    @classmethod
    def title_not_empty(cls, v: str) -> str:
        if not v.strip():
            raise ValueError("title must not be empty")
        return v.strip()


class CardCreate(CardBase):
    column_id: int


class CardUpdate(CardBase):
    pass


class CardRead(CardBase):
    id: int
    column_id: int
    position: int
    created_at: datetime
    updated_at: datetime

    model_config = {"from_attributes": True}


# ── Column schemas ────────────────────────────────────────────────────────────

class ColumnBase(BaseModel):
    title: str

    @field_validator("title")
    @classmethod
    def title_not_empty(cls, v: str) -> str:
        if not v.strip():
            raise ValueError("title must not be empty")
        return v.strip()


class ColumnCreate(ColumnBase):
    pass


class ColumnUpdate(ColumnBase):
    position: int | None = None


class ColumnRead(ColumnBase):
    id: int
    position: int
    cards: list[CardRead] = []
    created_at: datetime
    updated_at: datetime

    model_config = {"from_attributes": True}


# ── Move / Reorder schemas ────────────────────────────────────────────────────

class CardMove(BaseModel):
    to_column_id: int
    to_position: int


class CardReorder(BaseModel):
    card_id: int
    to_position: int
