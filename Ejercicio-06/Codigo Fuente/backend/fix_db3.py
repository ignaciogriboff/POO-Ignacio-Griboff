import paramiko
import time

c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("173.212.234.190", username="ignacio", password="12345",
          look_for_keys=False, allow_agent=False)

REMOTE_DIR = "/home/ignacio/kanban/backend"

def run_notty(cmd, sudo_pass=None):
    """Run without PTY. For sudo, pass password via stdin."""
    transport = c.get_transport()
    ch = transport.open_session()
    ch.exec_command(cmd)
    if sudo_pass:
        ch.send(sudo_pass + "\n")
    ch.shutdown_write()
    out = b""
    err = b""
    while True:
        if ch.recv_ready():
            out += ch.recv(4096)
        if ch.recv_stderr_ready():
            err += ch.recv_stderr(4096)
        if ch.exit_status_ready():
            break
        time.sleep(0.1)
    # Drain
    while ch.recv_ready():
        out += ch.recv(4096)
    while ch.recv_stderr_ready():
        err += ch.recv_stderr(4096)
    o = out.decode(errors="replace").strip()
    e = err.decode(errors="replace").strip()
    if o:
        for line in o.splitlines():
            print("   OUT:", line)
    if e and "warning" not in e.lower() and "password" not in e.lower():
        for line in e.splitlines():
            print("   ERR:", line)
    return o


# Write the fix script to VPS
print("[1] Writing fix script to VPS ...")
fix_script = """#!/bin/bash
mysql <<'EOSQL'
ALTER USER IF EXISTS 'kanban_user'@'localhost' IDENTIFIED BY 'Djg01248';
ALTER USER IF EXISTS 'kanban_user'@'%' IDENTIFIED BY 'Djg01248';
CREATE USER IF NOT EXISTS 'kanban_user'@'localhost' IDENTIFIED BY 'Djg01248';
CREATE USER IF NOT EXISTS 'kanban_user'@'%' IDENTIFIED BY 'Djg01248';
GRANT ALL PRIVILEGES ON kanban_db.* TO 'kanban_user'@'localhost';
GRANT ALL PRIVILEGES ON kanban_db.* TO 'kanban_user'@'%';
FLUSH PRIVILEGES;
SELECT User, Host FROM mysql.user WHERE User='kanban_user';
EOSQL
"""
sftp = c.open_sftp()
with sftp.open("/tmp/fix_kanban.sh", "w") as f:
    f.write(fix_script)
sftp.close()

# Run without PTY using sudo -S
print("[2] Running sudo fix script ...")
run_notty("chmod +x /tmp/fix_kanban.sh && sudo -S /tmp/fix_kanban.sh", sudo_pass="12345")

# Test kanban_user connection
print("[3] Testing kanban_user@127.0.0.1 ...")
run_notty("mysql -u kanban_user -pDjg01248 -h 127.0.0.1 kanban_db -e 'SHOW TABLES;'")

# Restart service
print("[4] Restarting service ...")
run_notty("sudo -S systemctl restart kanban-ignacio.service", sudo_pass="12345")
time.sleep(5)

# Health check
print("[5] Health check ...")
result = run_notty("curl -s -u admin:admin123 http://localhost:8000/columns")

# Show status
run_notty("sudo -S journalctl -u kanban-ignacio.service -n 8 --no-pager", sudo_pass="12345")

run_notty("rm -f /tmp/fix_kanban.sh")
c.close()

print()
print("=== Done ===")
print("API:  http://173.212.234.190:8000")
print("Docs: http://173.212.234.190:8000/docs")
