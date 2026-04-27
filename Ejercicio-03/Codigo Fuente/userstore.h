#pragma once
#include <QString>

class UserStore {
public:
    explicit UserStore(const QString& filePath);

    bool validateCredentials(const QString& username, const QString& password) const;

    QString filePath() const;

private:
    QString m_filePath;
};
