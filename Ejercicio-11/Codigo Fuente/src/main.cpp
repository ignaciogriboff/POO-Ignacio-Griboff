#include "database.h"
#include "logindialog.h"
#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    QFile styleFile(":/styles/dark_tech.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
    }

    const QString dbPath = QApplication::applicationDirPath() + "/datos.sqlite";

    QString errorMessage;
    if (!Database::initialize(dbPath, &errorMessage)) {
        QMessageBox::critical(nullptr, "Error", QString("No se pudo inicializar la base de datos: %1").arg(errorMessage));
        return 1;
    }

    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window(login.authenticatedUserId(), login.authenticatedUsername());
    window.show();

    return app.exec();
}
