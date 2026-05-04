# Kanban Qt Client — README

## Requirements

- Qt 6.11.0 with modules: **Widgets**, **Network**, **WebSockets**
- Qt Creator 6.11.0 (or any compatible version)
- qmake build system

---

## 1. Configure the client

```bash
cp config.example.json config.json
```

Edit `config.json`:

```json
{
    "api_base_url": "http://173.212.234.190:8000",
    "basic_user":   "admin",
    "basic_pass":   "your_password"
}
```

Place `config.json` in the **same directory as the compiled binary** (the build output folder).

---

## 2. Open in Qt Creator

1. Open **Qt Creator 6.11.0**
2. File → Open File or Project → select `KanbanClient.pro`
3. Choose a Qt 6.11.0 kit
4. Build → Build Project (Ctrl+B)
5. Run (Ctrl+R)

---

## 3. Build from command line (qmake)

```bash
cd qt-client
qmake KanbanClient.pro
make          # Linux/macOS
# or on Windows with MinGW:
mingw32-make
# or with MSVC:
nmake
```

---

## 4. UI usage

| Action | How |
|---|---|
| Add column | Toolbar "**+ Add Column**" button |
| Delete column | "**Del Col**" button in the column |
| Add card | "**Add Card**" button in a column |
| Edit card | Double-click card OR "**Edit**" button |
| Delete card | Select card → "**Del Card**" |
| Move card left/right | Select card → **◀ / ▶** |
| Reorder card up/down | Select card → **↑ / ↓** |
| Reorder by drag-drop | Drag card within the list |
| Full refresh | Toolbar "**⟳ Refresh**" |

Real-time updates arrive automatically via WebSocket. If a conflict is detected the board does a full refresh (server wins).

---

## 5. WebSocket behaviour

- Connects to `ws://<host>:8000/ws?u=USER&p=PASS` on startup.
- On connect: receives `snapshot` → triggers full GET /columns refresh.
- Subsequent events update the UI incrementally.
- If WS disconnects, the status bar shows a warning; manual refresh is available.
