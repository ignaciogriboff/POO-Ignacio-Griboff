import paramiko
import time

c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("173.212.234.190", username="ignacio", password="12345",
          look_for_keys=False, allow_agent=False)

REMOTE_DIR = "/home/ignacio/kanban/backend"
SUDO = "echo 12345 | sudo -S"

def run(cmd, show=True):
    _, out, _ = c.exec_command(cmd, get_pty=True)
    o = out.read().decode(errors="replace").strip()
    out.channel.recv_exit_status()
    if show and o:
        for line in o.splitlines():
            print("   ", line)
    return o

# 1. Stop + disable the old kanban systemd service
print("[1] Stopping kanban-api.service ...")
run(f"{SUDO} systemctl stop kanban-api.service 2>/dev/null")
run(f"{SUDO} systemctl disable kanban-api.service 2>/dev/null")
time.sleep(2)
run("ss -tlnp | grep 8000 || echo 'port 8000 free'")

# 2. Write our own systemd unit
print("[2] Installing our systemd service ...")
unit = (
    "[Unit]\\n"
    "Description=Kanban FastAPI Backend (ignacio)\\n"
    "After=network.target\\n"
    "\\n"
    "[Service]\\n"
    "User=ignacio\\n"
    f"WorkingDirectory={REMOTE_DIR}\\n"
    f"EnvironmentFile={REMOTE_DIR}/.env\\n"
    f"ExecStart={REMOTE_DIR}/.venv/bin/uvicorn app.main:app --host 0.0.0.0 --port 8000\\n"
    "Restart=always\\n"
    "RestartSec=3\\n"
    "\\n"
    "[Install]\\n"
    "WantedBy=multi-user.target\\n"
)
run(f"{SUDO} bash -c 'printf \"{unit}\" > /etc/systemd/system/kanban-ignacio.service'")
run(f"{SUDO} systemctl daemon-reload")
run(f"{SUDO} systemctl enable kanban-ignacio.service")
run(f"{SUDO} systemctl start kanban-ignacio.service")
time.sleep(5)

# 3. Check status
print("[3] Service status ...")
run(f"{SUDO} systemctl status kanban-ignacio.service --no-pager -l")

# 4. Health check
print("[4] Health check ...")
result = run("curl -s -u admin:admin123 http://localhost:8000/columns")
if "[]" in result or "[{" in result or result == "[]":
    print("   SUCCESS - API responding correctly!")
elif "detail" in result:
    print("   Response:", result)

# 5. Show journal logs
print("[5] Recent logs ...")
run(f"{SUDO} journalctl -u kanban-ignacio.service -n 30 --no-pager")

c.close()
print()
print("=== Complete ===")
print("API:  http://173.212.234.190:8000")
print("Docs: http://173.212.234.190:8000/docs")
