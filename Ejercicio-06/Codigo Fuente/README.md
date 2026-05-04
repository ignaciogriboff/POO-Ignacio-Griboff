# Kanban Collaborative Board

A full-stack collaborative Kanban board with:

- **Backend**: Python FastAPI + WebSocket, MySQL persistence
- **Client**: Qt 6.11.0 C++ desktop app

---

## Structure

```
/backend            Python FastAPI backend
  /app
    main.py         App entry point, CORS, route registration
    config.py       Settings from .env
    database.py     SQLAlchemy engine + session
    models.py       ORM models (Column, Card, CardOrder)
    schemas.py      Pydantic request/response schemas
    crud.py         Database operations
    auth.py         HTTP Basic Auth + WebSocket auth
    websocket_manager.py  Broadcast manager
    routes.py       All REST endpoints + /ws WebSocket
  requirements.txt
  schema.sql        DB creation script
  .env.example
  README.md

/qt-client          Qt 6.11.0 C++ desktop client
  KanbanClient.pro  qmake project file
  main.cpp
  mainwindow.h/cpp  Main board window
  api_client.h/cpp  REST client (QNetworkAccessManager)
  ws_client.h/cpp   WebSocket client (QWebSocket)
  models.h          Column/Card structs
  /dialogs
    columndialog.h/cpp
    carddialog.h/cpp
  config.example.json
  README.md
```

---

## Quick start

### Backend

```bash
cd backend
cp .env.example .env        # edit with real credentials
mysql -h 173.212.234.190 -u root -p < schema.sql
python -m venv .venv
.venv\Scripts\activate      # Windows
source .venv/bin/activate   # Linux/macOS
pip install -r requirements.txt
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

### Qt Client

```bash
cd qt-client
cp config.example.json config.json   # edit credentials
# Open KanbanClient.pro in Qt Creator 6.11.0 and build
```

See each folder's README for full details.
