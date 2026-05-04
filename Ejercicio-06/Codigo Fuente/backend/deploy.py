#!/usr/bin/env python3
"""
deploy.py - Upload backend to VPS via SCP and configure/run with SSH.
Usage: python deploy.py
"""
import subprocess
import sys
import os

VPS_HOST = "173.212.234.190"
VPS_USER = "ignacio"
VPS_PASS = "12345"
REMOTE_DIR = "/home/ignacio/kanban/backend"
LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))

def run(cmd, input_text=None):
    print(f">>> {cmd}")
    result = subprocess.run(cmd, shell=True, input=input_text,
                            capture_output=False, text=True)
    return result.returncode

# Use sshpass if available, otherwise fall back to regular ssh (requires manual password)
def ssh_cmd(remote_cmd):
    return (f'sshpass -p "{VPS_PASS}" ssh -o StrictHostKeyChecking=no '
            f'{VPS_USER}@{VPS_HOST} "{remote_cmd}"')

def scp_cmd(local, remote):
    return (f'sshpass -p "{VPS_PASS}" scp -o StrictHostKeyChecking=no -r '
            f'"{local}" {VPS_USER}@{VPS_HOST}:{remote}')

if __name__ == "__main__":
    print("=== Kanban Backend Deploy ===")
    print(f"Target: {VPS_USER}@{VPS_HOST}:{REMOTE_DIR}")
    print()

    # Check sshpass
    r = subprocess.run("sshpass -V", shell=True, capture_output=True)
    if r.returncode != 0:
        print("NOTE: sshpass not found. You will need to enter the SSH password manually for each command.")
        def ssh_cmd(remote_cmd):
            return f'ssh -o StrictHostKeyChecking=no {VPS_USER}@{VPS_HOST} "{remote_cmd}"'
        def scp_cmd(local, remote):
            return f'scp -o StrictHostKeyChecking=no -r "{local}" {VPS_USER}@{VPS_HOST}:{remote}'

    # 1. Create remote directory
    run(ssh_cmd(f"mkdir -p {REMOTE_DIR}"))

    # 2. Upload files
    for item in ["app", "requirements.txt", "schema.sql", ".env", ".env.example"]:
        local_path = os.path.join(LOCAL_DIR, item)
        if os.path.exists(local_path):
            run(scp_cmd(local_path, REMOTE_DIR + "/"))

    # 3. Install Python and venv on VPS
    setup_cmds = [
        "sudo apt-get update -qq",
        "sudo apt-get install -y python3 python3-pip python3-venv",
        f"cd {REMOTE_DIR} && python3 -m venv .venv",
        f"cd {REMOTE_DIR} && .venv/bin/pip install -q -r requirements.txt",
    ]
    for cmd in setup_cmds:
        run(ssh_cmd(cmd))

    # 4. Create/update .env with DB_HOST=localhost (on VPS, MySQL is local)
    env_content = (
        "DB_HOST=localhost\\n"
        "DB_PORT=3306\\n"
        "DB_USER=root\\n"
        "DB_PASSWORD=Djg001248\\n"
        "DB_NAME=kanban_db\\n"
        "BASIC_AUTH_USER=admin\\n"
        "BASIC_AUTH_PASS=admin123\\n"
    )
    run(ssh_cmd(f'printf "{env_content}" > {REMOTE_DIR}/.env'))

    # 5. Run schema.sql
    run(ssh_cmd(
        f"mysql -u root -pDjg001248 kanban_db < {REMOTE_DIR}/schema.sql 2>/dev/null || true"
    ))

    # 6. Kill any existing uvicorn
    run(ssh_cmd("pkill -f uvicorn || true"))

    # 7. Start uvicorn as background service
    run(ssh_cmd(
        f"cd {REMOTE_DIR} && nohup .venv/bin/uvicorn app.main:app "
        f"--host 0.0.0.0 --port 8000 > /tmp/kanban.log 2>&1 &"
    ))

    print()
    print("=== Deploy complete ===")
    print(f"API should be available at: http://{VPS_HOST}:8000")
    print(f"Docs: http://{VPS_HOST}:8000/docs")
    print(f"Logs: ssh {VPS_USER}@{VPS_HOST} 'tail -f /tmp/kanban.log'")
