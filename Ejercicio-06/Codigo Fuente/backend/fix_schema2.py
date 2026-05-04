import paramiko
import time

c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("173.212.234.190", username="ignacio", password="12345",
          look_for_keys=False, allow_agent=False)

def run_notty(cmd, sudo_pass=None, show=True):
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
    if show and o:
        for line in o.splitlines():
            print("   ", line)
    return o

MYSQL = "mysql -u kanban_user -pDjg01248 -h 127.0.0.1 kanban_db"

# First check MySQL version
print("[0] MySQL version ...")
run_notty(f"{MYSQL} -e 'SELECT VERSION();'")

print("[1] Recreating tables with correct schema ...")
recreate_sql = """
SET FOREIGN_KEY_CHECKS = 0;
DROP TABLE IF EXISTS card_order;
DROP TABLE IF EXISTS cards;
DROP TABLE IF EXISTS `columns`;
SET FOREIGN_KEY_CHECKS = 1;

CREATE TABLE `columns` (
    id INT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    position INT NOT NULL DEFAULT 0,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_position (position)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE cards (
    id INT AUTO_INCREMENT PRIMARY KEY,
    column_id INT NOT NULL,
    title VARCHAR(255) NOT NULL,
    description TEXT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_cards_column FOREIGN KEY (column_id) REFERENCES `columns`(id) ON DELETE CASCADE,
    INDEX idx_column_id (column_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE card_order (
    id INT AUTO_INCREMENT PRIMARY KEY,
    column_id INT NOT NULL,
    card_id INT NOT NULL,
    position INT NOT NULL DEFAULT 0,
    UNIQUE KEY uq_column_card (column_id, card_id),
    INDEX idx_column_position (column_id, position),
    CONSTRAINT fk_order_column FOREIGN KEY (column_id) REFERENCES `columns`(id) ON DELETE CASCADE,
    CONSTRAINT fk_order_card FOREIGN KEY (card_id) REFERENCES cards(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
"""

sftp = c.open_sftp()
with sftp.open("/tmp/recreate.sql", "w") as f:
    f.write(recreate_sql)
sftp.close()

run_notty(f"{MYSQL} < /tmp/recreate.sql")

print("[2] Verifying schemas ...")
run_notty(f"{MYSQL} -e 'DESCRIBE \\`columns\\`;'")
run_notty(f"{MYSQL} -e 'DESCRIBE cards;'")
run_notty(f"{MYSQL} -e 'DESCRIBE card_order;'")

print("[3] Restarting service ...")
run_notty("sudo -S systemctl restart kanban-ignacio.service", sudo_pass="12345")
time.sleep(5)

print("[4] Health check ...")
result = run_notty("curl -s -u admin:admin123 http://localhost:8000/columns")
if result.startswith("["):
    print(f"   SUCCESS! API works: {result}")
else:
    # Show last logs
    run_notty("sudo -S journalctl -u kanban-ignacio.service -n 5 --no-pager", sudo_pass="12345")

run_notty("rm -f /tmp/recreate.sql", show=False)
c.close()
print("\n=== Done ===")
print("API:  http://173.212.234.190:8000")
print("Docs: http://173.212.234.190:8000/docs")
