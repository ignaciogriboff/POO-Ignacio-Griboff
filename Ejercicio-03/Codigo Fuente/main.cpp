#include <QApplication>
#include <QDir>

#include "loginwidget.h"
#include "userstore.h"
#include "sessionmanager.h"
#include "mainwindowwidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    const QString usersPath = QDir::current().absoluteFilePath("users.json");
    const QString sessionPath = QDir::current().absoluteFilePath("session.json");

    UserStore store(usersPath);
    SessionManager session(sessionPath);

    // Si hay sesión válida, ir directo a la principal
    if (session.isSessionValid(5)) {
        QString username;
        QDateTime savedAt;
        session.loadSession(username, savedAt);

        MainWindowWidget* w = new MainWindowWidget(username);
        w->show();

        return app.exec();
    }

    // Si no hay sesión, mostrar login
    loginwidget login(&store);
    login.show();

    QObject::connect(&login, &loginwidget::loginSuccess, [&](const QString& username) {
        session.saveSession(username);

        MainWindowWidget* w = new MainWindowWidget(username);
        w->show();

        login.close();
    });

    return app.exec();
}
