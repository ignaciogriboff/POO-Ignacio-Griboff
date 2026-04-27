#pragma once

#include <QMainWindow>
#include "monitorservice.h"

class QLabel;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QPushButton;
class QPlainTextEdit;
class QCloseEvent;
class QProgressBar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onRefreshClicked();
    void onIntervalChanged(int v);

    void onMetricsUpdated(const VpsMetrics &m);
    void onRequestStateChanged(bool inFlight);

private:
    enum class PanelState { Ok, Alert, Down };

    void applyGrafanaDarkTheme();
    QWidget* createCard(const QString &title, QWidget *content);

    void loadSettings();
    void saveSettings() const;

    PanelState computeState(const VpsMetrics &m, QString *reasonOut = nullptr) const;
    void setStatus(PanelState st, const QString &reason = {});
    void appendEvent(const QString &line);

    // UI - conexión/config
    QLineEdit *m_leUrl = nullptr;
    QSpinBox *m_sbInterval = nullptr;
    QPushButton *m_btnRefresh = nullptr;

    QDoubleSpinBox *m_sbLoadWarn = nullptr;
    QSpinBox *m_sbMemWarn = nullptr;
    QSpinBox *m_sbDiskWarn = nullptr;

    // UI - estado
    QLabel *m_lblStatusBadge = nullptr;
    QLabel *m_lblStatusDetail = nullptr;
    QLabel *m_lblLastCheck = nullptr;

    // UI - métricas
    QLabel *m_lblUptime = nullptr;
    QLabel *m_lblLoad = nullptr;

    QLabel *m_lblMemText = nullptr;
    QProgressBar *m_pbMem = nullptr;

    QLabel *m_lblDiskText = nullptr;
    QProgressBar *m_pbDisk = nullptr;

    // UI - historial
    QPlainTextEdit *m_txtEvents = nullptr;

    // Logic
    MonitorService m_service;

    // Historial por transiciones
    PanelState m_lastState = PanelState::Down;
    QString m_lastAlertReason;
};