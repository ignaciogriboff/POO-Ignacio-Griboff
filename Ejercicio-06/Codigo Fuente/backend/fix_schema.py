import paramiko
import time

c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("173.212.234.190", username="ignacio", password="12345",
          look_for_keys=False, allow_agent=False)

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

MYSQL = "mysql -u kanban_user -pDjg01248 -h 127.0.0.1 kanban_db"

print("[1] Altering tables to match our schema ...")
alter_sql = """
-- Add position to columns
ALTER TABLE columns ADD COLUMN IF NOT EXISTS position INT NOT NULL DEFAULT 0 AFTER title;
ALTER TABLE columns ADD INDEX IF NOT EXISTS idx_position (position);

-- Add id to card_order (drop PK on card_id first, then add auto-increment id)
-- Only if card_order.id doesn't exist
SET @col_exists = (SELECT COUNT(*) FROM information_schema.COLUMNS 
                   WHERE TABLE_SCHEMA='kanban_db' AND TABLE_NAME='card_order' AND COLUMN_NAME='id');
-- We'll handle this with a conditional approach via a procedure

-- Update card_order: add id column if not exists
ALTER TABLE card_order ADD COLUMN IF NOT EXISTS id INT AUTO_INCREMENT PRIMARY KEY FIRST;

-- Add UNIQUE KEY on column_id, card_id if not exists
-- (ignore error if already exists)
ALTER TABLE card_order ADD UNIQUE KEY IF NOT EXISTS uq_column_card (column_id, card_id);
"""

# Write to temp file and run
sftp = c.open_sftp()
with sftp.open("/tmp/alter_schema.sql", "w") as f:
    f.write(alter_sql)
sftp.close()

run_notty(f"{MYSQL} < /tmp/alter_schema.sql")

print("[2] Verify fixed schemas ...")
run_notty(f"{MYSQL} -e 'DESCRIBE columns;'")
run_notty(f"{MYSQL} -e 'DESCRIBE card_order;'")

print("[3] Restart service ...")
run_notty("sudo -S systemctl restart kanban-ignacio.service", sudo_pass="12345")
time.sleep(5)

print("[4] Health check ...")
run_notty("curl -s -u admin:admin123 http://localhost:8000/columns")

run_notty("rm -f /tmp/alter_schema.sql")
c.close()
print("\n=== Done ===")
print("API:  http://173.212.234.190:8000")
print("Docs: http://173.212.234.190:8000/docs")
