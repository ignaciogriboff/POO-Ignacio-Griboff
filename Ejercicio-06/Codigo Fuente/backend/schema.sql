-- Kanban Collaborative Board - Database Schema
-- Run: mysql -u root -p < schema.sql

CREATE DATABASE IF NOT EXISTS kanban_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- Create application user (adjust password as needed)
-- CREATE USER IF NOT EXISTS 'kanban_user'@'%' IDENTIFIED BY 'KanbanPass2024!';
-- GRANT ALL PRIVILEGES ON kanban_db.* TO 'kanban_user'@'%';
-- FLUSH PRIVILEGES;

USE kanban_db;

CREATE TABLE IF NOT EXISTS columns (
    id INT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    position INT NOT NULL DEFAULT 0,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_position (position)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS cards (
    id INT AUTO_INCREMENT PRIMARY KEY,
    column_id INT NOT NULL,
    title VARCHAR(255) NOT NULL,
    description TEXT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_cards_column FOREIGN KEY (column_id) REFERENCES columns(id) ON DELETE CASCADE,
    INDEX idx_column_id (column_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS card_order (
    id INT AUTO_INCREMENT PRIMARY KEY,
    column_id INT NOT NULL,
    card_id INT NOT NULL,
    position INT NOT NULL DEFAULT 0,
    UNIQUE KEY uq_column_card (column_id, card_id),
    INDEX idx_column_position (column_id, position),
    CONSTRAINT fk_order_column FOREIGN KEY (column_id) REFERENCES columns(id) ON DELETE CASCADE,
    CONSTRAINT fk_order_card FOREIGN KEY (card_id) REFERENCES cards(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
