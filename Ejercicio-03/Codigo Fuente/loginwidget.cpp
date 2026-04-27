#include "loginwidget.h"
#include "userstore.h"

#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

loginwidget::loginwidget(UserStore* userStore, QWidget* parent)
    : QWidget(parent),
      m_userStore(userStore) {
    auto* title = new QLabel("Login");
    QFont tf = title->font();
    tf.setPointSize(14);
    tf.setBold(true);
    title->setFont(tf);

    auto* userLabel = new QLabel("Usuario:");
    m_userEdit = new QLineEdit();
    m_userEdit->setPlaceholderText("Ej: admin");

    auto* passLabel = new QLabel("Password:");
    m_passEdit = new QLineEdit();
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setPlaceholderText("Tu contraseña");

    m_loginBtn = new QPushButton("Ingresar");

    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("color:#ff5555;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setText("");

    // Info útil (opcional): mostrar si el archivo existe
    QString infoText;
    if (m_userStore) {
        QFileInfo fi(m_userStore->filePath());
        if (!fi.exists()) {
            infoText = QString("No se encontró users.json en:\n%1").arg(fi.absoluteFilePath());
        }
    }
    auto* infoLabel = new QLabel(infoText);
    infoLabel->setStyleSheet("color:#aaaaaa; font-size: 11px;");
    infoLabel->setWordWrap(true);

    auto* layout = new QVBoxLayout();
    layout->addWidget(title);
    layout->addSpacing(10);
    layout->addWidget(userLabel);
    layout->addWidget(m_userEdit);
    layout->addWidget(passLabel);
    layout->addWidget(m_passEdit);
    layout->addSpacing(10);
    layout->addWidget(m_loginBtn);
    layout->addWidget(m_errorLabel);
    if (!infoText.isEmpty())
        layout->addWidget(infoLabel);

    setLayout(layout);
    setWindowTitle("Planificador TP - Login");
    setFixedWidth(340);

    connect(m_loginBtn, &QPushButton::clicked, this, &loginwidget::onLoginClicked);
    connect(m_userEdit, &QLineEdit::returnPressed, this, &loginwidget::onLoginClicked);
    connect(m_passEdit, &QLineEdit::returnPressed, this, &loginwidget::onLoginClicked);
}

void loginwidget::onLoginClicked() {
    m_errorLabel->setText("");

    const QString user = m_userEdit->text().trimmed();
    const QString pass = m_passEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        m_errorLabel->setText("Completá usuario y password.");
        return;
    }

    if (!m_userStore || !m_userStore->validateCredentials(user, pass)) {
        m_errorLabel->setText("Usuario o password inválidos.");
        return;
    }

    emit loginSuccess(user);
}
