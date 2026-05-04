import paramiko
import time

c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("173.212.234.190", username="ignacio", password="12345",
          look_for_keys=False, allow_agent=False)

REMOTE_DIR = "/home/ignacio/kanban/backend"

def run_notty(cmd, sudo_pass=None):
    transport = c.get_transport()
    ch = transport.open_session()
    ch.exec_command(cmd)
    if sudo_pass:
        ch.send(sudo_pass + "\n")
    ch.shutdown_write()
    out = b""
    while True:
        if ch.recv_ready():
            out += ch.recv(4096)
        if ch.exit_status_ready():
            break
        time.sleep(0.1)
    while ch.recv_ready():
        out += ch.recv(4096)
    o = out.decode(errors="replace").strip()
    if o:
        for line in o.splitlines():
            print("   ", line)
    return o

# Check current table structure
print("[1] Current columns table structure ...")
run_notty("mysql -u kanban_user -pDjg01248 -h 127.0.0.1 kanban_db -e 'DESCRIBE columns;'")

print("[2] Current cards table structure ...")
run_notty("mysql -u kanban_user -pDjg01248 -h 127.0.0.1 kanban_db -e 'DESCRIBE cards;'")

print("[3] Current card_order table structure ...")
run_notty("mysql -u kanban_user -pDjg01248 -h 127.0.0.1 kanban_db -e 'DESCRIBE card_order;'")
