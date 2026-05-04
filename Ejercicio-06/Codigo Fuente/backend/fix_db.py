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

# Fix 1: Create kanban_user@localhost AND ensure kanban_user@% exists
print("[1] Fixing kanban_user MySQL permissions ...")
sql = (
    "CREATE USER IF NOT EXISTS 'kanban_user'@'localhost' IDENTIFIED BY 'Djg01248';"
    "CREATE USER IF NOT EXISTS 'kanban_user'@'%' IDENTIFIED BY 'Djg01248';"
    "GRANT ALL PRIVILEGES ON kanban_db.* TO 'kanban_user'@'localhost';"
    "GRANT ALL PRIVILEGES ON kanban_db.* TO 'kanban_user'@'%';"
    "FLUSH PRIVILEGES;"
)
run(f'mysql -u root -pDjg001248 -e "{sql}" 2>&1')

# Verify user created
run("mysql -u root -pDjg001248 -e \"SELECT User,Host FROM mysql.user WHERE User='kanban_user';\" 2>/dev/null")

# Fix 2: Test kanban_user can connect
print("[2] Testing kanban_user login ...")
run("mysql -u kanban_user -pDjg01248 -h 127.0.0.1 kanban_db -e 'SHOW TABLES;' 2>&1")

# Fix 3: Update .env to use 127.0.0.1 (forces TCP, avoids socket auth issues)
print("[3] Updating .env: DB_HOST=127.0.0.1 ...")
env = (
    "DB_HOST=127.0.0.1\n"
    "DB_PORT=3306\n"
    "DB_USER=kanban_user\n"
    "DB_PASSWORD=Djg01248\n"
    "DB_NAME=kanban_db\n"
    "\n"
    "BASIC_AUTH_USER=admin\n"
    "BASIC_AUTH_PASS=admin123\n"
)
sftp = c.open_sftp()
with sftp.open(REMOTE_DIR + "/.env", "w") as f:
    f.write(env)
sftp.close()

# Fix 4: Restart service
print("[4] Restarting kanban-ignacio service ...")
run(f"{SUDO} systemctl restart kanban-ignacio.service 2>&1")
time.sleep(5)
run(f"{SUDO} systemctl status kanban-ignacio.service --no-pager -l 2>/dev/null | head -20")

# Fix 5: Health check
print("[5] Health check ...")
result = run("curl -s -u admin:admin123 http://localhost:8000/columns 2>&1")

if result in ("[]", "") or result.startswith("["):
    print("   SUCCESS! API is up and returning:", result or "[] (empty board)")
else:
    print("   Response:", result)
    print("[5b] Checking logs ...")
    run(f"{SUDO} journalctl -u kanban-ignacio.service -n 15 --no-pager 2>/dev/null")

c.close()
print()
print("=== API ready at http://173.212.234.190:8000 ===")
print("=== Docs:       http://173.212.234.190:8000/docs ===")
