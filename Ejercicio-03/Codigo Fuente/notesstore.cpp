#include "notesstore.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

NotesStore::NotesStore(const QString& filePath)
    : m_filePath(filePath) {}

QString NotesStore::filePath() const {
    return m_filePath;
}

bool NotesStore::loadAllAsObject(QJsonObject& out) const {
    out = QJsonObject();

    QFile f(m_filePath);
    if (!f.exists()) return true; // si no existe, lo tratamos como vacío
    if (!f.open(QIODevice::ReadOnly)) return false;

    const QByteArray data = f.readAll();
    f.close();

    if (data.trimmed().isEmpty()) return true;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) return false;
    if (!doc.isObject()) return false;

    out = doc.object();
    return true;
}

bool NotesStore::saveObject(const QJsonObject& obj) const {
    QFile f(m_filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;

    const QJsonDocument doc(obj);
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    return true;
}

QString NotesStore::loadNote(int tpId) const {
    QJsonObject obj;
    if (!loadAllAsObject(obj)) return "";

    const QString key = QString::number(tpId);
    return obj.value(key).toString();
}

bool NotesStore::saveNote(int tpId, const QString& text) const {
    QJsonObject obj;
    if (!loadAllAsObject(obj)) return false;

    const QString key = QString::number(tpId);
    obj.insert(key, text);

    return saveObject(obj);
}