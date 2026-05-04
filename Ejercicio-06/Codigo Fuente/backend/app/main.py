from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from app.routes import router
from app.database import engine, Base

# Create tables if they don't exist (idempotent; schema.sql is preferred for production)
# Wrapped in try/except so the server still starts if DB is temporarily unreachable at boot
try:
    Base.metadata.create_all(bind=engine)
except Exception as e:
    import logging
    logging.warning(f"Could not run create_all at startup (DB may not be reachable): {e}")

app = FastAPI(
    title="Kanban Collaborative Board",
    description="REST + WebSocket API for collaborative Kanban board",
    version="1.0.0",
)

# CORS — allow all origins for desktop client compatibility
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(router)
