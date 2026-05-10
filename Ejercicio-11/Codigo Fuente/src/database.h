#pragma once

#include <QColor>
#include <QList>
#include <QPoint>
#include <QSqlDatabase>
#include <QString>

struct StrokeData {
    int id = -1;
    QColor color = Qt::black;
    int thickness = 4;
    int orderIndex = 0;
    QList<QPoint> points;
};

class Database {
public:
    static bool initialize(const QString &dbPath, QString *errorMessage = nullptr);
    static int validateUser(const QString &username, const QString &password, QString *errorMessage = nullptr);
    static void logAccessAttempt(const QString &username, bool success, const QString &message = QString());

    static QList<StrokeData> loadStrokesForUser(int userId, QString *errorMessage = nullptr);
    static int saveStrokeForUser(int userId, const StrokeData &stroke, QString *errorMessage = nullptr);
    static bool deleteStrokeById(int strokeId, QString *errorMessage = nullptr);
    static bool clearStrokesForUser(int userId, QString *errorMessage = nullptr);

private:
    static QString hashPassword(const QString &password);
    static bool ensureSchema(QString *errorMessage = nullptr);
    static bool ensureDefaultUser(QString *errorMessage = nullptr);
    static int nextOrderIndexForUser(int userId, QString *errorMessage = nullptr);
    static QSqlDatabase db();
};
