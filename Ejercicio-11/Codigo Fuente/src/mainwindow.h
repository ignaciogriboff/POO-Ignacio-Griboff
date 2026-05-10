#pragma once

#include <QMainWindow>

class QLabel;
class Pintura;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(int userId, const QString &username, QWidget *parent = nullptr);

private:
    void setupUi(int userId, const QString &username);

    QLabel *m_statusLabel = nullptr;
    Pintura *m_pintura = nullptr;
};
