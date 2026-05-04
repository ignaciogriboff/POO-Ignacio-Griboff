import paramiko
import time

VPS_HOST = "173.212.234.190"
VPS_USER = "ignacio"
VPS_PASS = "12345"
REMOTE_DIR = "/home/ignacio/kanban/backend"

ENV_CONTENT = (
    "DB_HOST=localhost\n"
    "DB_PORT=3306\n"
    "DB_USER=kanban_user\n"
    "DB_PASSWORD=Djg01248\n"
    "DB_NAME=kanban_db\n"
    "\n"
    "BASIC_AUTH_USER=admin\n"
    "BASIC_AUTH_PASS=admin123\n"
)

def run(client, cmd):
    _, out, _ = client.exec_command(cmd, get_pty=True)
    o = out.read().decode(errors="replace").strip()
    code = out.channel.recv_exit_status()
    if o:
        for line in o.splitlines():
            print("   ", line)
    return code


c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect(VPS_HOST, username=VPS_USER, password=VPS_PASS,
          look_for_keys=False, allow_agent=False)

print("[1] Killing existing uvicorn on port 8000 ...")
run(c, "echo 12345 | sudo -S pkill -f 'uvicorn app' 2>/dev/null; true")
time.sleep(2)
run(c, "ss -tlnp | grep 8000 || echo 'port 8000 is now free'")

print("[2] Creating kanban_user in MySQL ...")
sql = (
    "CREATE USER IF NOT EXISTS 'kanban_user'@'%' IDENTIFIED BY 'Djg01248';"
    "GRANT ALL PRIVILEGES ON kanban_db.* TO 'kanban_user'@'%';"
    "FLUSH PRIVILEGES;"
)
run(c, "mysql -u root -pDjg001248 -e \"" + sql + "\" 2>/dev/null")

print("[3] Updating .env ...")
sftp = c.open_sftp()
with sftp.open(REMOTE_DIR + "/.env", "w") as f:
    f.write(ENV_CONTENT)
sftp.close()

print("[4] Starting our backend on port 8000 ...")
run(c, (
    "cd " + REMOTE_DIR +
    " && nohup .venv/bin/uvicorn app.main:app"
    " --host 0.0.0.0 --port 8000"
    " > /home/ignacio/kanban/kanban.log 2>&1 &"
))
time.sleep(5)

print("[5] Health check ...")
run(c, "curl -s -u admin:admin123 http://localhost:8000/columns")

print("[6] Server log ...")
run(c, "tail -25 /home/ignacio/kanban/kanban.log")

c.close()
print()
print("=== Done ===")
print("API:  http://173.212.234.190:8000")
print("Docs: http://173.212.234.190:8000/docs")
