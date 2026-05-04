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

# 1. Use sudo mysql (no password needed, uses auth_socket)
print("[1] Creating kanban_user via sudo mysql ...")
sql_cmds = [
    "DROP USER IF EXISTS 'kanban_user'@'localhost';",
    "DROP USER IF EXISTS 'kanban_user'@'%';",
    "CREATE USER 'kanban_user'@'localhost' IDENTIFIED BY 'Djg01248';",
    "CREATE USER 'kanban_user'@'%' IDENTIFIED BY 'Djg01248';",
    "GRANT ALL PRIVILEGES ON kanban_db.* TO 'kanban_user'@'localhost';",
    "GRANT ALL PRIVILEGES ON kanban_db.* TO 'kanban_user'@'%';",
    "FLUSH PRIVILEGES;",
]
# Write SQL to a temp file and execute
sql_content = "\n".join(sql_cmds)
sftp = c.open_sftp()
with sftp.open("/tmp/create_user.sql", "w") as f:
    f.write(sql_content)
sftp.close()

run(f"{SUDO} mysql < /tmp/create_user.sql 2>&1")
run(f"{SUDO} mysql -e \"SELECT User,Host,plugin FROM mysql.user WHERE User='kanban_user';\" 2>&1")

# 2. Test kanban_user login
print("[2] Testing kanban_user login via 127.0.0.1 ...")
run("mysql -u kanban_user -pDjg01248 -h 127.0.0.1 kanban_db -e 'SHOW TABLES;' 2>&1")

# 3. Apply schema if needed (using sudo mysql)
print("[3] Applying schema.sql via sudo mysql ...")
run(f"{SUDO} mysql kanban_db < {REMOTE_DIR}/schema.sql 2>&1")
run(f"{SUDO} mysql kanban_db -e 'SHOW TABLES;' 2>&1")

# 4. Update .env to use 127.0.0.1
print("[4] Updating .env ...")
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
sftp2 = c.open_sftp()
with sftp2.open(REMOTE_DIR + "/.env", "w") as f:
    f.write(env)
sftp2.close()

# 5. Restart service
print("[5] Restarting service ...")
run(f"{SUDO} systemctl restart kanban-ignacio.service 2>/dev/null")
time.sleep(5)
run(f"{SUDO} journalctl -u kanban-ignacio.service -n 10 --no-pager 2>/dev/null")

# 6. Final health check
print("[6] Final health check ...")
result = run("curl -s -u admin:admin123 http://localhost:8000/columns 2>&1")
if result.startswith("["):
    print(f"   SUCCESS! Response: {result}")
else:
    print(f"   Response: {result}")

# Cleanup
run("rm -f /tmp/create_user.sql")
c.close()
print()
print("API:  http://173.212.234.190:8000")
print("Docs: http://173.212.234.190:8000/docs")
