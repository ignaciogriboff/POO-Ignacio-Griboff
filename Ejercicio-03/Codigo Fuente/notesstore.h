#pragma once
#include <QString>
#include <QJsonObject>   // <-- AGREGAR ESTO

class NotesStore {
public:
    explicit NotesStore(const QString& filePath);

    QString loadNote(int tpId) const;                 // devuelve "" si no existe
    bool saveNote(int tpId, const QString& text) const; // true si pudo guardar

    QString filePath() const;

private:
    QString m_filePath;

    bool loadAllAsObject(QJsonObject& out) const;
    bool saveObject(const QJsonObject& obj) const;
};