#pragma once
#include <QMainWindow>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QStatusBar>
#include "models.h"
#include "api_client.h"
#include "ws_client.h"

class ColumnWidget;

/**
 * MainWindow — The root Kanban board window.
 * Loads config from config.json, initialises ApiClient and WsClient,
 * and renders the board as a scrollable horizontal list of ColumnWidgets.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // REST responses
    void onColumnsFetched(const QList<Column> &columns);
    void onColumnCreated(const Column &col);
    void onColumnUpdated(const Column &col);
    void onColumnDeleted(int id);
    void onCardCreated(const Card &card);
    void onCardUpdated(const Card &card);
    void onCardDeleted(int id);
    void onCardMoved(const Card &card);
    void onCardReordered(int columnId, int cardId, int toPosition);
    void onApiError(const QString &msg);

    // WebSocket events
    void onWsEvent(const QString &type, const QJsonObject &data);
    void onWsConnected();
    void onWsDisconnected();
    void onWsError(const QString &msg);

    // Toolbar actions
    void onAddColumnClicked();

private:
    void loadConfig();
    void buildToolbar();
    void rebuildBoard();
    void refreshBoard();   // full GET /columns
    void addColumnWidget(const Column &col);
    void clearBoard();

    // Model helpers
    int findColumnIndex(int colId) const;
    int findCardIndex(int colId, int cardId) const;

    // UI
    QScrollArea *m_scrollArea = nullptr;
    QWidget     *m_boardWidget = nullptr;
    QHBoxLayout *m_boardLayout = nullptr;

    // Networking
    ApiClient *m_api = nullptr;
    WsClient  *m_ws  = nullptr;

    // Config
    QString m_baseUrl;
    QString m_wsBaseUrl;
    QString m_user;
    QString m_pass;

    // In-memory model
    QList<Column> m_columns;
};
