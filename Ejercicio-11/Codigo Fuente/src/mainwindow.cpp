#include "mainwindow.h"

#include "pintura.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(int userId, const QString &username, QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(userId, username);
}

void MainWindow::setupUi(int userId, const QString &username)
{
    setWindowTitle("Ejercicio 09 - Coordenadas en base de datos");
    resize(1000, 650);

    QWidget *central = new QWidget(this);
    central->setObjectName("mainRoot");
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    QLabel *title = new QLabel(QString("Usuario: %1").arg(username), central);
    title->setObjectName("mainTitle");

    QLabel *help = new QLabel(
        "Atajos: R/G/B cambia color | Rueda cambia grosor | Escape borra | Ctrl+Z deshace (ultimas 10)",
        central);
    help->setObjectName("helpLabel");
    help->setWordWrap(true);
    help->setMinimumHeight(44);

    m_pintura = new Pintura(userId, central);
    m_statusLabel = new QLabel(central);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setMinimumHeight(36);

    layout->addWidget(title);
    layout->addWidget(help);
    layout->addWidget(m_pintura, 1);
    layout->addWidget(m_statusLabel);

    setCentralWidget(central);

    connect(m_pintura, &Pintura::statusMessageChanged, m_statusLabel, &QLabel::setText);
    m_statusLabel->setText("Listo para dibujar");
}
