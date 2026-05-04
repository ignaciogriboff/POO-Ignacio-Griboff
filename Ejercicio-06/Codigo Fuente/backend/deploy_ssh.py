"""
deploy_ssh.py - Deploy backend to VPS using paramiko (password-based SSH/SFTP).
"""
import os
import sys
import paramiko
import stat

VPS_HOST = "173.212.234.190"
VPS_PORT = 22
VPS_USER = "ignacio"
VPS_PASS = "12345"
REMOTE_DIR = "/home/ignacio/kanban/backend"
LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))

ENV_CONTENT = """DB_HOST=localhost
DB_PORT=3306
DB_USER=root
DB_PASSWORD=Djg001248
DB_NAME=kanban_db

BASIC_AUTH_USER=admin
BASIC_AUTH_PASS=admin123
"""

def ssh_exec(client, cmd, desc=""):
    if desc:
        print(f"  [{desc}]")
    print(f"  $ {cmd}")
    stdin, stdout, stderr = client.exec_command(cmd, get_pty=True)
    out = stdout.read().decode(errors="replace")
    err = stderr.read().decode(errors="replace")
    code = stdout.channel.recv_exit_status()
    if out.strip():
        print(f"    {out.strip()}")
    if err.strip() and code != 0:
        print(f"    ERR: {err.strip()}")
    return code


def sftp_upload_dir(sftp, local_path, remote_path):
    """Recursively upload a local directory to remote."""
    try:
        sftp.mkdir(remote_path)
    except OSError:
        pass  # already exists
    for item in os.listdir(local_path):
        if item in {".venv", "__pycache__", ".git", "*.pyc"}:
            continue
        local_item = os.path.join(local_path, item)
        remote_item = remote_path + "/" + item
        if os.path.isdir(local_item):
            sftp_upload_dir(sftp, local_item, remote_item)
        else:
            print(f"    upload: {local_item} -> {remote_item}")
            sftp.put(local_item, remote_item)


def main():
    print("=" * 50)
    print("  Kanban Backend — VPS Deploy")
    print(f"  Target: {VPS_USER}@{VPS_HOST}:{REMOTE_DIR}")
    print("=" * 50)

    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    print(f"\n[1/7] Connecting to {VPS_HOST}:{VPS_PORT} ...")
    try:
        client.connect(VPS_HOST, port=VPS_PORT, username=VPS_USER, password=VPS_PASS,
                       timeout=15, look_for_keys=False, allow_agent=False)
    except Exception as e:
        print(f"  ERROR: Could not connect: {e}")
        sys.exit(1)
    print("  Connected!")

    # Create remote dirs
    print("\n[2/7] Creating remote directory structure ...")
    ssh_exec(client, f"mkdir -p {REMOTE_DIR}/app")

    # Upload files via SFTP
    print("\n[3/7] Uploading backend files ...")
    sftp = client.open_sftp()

    # Upload app/ directory
    sftp_upload_dir(sftp, os.path.join(LOCAL_DIR, "app"), REMOTE_DIR + "/app")

    # Upload top-level files
    for fname in ["requirements.txt", "schema.sql"]:
        local_f = os.path.join(LOCAL_DIR, fname)
        if os.path.exists(local_f):
            remote_f = REMOTE_DIR + "/" + fname
            print(f"    upload: {fname}")
            sftp.put(local_f, remote_f)

    # Write .env directly on remote (with localhost DB)
    print("    writing .env (DB_HOST=localhost)")
    with sftp.open(REMOTE_DIR + "/.env", "w") as f:
        f.write(ENV_CONTENT)

    sftp.close()

    # Install Python
    print("\n[4/7] Installing Python3 + venv on VPS ...")
    ssh_exec(client, "which python3 || sudo apt-get install -y python3 python3-venv python3-pip",
             "python3 check")

    # Create venv
    print("\n[5/7] Setting up virtual environment ...")
    ssh_exec(client, f"cd {REMOTE_DIR} && python3 -m venv .venv", "create venv")
    ssh_exec(client, f"cd {REMOTE_DIR} && .venv/bin/pip install -q --upgrade pip", "upgrade pip")
    ssh_exec(client, f"cd {REMOTE_DIR} && .venv/bin/pip install -q -r requirements.txt",
             "install deps")

    # Apply schema
    print("\n[6/7] Applying schema.sql to MySQL (localhost) ...")
    ssh_exec(client,
             f"mysql -u root -pDjg001248 < {REMOTE_DIR}/schema.sql 2>/dev/null || "
             f"mysql -u root -pDjg001248 kanban_db < {REMOTE_DIR}/schema.sql 2>/dev/null || true",
             "schema.sql")

    # Kill existing uvicorn and restart
    print("\n[7/7] Starting uvicorn on port 8000 ...")
    ssh_exec(client, "pkill -f 'uvicorn app.main' || true", "kill old")
    import time; time.sleep(1)
    start_cmd = (
        f"cd {REMOTE_DIR} && "
        f"nohup .venv/bin/uvicorn app.main:app --host 0.0.0.0 --port 8000 "
        f"> /tmp/kanban.log 2>&1 &"
    )
    ssh_exec(client, start_cmd, "start uvicorn")
    time.sleep(3)

    # Check if it started
    ssh_exec(client, "curl -s -o /dev/null -w '%{http_code}' http://localhost:8000/docs || echo 'not yet ready'",
             "health check")
    # Show last lines of log
    ssh_exec(client, "tail -n 20 /tmp/kanban.log", "server log")

    client.close()

    print("\n" + "=" * 50)
    print("  Deploy complete!")
    print(f"  API:  http://{VPS_HOST}:8000")
    print(f"  Docs: http://{VPS_HOST}:8000/docs")
    print(f"  Logs: ssh {VPS_USER}@{VPS_HOST} 'tail -f /tmp/kanban.log'")
    print("=" * 50)


if __name__ == "__main__":
    main()
