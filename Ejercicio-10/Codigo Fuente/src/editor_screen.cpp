#include "editor_screen.h"

#include "logger.h"
#include "validadores.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFocusEvent>
#include <QFont>
#include <QFontDatabase>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QShortcut>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>

EditorScreen::EditorScreen(const QString& defaultLanguage, const QString& exportPath, QWidget* parent)
    : Pantalla(parent)
    , m_defaultLanguage(defaultLanguage)
    , m_exportPath(exportPath)
    , m_lastLine(0)
    , m_languageSelector(nullptr)
    , m_editor(nullptr)
    , m_diagnostico(nullptr)
    , m_btnExport(nullptr)
    , m_curriculum(nullptr) {
    inicializarUI();
    conectarEventos();
    cargarDatos();
    validarEstado();
    registrarEvento("Pantalla Editor inicializada");
}

void EditorScreen::inicializarUI() {
    setWindowTitle("Editor Multilenguaje — Dark Tech");

    auto* root = new QHBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // ── Panel principal ──────────────────────────────────────────────
    auto* mainPanel = new QWidget(this);
    mainPanel->setStyleSheet("background-color: #0a0e1a;");
    auto* mainLayout = new QVBoxLayout(mainPanel);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Barra de herramientas
    auto* toolbarWidget = new QWidget(mainPanel);
    toolbarWidget->setFixedHeight(46);
    toolbarWidget->setStyleSheet(
        "background-color: #111827;"
        "border-bottom: 1px solid #1e3a5f;"
    );
    auto* toolbar = new QHBoxLayout(toolbarWidget);
    toolbar->setContentsMargins(12, 6, 12, 6);
    toolbar->setSpacing(10);

    auto* label = new QLabel("LENGUAJE:", toolbarWidget);
    label->setStyleSheet("color: #475569; font-size: 10px; letter-spacing: 2px;");

    m_languageSelector = new QComboBox(toolbarWidget);
    m_languageSelector->addItems({"C++", "Python", "Java"});
    m_languageSelector->setFixedWidth(120);
    m_languageSelector->setStyleSheet(
        "QComboBox { background: #1a1f2e; border: 1px solid #2d3748;"
        "  border-radius: 4px; padding: 4px 8px; color: #00d4ff; font-weight: 600; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: #1a1f2e; border: 1px solid #2d3748;"
        "  color: #e2e8f0; selection-background-color: #1e3a5f; }"
    );

    m_btnExport = new QPushButton("⬇  EXPORTAR JPG", toolbarWidget);
    m_btnExport->setFixedHeight(30);
    m_btnExport->setStyleSheet(
        "QPushButton { background: transparent; border: 1px solid #00d4ff;"
        "  border-radius: 4px; padding: 4px 14px; color: #00d4ff;"
        "  font-size: 11px; font-weight: 700; letter-spacing: 1px; }"
        "QPushButton:hover { background: #00d4ff; color: #0a0e1a; }"
        "QPushButton:pressed { background: #0099bb; color: #0a0e1a; }"
    );

    toolbar->addWidget(label);
    toolbar->addWidget(m_languageSelector);
    toolbar->addStretch();
    toolbar->addWidget(m_btnExport);

    // Editor
    m_editor = new QPlainTextEdit(mainPanel);
    m_editor->setPlaceholderText("// Escriba su codigo aqui...");
    m_editor->setStyleSheet(
        "QPlainTextEdit {"
        "  background-color: #0d1117;"
        "  color: #c9d1d9;"
        "  border: none;"
        "  padding: 12px 16px;"
        "  selection-background-color: #1e3a5f;"
        "}"
    );
    QFont mono("Consolas");
    mono.setPointSize(12);
    m_editor->setFont(mono);
    m_editor->setLineWrapMode(QPlainTextEdit::NoWrap);

    // Barra de estado/diagnóstico
    m_diagnostico = new QLabel("  > sin validaciones", mainPanel);
    m_diagnostico->setFixedHeight(28);
    m_diagnostico->setStyleSheet(
        "background-color: #111827;"
        "border-top: 1px solid #1e3a5f;"
        "color: #3b82f6;"
        "padding: 0 12px;"
        "font-family: 'Consolas', monospace;"
        "font-size: 11px;"
    );

    mainLayout->addWidget(toolbarWidget);
    mainLayout->addWidget(m_editor, 1);
    mainLayout->addWidget(m_diagnostico);

    // ── Panel lateral LinkedIn dark tech ─────────────────────────────
    auto* sidePanel = new QWidget(this);
    sidePanel->setFixedWidth(300);
    sidePanel->setStyleSheet(
        "background-color: #111827;"
        "border-left: 1px solid #1e3a5f;"
    );
    auto* sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setSpacing(0);
    sideLayout->setContentsMargins(0, 0, 0, 0);

    // Cabecera
    auto* sideHeader = new QWidget(sidePanel);
    sideHeader->setFixedHeight(48);
    sideHeader->setStyleSheet("background-color: #0a0e1a; border-bottom: 1px solid #1e3a5f;");
    auto* sideHeaderLayout = new QHBoxLayout(sideHeader);
    sideHeaderLayout->setContentsMargins(16, 0, 16, 0);
    auto* sideTitle = new QLabel("PERFIL", sideHeader);
    sideTitle->setStyleSheet("color: #00d4ff; font-size: 10px; font-weight: 700; letter-spacing: 3px;");
    sideHeaderLayout->addWidget(sideTitle);
    sideHeaderLayout->addStretch();

    auto* scrollArea = new QScrollArea(sidePanel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* content = new QWidget();
    content->setStyleSheet("background-color: #111827;");
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(16);
    contentLayout->setContentsMargins(20, 20, 20, 20);

    // Avatar
    auto* foto = new QLabel(content);
    foto->setFixedSize(90, 90);
    foto->setAlignment(Qt::AlignCenter);
    foto->setText("FC");
    foto->setStyleSheet(
        "border-radius: 45px;"
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #0099bb,stop:1 #3b82f6);"
        "color: white; font-size: 28px; font-weight: 700;"
    );

    auto* nombre = new QLabel("Facundo Coder", content);
    nombre->setStyleSheet("font-size: 17px; font-weight: 700; color: #e2e8f0;");
    nombre->setAlignment(Qt::AlignHCenter);

    auto* rol = new QLabel("Software Developer", content);
    rol->setStyleSheet("color: #00d4ff; font-size: 12px; letter-spacing: 1px;");
    rol->setAlignment(Qt::AlignHCenter);

    auto* sep1 = new QFrame(content);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("background: #1e3a5f; border: none; max-height: 1px;");

    auto* secAbout = new QLabel("ACERCA", content);
    secAbout->setStyleSheet("color: #475569; font-size: 10px; letter-spacing: 2px;");

    auto* bio = new QLabel("Desarrollador con foco en C++, Qt Widgets y arquitectura de escritorio orientada a objetos.", content);
    bio->setWordWrap(true);
    bio->setStyleSheet("color: #94a3b8; font-size: 12px; line-height: 1.5;");

    auto* sep2 = new QFrame(content);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("background: #1e3a5f; border: none; max-height: 1px;");

    auto* secSkills = new QLabel("HABILIDADES", content);
    secSkills->setStyleSheet("color: #475569; font-size: 10px; letter-spacing: 2px;");

    auto makeSkill = [&](const QString& skill, const QString& color, QWidget* parent) -> QWidget* {
        auto* chip = new QLabel(skill, parent);
        chip->setStyleSheet(
            "background-color: " + color + "22;"
            "border: 1px solid " + color + ";"
            "border-radius: 3px;"
            "color: " + color + ";"
            "padding: 3px 8px;"
            "font-size: 11px;"
            "font-weight: 600;"
        );
        return chip;
    };

    auto* skillsFlow = new QHBoxLayout();
    skillsFlow->setSpacing(6);
    skillsFlow->setContentsMargins(0, 0, 0, 0);
    skillsFlow->addWidget(makeSkill("C++",    "#00d4ff", content));
    skillsFlow->addWidget(makeSkill("Python", "#3b82f6", content));
    skillsFlow->addWidget(makeSkill("Java",   "#f97316", content));
    skillsFlow->addStretch();

    auto* skillsFlow2 = new QHBoxLayout();
    skillsFlow2->setSpacing(6);
    skillsFlow2->setContentsMargins(0, 0, 0, 0);
    skillsFlow2->addWidget(makeSkill("Qt6",       "#8b5cf6", content));
    skillsFlow2->addWidget(makeSkill("CMake",     "#10b981", content));
    skillsFlow2->addWidget(makeSkill("OOP",       "#00d4ff", content));
    skillsFlow2->addStretch();

    auto* sep3 = new QFrame(content);
    sep3->setFrameShape(QFrame::HLine);
    sep3->setStyleSheet("background: #1e3a5f; border: none; max-height: 1px;");

    auto* secContact = new QLabel("CONTACTO", content);
    secContact->setStyleSheet("color: #475569; font-size: 10px; letter-spacing: 2px;");

    m_curriculum = new QLabel(
        "✉  facundo.dev@example.com\n"
        "🔗  linkedin.com/in/facundo-dev\n"
        "⚙  github.com/facundo-dev",
        content);
    m_curriculum->setStyleSheet(
        "color: #94a3b8; font-size: 12px; line-height: 2;"
    );
    m_curriculum->setWordWrap(true);

    contentLayout->addWidget(foto, 0, Qt::AlignHCenter);
    contentLayout->addWidget(nombre);
    contentLayout->addWidget(rol);
    contentLayout->addWidget(sep1);
    contentLayout->addWidget(secAbout);
    contentLayout->addWidget(bio);
    contentLayout->addWidget(sep2);
    contentLayout->addWidget(secSkills);
    contentLayout->addLayout(skillsFlow);
    contentLayout->addLayout(skillsFlow2);
    contentLayout->addWidget(sep3);
    contentLayout->addWidget(secContact);
    contentLayout->addWidget(m_curriculum);
    contentLayout->addStretch();

    scrollArea->setWidget(content);

    sideLayout->addWidget(sideHeader);
    sideLayout->addWidget(scrollArea);

    root->addWidget(mainPanel, 1);
    root->addWidget(sidePanel);

    auto* exportShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(exportShortcut, &QShortcut::activated, this, &EditorScreen::exportarJpg);
}

void EditorScreen::conectarEventos() {
    connect(m_languageSelector, &QComboBox::currentTextChanged, this, &EditorScreen::onLanguageChanged);
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged, this, &EditorScreen::onCursorPositionChanged);
    connect(m_btnExport, &QPushButton::clicked, this, &EditorScreen::exportarJpg);
}

