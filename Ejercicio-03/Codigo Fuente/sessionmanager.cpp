#include "sessionmanager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

SessionManager::SessionManager(const QString& sessionFilePath)
    : m_sessionFilePath(sessionFilePath) {}

QString SessionManager::filePath() const {
    return m_sessionFilePath;
}

void SessionManager::saveSession(const QString& username) const {
    QJsonObject obj;
    obj["username"] = username;
    // ISO 8601, fácil de parsear
    obj["saved_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    const QJsonDocument doc(obj);

    QFile f(m_sessionFilePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(doc.toJson(QJsonDocument::Indented));
        f.close();
    }
}

bool SessionManager::loadSession(QString& usernameOut, QDateTime& savedAtOut) const {
    QFile f(m_sessionFilePath);
    if (!f.exists()) return false;
    if (!f.open(QIODevice::ReadOnly)) return false;

    const QByteArray data = f.readAll();
    f.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) return false;
    if (!doc.isObject()) return false;

    const QJsonObject obj = doc.object();
    usernameOut = obj.value("username").toString();

    const QString savedAtStr = obj.value("saved_at").toString();
    const QDateTime dt = QDateTime::fromString(savedAtStr, Qt::ISODate);

    if (!dt.isValid() || usernameOut.isEmpty()) return false;

    savedAtOut = dt;
    return true;
}

bool SessionManager::isSessionValid(int validMinutes) const {
    QString username;
    QDateTime savedAt;
    if (!loadSession(username, savedAt)) return false;

    const QDateTime now = QDateTime::currentDateTimeUtc();

    // si la fecha guardada es futura (raro), consideramos inválida
    if (savedAt > now) return false;

    const qint64 seconds = savedAt.secsTo(now);
    return seconds <= (validMinutes * 60);
}

void SessionManager::clearSession() const {
    QFile::remove(m_sessionFilePath);
}
