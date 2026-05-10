#include "database.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

namespace {
constexpr const char *kConnectionName = "main_connection";
}

QSqlDatabase Database::db()
{
    return QSqlDatabase::database(kConnectionName);
}

QString Database::hashPassword(const QString &password)
{
    return QString::fromLatin1(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool Database::initialize(const QString &dbPath, QString *errorMessage)
{
    if (QSqlDatabase::contains(kConnectionName)) {
        QSqlDatabase existing = QSqlDatabase::database(kConnectionName);
        if (existing.isOpen()) {
            return true;
        }
    }

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
    database.setDatabaseName(dbPath);

    if (!database.open()) {
        if (errorMessage) {
            *errorMessage = database.lastError().text();
        }
        return false;
    }

    if (!ensureSchema(errorMessage)) {
        return false;
    }

    if (!ensureDefaultUser(errorMessage)) {
        return false;
    }

    return true;
}

bool Database::ensureSchema(QString *errorMessage)
{
    QSqlQuery query(db());

    const QString createUsers =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "password_hash TEXT NOT NULL"
        ")";

    const QString createAccessLogs =
        "CREATE TABLE IF NOT EXISTS access_logs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp TEXT NOT NULL,"
        "username TEXT NOT NULL,"
        "success INTEGER NOT NULL,"
        "message TEXT"
        ")";

    const QString createStrokes =
        "CREATE TABLE IF NOT EXISTS strokes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "created_at TEXT NOT NULL,"
        "color_r INTEGER NOT NULL,"
        "color_g INTEGER NOT NULL,"
        "color_b INTEGER NOT NULL,"
        "thickness INTEGER NOT NULL,"
        "order_index INTEGER NOT NULL,"
        "FOREIGN KEY(user_id) REFERENCES users(id)"
        ")";

    const QString createStrokePoints =
        "CREATE TABLE IF NOT EXISTS stroke_points ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "stroke_id INTEGER NOT NULL,"
        "point_index INTEGER NOT NULL,"
        "x INTEGER NOT NULL,"
        "y INTEGER NOT NULL,"
        "FOREIGN KEY(stroke_id) REFERENCES strokes(id)"
        ")";

    const QStringList statements = {
        createUsers,
        createAccessLogs,
        createStrokes,
        createStrokePoints,
        "CREATE INDEX IF NOT EXISTS idx_strokes_user_order ON strokes(user_id, order_index)",
        "CREATE INDEX IF NOT EXISTS idx_points_stroke ON stroke_points(stroke_id, point_index)"
    };

    for (const QString &statement : statements) {
        if (!query.exec(statement)) {
            if (errorMessage) {
                *errorMessage = query.lastError().text();
            }
            return false;
        }
    }

    return true;
}

bool Database::ensureDefaultUser(QString *errorMessage)
{
    QSqlQuery countQuery(db());
    if (!countQuery.exec("SELECT COUNT(*) FROM users")) {
        if (errorMessage) {
            *errorMessage = countQuery.lastError().text();
        }
        return false;
    }

    if (!countQuery.next()) {
        if (errorMessage) {
            *errorMessage = "No se pudo leer la cantidad de usuarios.";
        }
        return false;
    }

    if (countQuery.value(0).toInt() > 0) {
        return true;
    }

    QSqlQuery insertQuery(db());
    insertQuery.prepare("INSERT INTO users (username, password_hash) VALUES (:username, :password_hash)");
    insertQuery.bindValue(":username", "admin");
    insertQuery.bindValue(":password_hash", hashPassword("admin123"));

    if (!insertQuery.exec()) {
        if (errorMessage) {
            *errorMessage = insertQuery.lastError().text();
        }
        return false;
    }

    return true;
}

int Database::validateUser(const QString &username, const QString &password, QString *errorMessage)
{
    QSqlQuery query(db());
    query.prepare("SELECT id, password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return -1;
    }

    if (!query.next()) {
        return 0;
    }

    const int userId = query.value(0).toInt();
    const QString storedHash = query.value(1).toString();

    if (storedHash == hashPassword(password)) {
        return userId;
    }

    return 0;
}

void Database::logAccessAttempt(const QString &username, bool success, const QString &message)
{
    QSqlQuery query(db());
    query.prepare(
        "INSERT INTO access_logs (timestamp, username, success, message) "
        "VALUES (:timestamp, :username, :success, :message)");

    query.bindValue(":timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":username", username);
    query.bindValue(":success", success ? 1 : 0);
    query.bindValue(":message", message);
    query.exec();
}

int Database::nextOrderIndexForUser(int userId, QString *errorMessage)
{
    QSqlQuery query(db());
    query.prepare("SELECT COALESCE(MAX(order_index), 0) + 1 FROM strokes WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return -1;
    }

    if (!query.next()) {
        if (errorMessage) {
            *errorMessage = "No se pudo calcular el order_index.";
        }
        return -1;
    }

    return query.value(0).toInt();
}

QList<StrokeData> Database::loadStrokesForUser(int userId, QString *errorMessage)
{
    QList<StrokeData> strokes;

    QSqlQuery strokesQuery(db());
    strokesQuery.prepare(
        "SELECT id, color_r, color_g, color_b, thickness, order_index "
        "FROM strokes WHERE user_id = :user_id "
        "ORDER BY order_index ASC");
    strokesQuery.bindValue(":user_id", userId);

    if (!strokesQuery.exec()) {
        if (errorMessage) {
            *errorMessage = strokesQuery.lastError().text();
        }
        return strokes;
    }

    while (strokesQuery.next()) {
        StrokeData stroke;
        stroke.id = strokesQuery.value(0).toInt();
        stroke.color = QColor(strokesQuery.value(1).toInt(), strokesQuery.value(2).toInt(), strokesQuery.value(3).toInt());
        stroke.thickness = strokesQuery.value(4).toInt();
        stroke.orderIndex = strokesQuery.value(5).toInt();

        QSqlQuery pointsQuery(db());
        pointsQuery.prepare(
            "SELECT x, y FROM stroke_points "
            "WHERE stroke_id = :stroke_id ORDER BY point_index ASC");
        pointsQuery.bindValue(":stroke_id", stroke.id);

        if (!pointsQuery.exec()) {
            if (errorMessage) {
                *errorMessage = pointsQuery.lastError().text();
            }
            return QList<StrokeData>();
        }

        while (pointsQuery.next()) {
            stroke.points.append(QPoint(pointsQuery.value(0).toInt(), pointsQuery.value(1).toInt()));
        }

        strokes.append(stroke);
    }

    return strokes;
}

int Database::saveStrokeForUser(int userId, const StrokeData &stroke, QString *errorMessage)
{
    if (stroke.points.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "No se puede guardar un trazo sin puntos.";
        }
        return -1;
    }

    QSqlDatabase database = db();
    if (!database.transaction()) {
        if (errorMessage) {
            *errorMessage = database.lastError().text();
        }
        return -1;
    }

    const int orderIndex = nextOrderIndexForUser(userId, errorMessage);
    if (orderIndex < 0) {
        database.rollback();
        return -1;
    }

    QSqlQuery insertStroke(database);
    insertStroke.prepare(
        "INSERT INTO strokes (user_id, created_at, color_r, color_g, color_b, thickness, order_index) "
        "VALUES (:user_id, :created_at, :r, :g, :b, :thickness, :order_index)");

    insertStroke.bindValue(":user_id", userId);
    insertStroke.bindValue(":created_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    insertStroke.bindValue(":r", stroke.color.red());
    insertStroke.bindValue(":g", stroke.color.green());
    insertStroke.bindValue(":b", stroke.color.blue());
    insertStroke.bindValue(":thickness", stroke.thickness);
    insertStroke.bindValue(":order_index", orderIndex);

    if (!insertStroke.exec()) {
        if (errorMessage) {
            *errorMessage = insertStroke.lastError().text();
        }
        database.rollback();
        return -1;
    }

    const int strokeId = insertStroke.lastInsertId().toInt();

    QSqlQuery insertPoint(database);
    insertPoint.prepare(
        "INSERT INTO stroke_points (stroke_id, point_index, x, y) "
        "VALUES (:stroke_id, :point_index, :x, :y)");

    for (int i = 0; i < stroke.points.size(); ++i) {
        const QPoint &point = stroke.points.at(i);
        insertPoint.bindValue(":stroke_id", strokeId);
        insertPoint.bindValue(":point_index", i);
        insertPoint.bindValue(":x", point.x());
        insertPoint.bindValue(":y", point.y());

        if (!insertPoint.exec()) {
            if (errorMessage) {
                *errorMessage = insertPoint.lastError().text();
            }
            database.rollback();
            return -1;
        }
    }

    if (!database.commit()) {
        if (errorMessage) {
            *errorMessage = database.lastError().text();
        }
        database.rollback();
        return -1;
    }

    return strokeId;
}

bool Database::deleteStrokeById(int strokeId, QString *errorMessage)
{
    QSqlDatabase database = db();
    if (!database.transaction()) {
        if (errorMessage) {
            *errorMessage = database.lastError().text();
        }
        return false;
    }

    QSqlQuery deletePoints(database);
    deletePoints.prepare("DELETE FROM stroke_points WHERE stroke_id = :stroke_id");
    deletePoints.bindValue(":stroke_id", strokeId);

    if (!deletePoints.exec()) {
        if (errorMessage) {
            *errorMessage = deletePoints.lastError().text();
        }
        database.rollback();
        return false;
    }

    QSqlQuery deleteStroke(database);
    deleteStroke.prepare("DELETE FROM strokes WHERE id = :stroke_id");
    deleteStroke.bindValue(":stroke_id", strokeId);

    if (!deleteStroke.exec()) {
        if (errorMessage) {
            *errorMessage = deleteStroke.lastError().text();
        }
        database.rollback();
        return false;
    }

    if (!database.commit()) {
        if (errorMessage) {
            *errorMessage = database.lastError().text();
        }
        database.rollback();
        return false;
    }

    return true;
}

bool Database::clearStrokesForUser(int userId, QString *errorMessage)
{
    QSqlDatabase database = db();
    if (!database.transaction()) {
        if (errorMessage) {
            *errorMessage = database.lastError().text();
        }
        return false;
    }

    QSqlQuery selectStrokes(database);
    selectStrokes.prepare("SELECT id FROM strokes WHERE user_id = :user_id");
    selectStrokes.bindValue(":user_id", userId);

    if (!selectStrokes.exec()) {
        if (errorMessage) {
            *errorMessage = selectStrokes.lastError().text();
        }
        database.rollback();
        return false;
    }

    QList<int> strokeIds;
    while (selectStrokes.next()) {
        strokeIds.append(selectStrokes.value(0).toInt());
    }

    QSqlQuery deletePoints(database);
    deletePoints.prepare("DELETE FROM stroke_points WHERE stroke_id = :stroke_id");

    for (int strokeId : strokeIds) {
        deletePoints.bindValue(":stroke_id", strokeId);
        if (!deletePoints.exec()) {
            if (errorMessage) {
                *errorMessage = deletePoints.lastError().text();
            }
            database.rollback();
            return false;
        }
    }

    QSqlQuery deleteStrokes(database);
    deleteStrokes.prepare("DELETE FROM strokes WHERE user_id = :user_id");
    deleteStrokes.bindValue(":user_id", userId);

    if (!deleteStrokes.exec()) {
        if (errorMessage) {
            *errorMessage = deleteStrokes.lastError().text();
        }
        database.rollback();
        return false;
    }

    if (!database.commit()) {
        if (errorMessage) {
            *errorMessage = database.lastError().text();
        }
        database.rollback();
        return false;
    }

    return true;
}