void EditorScreen::cargarDatos() {
    const int idx = m_languageSelector->findText(m_defaultLanguage);
    m_languageSelector->setCurrentIndex(idx >= 0 ? idx : 1);
    recrearValidador(m_languageSelector->currentText());
    m_lastLine = m_editor->textCursor().blockNumber();
}

bool EditorScreen::validarEstado() {
    const bool ok = (m_validador != nullptr);
    if (!ok) {
        m_diagnostico->setText("No se pudo inicializar el validador de sintaxis");
        m_diagnostico->setStyleSheet("color: #a11d1d; padding: 4px;");
    }
    return ok;
}

void EditorScreen::registrarEvento(const QString& descripcion) {
    Logger::instancia().registrar("[Editor] " + descripcion);
}

void EditorScreen::onLanguageChanged(const QString& language) {
    recrearValidador(language);
    registrarEvento("Lenguaje seleccionado: " + language);
    m_diagnostico->setText("  > validador activo: " + language);
        m_diagnostico->setStyleSheet("background-color: #111827; border-top: 1px solid #1e3a5f; color: #3b82f6; padding: 0 12px; font-family: 'Consolas', monospace; font-size: 11px;");
}

void EditorScreen::onCursorPositionChanged() {
    const int currentLine = m_editor->textCursor().blockNumber();
    if (currentLine != m_lastLine) {
        validarLinea(m_lastLine);
        m_lastLine = currentLine;
    }
}

