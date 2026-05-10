#pragma once

#include "pantalla.h"
#include "validadores.h"

#include <memory>

class QLabel;
class QComboBox;
class QPlainTextEdit;
class QPushButton;

class EditorScreen final : public Pantalla {
    Q_OBJECT

public:
    explicit EditorScreen(const QString& defaultLanguage, const QString& exportPath, QWidget* parent = nullptr);

    void inicializarUI() override;
    void conectarEventos() override;
    void cargarDatos() override;
    bool validarEstado() override;
    void registrarEvento(const QString& descripcion) override;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void onLanguageChanged(const QString& language);
    void onCursorPositionChanged();
    void exportarJpg();

private:
    void recrearValidador(const QString& language);
    void validarLinea(int lineNumber);
    void resaltarLineaError(int lineNumber, bool activa);
    QString rutaExportacionFinal() const;

    QString m_defaultLanguage;
    QString m_exportPath;
    int m_lastLine;

    QComboBox* m_languageSelector;
    QPlainTextEdit* m_editor;
    QLabel* m_diagnostico;
    QPushButton* m_btnExport;
    QLabel* m_curriculum;

    std::unique_ptr<ValidadorSintaxis> m_validador;
};
