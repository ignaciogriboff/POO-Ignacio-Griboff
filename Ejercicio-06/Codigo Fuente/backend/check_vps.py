import paramiko

c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("173.212.234.190", username="ignacio", password="12345",
          look_for_keys=False, allow_agent=False)

def run(cmd):
    _, out, _ = c.exec_command(cmd, get_pty=True)
    o = out.read().decode(errors="replace").strip()
    out.channel.recv_exit_status()
    if o:
        for line in o.splitlines():
            print("   ", line)

print("=== process on port 8000 ===")
run("ss -tlnp | grep 8000")
run("ps aux | grep uvicorn | grep -v grep")

print("=== systemd services ===")
run("echo 12345 | sudo -S systemctl list-units --type=service --state=running 2>/dev/null | grep -i kanban")

print("=== try our venv python directly ===")
run("ls /home/ignacio/kanban/backend/.venv/bin/ | head -20")

print("=== check .env ===")
run("cat /home/ignacio/kanban/backend/.env")

print("=== test start manually with output ===")
run("cd /home/ignacio/kanban/backend && .venv/bin/python -m uvicorn app.main:app --host 127.0.0.1 --port 8888 &")

import time; time.sleep(3)
run("curl -s -u admin:admin123 http://localhost:8888/columns 2>&1 || echo 'port 8888 failed'")
run("pkill -f 'port 8888' 2>/dev/null || true")

c.close()
