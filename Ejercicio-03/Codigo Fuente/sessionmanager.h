#pragma once
#include <QString>
#include <QDateTime>

class SessionManager {
public:
    explicit SessionManager(const QString& sessionFilePath);

    void saveSession(const QString& username) const;
    bool loadSession(QString& usernameOut, QDateTime& savedAtOut) const;

    bool isSessionValid(int validMinutes = 5) const;
    void clearSession() const;

    QString filePath() const;

private:
    QString m_sessionFilePath;
};
