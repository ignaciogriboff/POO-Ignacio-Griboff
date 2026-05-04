# Kanban Backend — README

## Requirements

- Python 3.10+
- MySQL 8.x running on `173.212.234.190`

---

## 1. Initialize the database

```bash
# Connect as root and run the schema
mysql -h 173.212.234.190 -u root -p < schema.sql
```

To create a dedicated user (run inside MySQL):

```sql
CREATE USER IF NOT EXISTS 'kanban_user'@'%' IDENTIFIED BY 'KanbanPass2024!';
GRANT ALL PRIVILEGES ON kanban_db.* TO 'kanban_user'@'%';
FLUSH PRIVILEGES;
```

---

## 2. Configure environment

```bash
cp .env.example .env
# Edit .env with your real DB password and desired API credentials
```

`.env` variables:

| Variable | Description |
|---|---|
| `DB_HOST` | MySQL host (default: 173.212.234.190) |
| `DB_PORT` | MySQL port (default: 3306) |
| `DB_USER` | MySQL username |
| `DB_PASSWORD` | MySQL password |
| `DB_NAME` | Database name (default: kanban_db) |
| `BASIC_AUTH_USER` | API username |
| `BASIC_AUTH_PASS` | API password |

---

## 3. Install dependencies

```bash
python -m venv .venv
# Windows:
.venv\Scripts\activate
# Linux/macOS:
source .venv/bin/activate

pip install -r requirements.txt
```

---

## 4. Run the backend

```bash
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

API available at: http://173.212.234.190:8000  
Interactive docs: http://173.212.234.190:8000/docs

---

## 5. Endpoints summary

| Method | Path | Description |
|---|---|---|
| GET | `/columns` | List columns with ordered cards |
| POST | `/columns` | Create column |
| PUT | `/columns/{id}` | Update column title/position |
| DELETE | `/columns/{id}` | Delete column (cascades cards) |
| POST | `/cards` | Create card |
| PUT | `/cards/{id}` | Update card |
| DELETE | `/cards/{id}` | Delete card |
| POST | `/cards/{id}/move` | Move card to another column |
| POST | `/columns/{id}/reorder` | Reorder card within column |
| WS | `/ws?u=USER&p=PASS` | WebSocket real-time channel |

All REST endpoints require **HTTP Basic Auth**.

---

## 6. WebSocket events

Connect to `ws://173.212.234.190:8000/ws?u=USER&p=PASS`

On connect, the server sends a `snapshot` event:

```json
{ "type": "snapshot", "data": { "columns": [...] } }
```

Other event types: `column_created`, `column_updated`, `column_deleted`,  
`card_created`, `card_updated`, `card_deleted`, `card_moved`, `card_reordered`

---

## 7. systemd service (optional)

Create `/etc/systemd/system/kanban.service`:

```ini
[Unit]
Description=Kanban FastAPI Backend
After=network.target

[Service]
User=www-data
WorkingDirectory=/opt/kanban/backend
EnvironmentFile=/opt/kanban/backend/.env
ExecStart=/opt/kanban/backend/.venv/bin/uvicorn app.main:app --host 0.0.0.0 --port 8000
Restart=always

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable kanban
sudo systemctl start kanban
```
