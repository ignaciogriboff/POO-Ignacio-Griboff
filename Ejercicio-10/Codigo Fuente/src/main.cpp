#include "appconfig.h"
#include "application_controller.h"
#include "logger.h"

#include <QApplication>
#include <QDir>
#include <QFont>
#include <QPalette>

static void aplicarTemaOscuro(QApplication& app) {
    QPalette p;
    const QColor bg(10, 14, 26);
    const QColor bgAlt(17, 24, 39);
    const QColor text(226, 232, 240);
    const QColor textDis(71, 85, 105);
    const QColor accent(0, 212, 255);
    const QColor border(45, 55, 72);
    const QColor selection(59, 130, 246, 120);

    p.setColor(QPalette::Window,          bg);
    p.setColor(QPalette::WindowText,      text);
    p.setColor(QPalette::Base,            bgAlt);
    p.setColor(QPalette::AlternateBase,   bg);
    p.setColor(QPalette::Text,            text);
    p.setColor(QPalette::BrightText,      accent);
    p.setColor(QPalette::ButtonText,      text);
    p.setColor(QPalette::Button,          QColor(26, 31, 46));
    p.setColor(QPalette::Highlight,       selection);
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Disabled, QPalette::WindowText, textDis);
    p.setColor(QPalette::Disabled, QPalette::Text,       textDis);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, textDis);
    p.setColor(QPalette::Link,            accent);
    p.setColor(QPalette::LinkVisited,     QColor(139, 92, 246));
    p.setColor(QPalette::ToolTipBase,     bgAlt);
    p.setColor(QPalette::ToolTipText,     text);
    app.setPalette(p);

    app.setStyleSheet(R"(
        QWidget {
            background-color: #0a0e1a;
            color: #e2e8f0;
            font-family: 'Segoe UI', 'Consolas', monospace;
            font-size: 13px;
        }
        QLineEdit, QPlainTextEdit, QTextEdit, QComboBox {
            background-color: #111827;
            border: 1px solid #2d3748;
            border-radius: 4px;
            padding: 6px 10px;
            color: #e2e8f0;
            selection-background-color: #3b82f6;
        }
        QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus {
            border: 1px solid #00d4ff;
            outline: none;
        }
        QComboBox::drop-down {
            border: none;
            padding-right: 8px;
        }
        QComboBox QAbstractItemView {
            background-color: #1a1f2e;
            border: 1px solid #2d3748;
            selection-background-color: #1e3a5f;
            color: #e2e8f0;
        }
        QPushButton {
            background-color: #1a1f2e;
            border: 1px solid #00d4ff;
            border-radius: 4px;
            padding: 7px 18px;
            color: #00d4ff;
            font-weight: 600;
            letter-spacing: 0.5px;
        }
        QPushButton:hover {
            background-color: #00d4ff;
            color: #0a0e1a;
        }
        QPushButton:pressed {
            background-color: #0099bb;
            color: #0a0e1a;
        }
        QPushButton:disabled {
            border-color: #2d3748;
            color: #475569;
        }
        QProgressBar {
            background-color: #111827;
            border: 1px solid #2d3748;
            border-radius: 4px;
            text-align: center;
            color: #e2e8f0;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #00d4ff, stop:1 #3b82f6);
            border-radius: 3px;
        }
        QScrollBar:vertical {
            background: #111827;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #2d3748;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #00d4ff;
        }
        QScrollBar:horizontal {
            background: #111827;
            height: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal {
            background: #2d3748;
            border-radius: 4px;
            min-width: 20px;
        }
        QScrollBar::add-line, QScrollBar::sub-line { background: none; }
        QMessageBox {
            background-color: #111827;
        }
        QToolTip {
            background-color: #1a1f2e;
            border: 1px solid #00d4ff;
            color: #e2e8f0;
            padding: 4px 8px;
        }
    )");
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    aplicarTemaOscuro(app);

    const AppConfig config = AppConfig::cargar("config.ini");

    QDir().mkpath("logs");
    Logger::instancia().configurarRuta("logs/eventos.log");
    Logger::instancia().registrar("Aplicacion iniciada en modo offline");

    ApplicationController controller(config);
    controller.iniciar();

    return app.exec();
}