void EditorScreen::recrearValidador(const QString& language) {
    if (language == "C++") {
        m_validador = std::make_unique<ValidadorCpp>();
    } else if (language == "Java") {
        m_validador = std::make_unique<ValidadorJava>();
    } else {
        m_validador = std::make_unique<ValidadorPython>();
    }
}

void EditorScreen::validarLinea(int lineNumber) {
    if (!m_validador || lineNumber < 0) {
        return;
    }

    QTextBlock block = m_editor->document()->findBlockByNumber(lineNumber);
    if (!block.isValid()) {
        return;
    }

    const ResultadoValidacion r = m_validador->validarLinea(block.text());
    if (!r.valido) {
        m_diagnostico->setText("  > ERROR ln:" + QString::number(lineNumber + 1) + "  " + r.diagnostico);
        m_diagnostico->setStyleSheet("background-color: #111827; border-top: 1px solid #1e3a5f; color: #ef4444; padding: 0 12px; font-family: 'Consolas', monospace; font-size: 11px;");
        resaltarLineaError(lineNumber, true);
        registrarEvento("Validacion fallida en linea " + QString::number(lineNumber + 1) + ": " + r.diagnostico);
    } else {
        m_diagnostico->setText("  > OK  ln:" + QString::number(lineNumber + 1) + "  sintaxis valida");
        m_diagnostico->setStyleSheet("background-color: #111827; border-top: 1px solid #1e3a5f; color: #10b981; padding: 0 12px; font-family: 'Consolas', monospace; font-size: 11px;");
        resaltarLineaError(lineNumber, false);
        registrarEvento("Validacion correcta en linea " + QString::number(lineNumber + 1));
    }
}

