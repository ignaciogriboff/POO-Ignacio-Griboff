#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;
class UserStore;

class loginwidget : public QWidget {
    Q_OBJECT
public:
    explicit loginwidget(UserStore* userStore, QWidget* parent = nullptr);

signals:
    void loginSuccess(const QString& username);

private slots:
    void onLoginClicked();

private:
    UserStore* m_userStore = nullptr;

    QLineEdit* m_userEdit = nullptr;
    QLineEdit* m_passEdit = nullptr;
    QLabel* m_errorLabel = nullptr;
    QPushButton* m_loginBtn = nullptr;
};
