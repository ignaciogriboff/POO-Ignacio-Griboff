#pragma once
#include <QString>
#include <QStringList>

class HistoryLogger {
public:
    explicit HistoryLogger(const QString& filePath);

    QString log(const QString& username, const QString& action, const QString& details);
    QStringList readAllLines() const;  // <-- NUEVO

    QString filePath() const;

private:
    QString m_filePath;
};