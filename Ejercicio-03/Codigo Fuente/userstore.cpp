#include "userstore.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

UserStore::UserStore(const QString& filePath)
    : m_filePath(filePath) {}

QString UserStore::filePath() const {
    return m_filePath;
}

bool UserStore::validateCredentials(const QString& username, const QString& password) const {
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) return false;
    if (!doc.isArray()) return false;

    const QJsonArray arr = doc.array();
    for (const QJsonValue& v : arr) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        const QString u = o.value("username").toString();
        const QString p = o.value("password").toString();

        if (u == username && p == password) {
            return true;
        }
    }
    return false;
}
