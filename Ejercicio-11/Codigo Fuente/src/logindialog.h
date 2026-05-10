#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog() override;

    int authenticatedUserId() const;
    QString authenticatedUsername() const;

private slots:
    void onLoginClicked();

private:
    Ui::LoginDialog *ui;
    int m_userId = -1;
    QString m_username;
};