void EditorScreen::resaltarLineaError(int lineNumber, bool activa) {
    QList<QTextEdit::ExtraSelection> extras;

    if (activa) {
        QTextBlock block = m_editor->document()->findBlockByNumber(lineNumber);
        if (block.isValid()) {
            QTextEdit::ExtraSelection sel;
            sel.cursor = QTextCursor(block);
            sel.format.setBackground(QColor(80, 15, 15));
            sel.format.setForeground(QColor(255, 200, 200));
            sel.format.setProperty(QTextFormat::FullWidthSelection, true);
            extras.append(sel);
        }
    }

    m_editor->setExtraSelections(extras);
}

QString EditorScreen::rutaExportacionFinal() const {
    QString out = m_exportPath.trimmed();
    if (out.isEmpty()) {
        out = "exports/codigo_exportado.jpg";
    }
    return out;
}

void EditorScreen::exportarJpg() {
    const QString defaultPath = rutaExportacionFinal();
    const QString filePath = QFileDialog::getSaveFileName(this, "Exportar a JPG", defaultPath, "Imagen JPG (*.jpg *.jpeg)");
    if (filePath.isEmpty()) {
        registrarEvento("Exportacion cancelada por el usuario");
        return;
    }

    const QString texto = m_editor->toPlainText();
    QStringList lineas = texto.split('\n');
    if (lineas.isEmpty()) {
        lineas << "";
    }

    QFont font = m_editor->font();
    QFontMetrics metrics(font);

    int maxWidth = 0;
    for (const QString& linea : lineas) {
        maxWidth = qMax(maxWidth, metrics.horizontalAdvance(linea));
    }

    const int margen = 30;
    const int ancho = qMax(800, maxWidth + (margen * 2));
    const int alto = qMax(500, (lineas.size() * metrics.lineSpacing()) + (margen * 2));

    QImage imagen(ancho, alto, QImage::Format_RGB32);
    imagen.fill(Qt::white);

    QPainter painter(&imagen);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setFont(font);
    painter.setPen(Qt::black);

    int y = margen + metrics.ascent();
    for (const QString& linea : lineas) {
        painter.drawText(margen, y, linea);
        y += metrics.lineSpacing();
    }

    painter.end();

    QFileInfo info(filePath);
    QDir().mkpath(info.absolutePath());

    if (imagen.save(filePath, "JPG", 95)) {
        m_diagnostico->setText("  > EXPORTADO: " + filePath);
        m_diagnostico->setStyleSheet("background-color: #111827; border-top: 1px solid #1e3a5f; color: #10b981; padding: 0 12px; font-family: 'Consolas', monospace; font-size: 11px;");
        registrarEvento("Exportacion exitosa a " + filePath);
    } else {
        m_diagnostico->setText("  > ERROR: fallo al exportar JPG");
        m_diagnostico->setStyleSheet("background-color: #111827; border-top: 1px solid #1e3a5f; color: #ef4444; padding: 0 12px; font-family: 'Consolas', monospace; font-size: 11px;");
        registrarEvento("Fallo al exportar JPG");
    }
}

void EditorScreen::keyPressEvent(QKeyEvent* event) {
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_S) {
        registrarEvento("Atajo Ctrl+S en Editor");
        exportarJpg();
        return;
    }
    Pantalla::keyPressEvent(event);
}

void EditorScreen::mousePressEvent(QMouseEvent* event) {
    registrarEvento("Click en Editor para posicionar cursor");
    Pantalla::mousePressEvent(event);
}

void EditorScreen::resizeEvent(QResizeEvent* event) {
    registrarEvento("Resize Editor a " + QString::number(event->size().width()) + "x" + QString::number(event->size().height()));
    Pantalla::resizeEvent(event);
}

void EditorScreen::closeEvent(QCloseEvent* event) {
    const auto respuesta = QMessageBox::question(
        this,
        "Salir",
        "Desea salir? Puede exportar antes de cerrar.",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::No);

    if (respuesta == QMessageBox::Cancel) {
        registrarEvento("Cierre cancelado en Editor");
        event->ignore();
        return;
    }

    if (respuesta == QMessageBox::No) {
        exportarJpg();
    }

    registrarEvento("Cierre aceptado en Editor");
    event->accept();
}

void EditorScreen::focusInEvent(QFocusEvent* event) {
    registrarEvento("Editor obtuvo foco");
    Pantalla::focusInEvent(event);
}

void EditorScreen::focusOutEvent(QFocusEvent* event) {
    registrarEvento("Editor perdio foco y dispara validacion");
    validarLinea(m_editor->textCursor().blockNumber());
    Pantalla::focusOutEvent(event);
}
