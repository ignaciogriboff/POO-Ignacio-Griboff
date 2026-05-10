#include "logindialog.h"

#include "database.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    connect(ui->btnLogin, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

int LoginDialog::authenticatedUserId() const
{
    return m_userId;
}

QString LoginDialog::authenticatedUsername() const
{
    return m_username;
}

void LoginDialog::onLoginClicked()
{
    const QString username = ui->editUsername->text().trimmed();
    const QString password = ui->editPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        ui->lblStatus->setText("Completa usuario y contrasena.");
        Database::logAccessAttempt(username.isEmpty() ? "<vacio>" : username, false, "Campos incompletos");
        return;
    }

    QString errorMessage;
    const int userId = Database::validateUser(username, password, &errorMessage);

    if (userId < 0) {
        ui->lblStatus->setText("Error consultando base de datos.");
        Database::logAccessAttempt(username, false, QString("Error DB: %1").arg(errorMessage));
        return;
    }

    if (userId == 0) {
        ui->lblStatus->setText("Usuario o contrasena incorrectos.");
        Database::logAccessAttempt(username, false, "Credenciales invalidas");
        return;
    }

    Database::logAccessAttempt(username, true, "Login exitoso");
    m_userId = userId;
    m_username = username;
    accept();
}
