#include "mainwindow.h"
#include <QToolBar>
#include <QAction>
#include <QScrollArea>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStatusBar>
#include <QInputDialog>
#include <QCoreApplication>

#include "dialogs/columndialog.h"
#include "dialogs/carddialog.h"

// ─── ColumnWidget ─────────────────────────────────────────────────────────────
// Inner widget representing a single column on the board.

class ColumnWidget : public QGroupBox
{
    Q_OBJECT
public:
    ColumnWidget(const Column &col, ApiClient *api, QWidget *parent = nullptr)
        : QGroupBox(col.title, parent)
        , m_col(col)
        , m_api(api)
    {
        setMinimumWidth(250);
        setMaximumWidth(300);

        auto *vlay = new QVBoxLayout(this);

        m_list = new QListWidget(this);
        m_list->setDragDropMode(QAbstractItemView::InternalMove);
        m_list->setDefaultDropAction(Qt::MoveAction);
        m_list->setSelectionMode(QAbstractItemView::SingleSelection);
        vlay->addWidget(m_list);

        // Detect drag-drop reorder
        connect(m_list->model(), &QAbstractItemModel::rowsMoved,
                this, &ColumnWidget::onRowsMoved);

        // ── Buttons bar ──
        auto *btnBar = new QWidget(this);
        auto *hlay   = new QHBoxLayout(btnBar);
        hlay->setContentsMargins(0, 0, 0, 0);

        auto *btnAdd    = new QPushButton(tr("Add Card"), btnBar);
        auto *btnEdit   = new QPushButton(tr("Edit"), btnBar);
        auto *btnDelCol = new QPushButton(tr("Del Col"), btnBar);
        auto *btnDelCard= new QPushButton(tr("Del Card"), btnBar);
        auto *btnMoveL  = new QPushButton(tr("◀"), btnBar);
        auto *btnMoveR  = new QPushButton(tr("▶"), btnBar);
        auto *btnUp     = new QPushButton(tr("↑"), btnBar);
        auto *btnDown   = new QPushButton(tr("↓"), btnBar);

        hlay->addWidget(btnAdd);
        hlay->addWidget(btnEdit);
        hlay->addWidget(btnDelCol);
        hlay->addWidget(btnDelCard);
        vlay->addWidget(btnBar);

        auto *btnBar2 = new QWidget(this);
        auto *hlay2   = new QHBoxLayout(btnBar2);
        hlay2->setContentsMargins(0, 0, 0, 0);
        hlay2->addWidget(btnMoveL);
        hlay2->addWidget(btnMoveR);
        hlay2->addWidget(btnUp);
        hlay2->addWidget(btnDown);
        vlay->addWidget(btnBar2);

        connect(btnAdd,     &QPushButton::clicked, this, &ColumnWidget::onAddCard);
        connect(btnEdit,    &QPushButton::clicked, this, &ColumnWidget::onEditCard);
        connect(btnDelCol,  &QPushButton::clicked, this, &ColumnWidget::onDeleteColumn);
        connect(btnDelCard, &QPushButton::clicked, this, &ColumnWidget::onDeleteCard);
        connect(btnMoveL,   &QPushButton::clicked, this, &ColumnWidget::onMoveCardLeft);
        connect(btnMoveR,   &QPushButton::clicked, this, &ColumnWidget::onMoveCardRight);
        connect(btnUp,      &QPushButton::clicked, this, &ColumnWidget::onMoveCardUp);
        connect(btnDown,    &QPushButton::clicked, this, &ColumnWidget::onMoveCardDown);

        // Context menu on double-click to edit card
        connect(m_list, &QListWidget::itemDoubleClicked, this, &ColumnWidget::onEditCard);

        populateList();
    }

    void updateColumn(const Column &col)
    {
        m_col = col;
        setTitle(col.title);
        populateList();
    }

    int columnId() const { return m_col.id; }

    // Called when a card has been moved OUT of this column
    void removeCard(int cardId)
    {
        for (int i = 0; i < m_col.cards.size(); ++i) {
            if (m_col.cards[i].id == cardId) {
                m_col.cards.removeAt(i);
                break;
            }
        }
        populateList();
    }

    // Called when a card arrives in this column from outside (card_moved event)
    void addCard(const Card &card)
    {
        // Remove any existing entry for this card
        for (int i = 0; i < m_col.cards.size(); ++i) {
            if (m_col.cards[i].id == card.id) {
                m_col.cards.removeAt(i);
                break;
            }
        }
        m_col.cards.append(card);
        std::sort(m_col.cards.begin(), m_col.cards.end(),
                  [](const Card &a, const Card &b) { return a.position < b.position; });
        populateList();
    }

    void updateCardInList(const Card &card)
    {
        for (auto &c : m_col.cards) {
            if (c.id == card.id) {
                c.title = card.title;
                c.description = card.description;
                break;
            }
        }
        populateList();
    }

    // Provide neighbour column IDs for move left/right
    void setNeighbours(int leftColId, int rightColId)
    {
        m_leftColId  = leftColId;
        m_rightColId = rightColId;
    }

signals:
    void columnDeleteRequested(int colId);
    void requestRefresh();

private slots:
    void onAddCard()
    {
        CardDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted) return;
        m_api->createCard(m_col.id, dlg.title(), dlg.description());
    }

    void onEditCard()
    {
        auto *item = m_list->currentItem();
        if (!item) { QMessageBox::information(this, tr("Info"), tr("Select a card first.")); return; }
        int cardId = item->data(Qt::UserRole).toInt();
        Card *card = findCard(cardId);
        if (!card) return;

        CardDialog dlg(this, card->title, card->description);
        if (dlg.exec() != QDialog::Accepted) return;
        m_api->updateCard(cardId, dlg.title(), dlg.description());
    }

    void onDeleteColumn()
    {
        if (QMessageBox::question(this, tr("Confirm"),
                                  tr("Delete column \"%1\" and all its cards?").arg(m_col.title))
            == QMessageBox::Yes)
        {
            emit columnDeleteRequested(m_col.id);
        }
    }

    void onDeleteCard()
    {
        auto *item = m_list->currentItem();
        if (!item) { QMessageBox::information(this, tr("Info"), tr("Select a card first.")); return; }
        int cardId = item->data(Qt::UserRole).toInt();
        if (QMessageBox::question(this, tr("Confirm"), tr("Delete this card?")) == QMessageBox::Yes)
            m_api->deleteCard(cardId);
    }

    void onMoveCardLeft()
    {
        if (m_leftColId <= 0) { QMessageBox::information(this, tr("Info"), tr("No column to the left.")); return; }
        auto *item = m_list->currentItem();
        if (!item) { QMessageBox::information(this, tr("Info"), tr("Select a card first.")); return; }
        int cardId = item->data(Qt::UserRole).toInt();
        m_api->moveCard(cardId, m_leftColId, 0);
    }

    void onMoveCardRight()
    {
        if (m_rightColId <= 0) { QMessageBox::information(this, tr("Info"), tr("No column to the right.")); return; }
        auto *item = m_list->currentItem();
        if (!item) { QMessageBox::information(this, tr("Info"), tr("Select a card first.")); return; }
        int cardId = item->data(Qt::UserRole).toInt();
        m_api->moveCard(cardId, m_rightColId, 0);
    }

    void onMoveCardUp()
    {
        auto *item = m_list->currentItem();
        if (!item) return;
        int row = m_list->row(item);
        if (row <= 0) return;
        int cardId = item->data(Qt::UserRole).toInt();
        m_api->reorderCard(m_col.id, cardId, row - 1);
    }

    void onMoveCardDown()
    {
        auto *item = m_list->currentItem();
        if (!item) return;
        int row = m_list->row(item);
        if (row >= m_list->count() - 1) return;
        int cardId = item->data(Qt::UserRole).toInt();
        m_api->reorderCard(m_col.id, cardId, row + 1);
    }

    void onRowsMoved(const QModelIndex & /*parent*/, int start, int /*end*/,
                     const QModelIndex & /*dest*/, int row)
    {
        // Drag-drop reorder within the list widget
        if (start == row || start == row - 1) return;
        auto *item = m_list->item(row > start ? row - 1 : row);
        if (!item) return;
        int cardId  = item->data(Qt::UserRole).toInt();
        int newPos  = row > start ? row - 1 : row;
        m_api->reorderCard(m_col.id, cardId, newPos);
    }

private:
    void populateList()
    {
        m_list->clear();
        for (const Card &card : m_col.cards) {
            auto *item = new QListWidgetItem(card.title);
            item->setData(Qt::UserRole, card.id);
            if (!card.description.isEmpty())
                item->setToolTip(card.description);
            m_list->addItem(item);
        }
    }

    Card *findCard(int cardId)
    {
        for (auto &c : m_col.cards)
            if (c.id == cardId) return &c;
        return nullptr;
    }

    Column       m_col;
    ApiClient   *m_api;
    QListWidget *m_list = nullptr;
    int          m_leftColId  = -1;
    int          m_rightColId = -1;
};

// ─── MainWindow ───────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_api(new ApiClient(this))
    , m_ws(new WsClient(this))
{
    setWindowTitle(tr("Kanban Board"));
    resize(1200, 700);

    loadConfig();
    buildToolbar();

    // Scroll area for columns
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setCentralWidget(m_scrollArea);

    m_boardWidget = new QWidget;
    m_boardLayout = new QHBoxLayout(m_boardWidget);
    m_boardLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_boardLayout->setSpacing(12);
    m_boardLayout->setContentsMargins(12, 12, 12, 12);
    m_boardWidget->setLayout(m_boardLayout);
    m_scrollArea->setWidget(m_boardWidget);

    // Wire ApiClient signals
    connect(m_api, &ApiClient::columnsFetched,  this, &MainWindow::onColumnsFetched);
    connect(m_api, &ApiClient::columnCreated,   this, &MainWindow::onColumnCreated);
    connect(m_api, &ApiClient::columnUpdated,   this, &MainWindow::onColumnUpdated);
    connect(m_api, &ApiClient::columnDeleted,   this, &MainWindow::onColumnDeleted);
    connect(m_api, &ApiClient::cardCreated,     this, &MainWindow::onCardCreated);
    connect(m_api, &ApiClient::cardUpdated,     this, &MainWindow::onCardUpdated);
    connect(m_api, &ApiClient::cardDeleted,     this, &MainWindow::onCardDeleted);
    connect(m_api, &ApiClient::cardMoved,       this, &MainWindow::onCardMoved);
    connect(m_api, &ApiClient::cardReordered,   this, &MainWindow::onCardReordered);
    connect(m_api, &ApiClient::errorOccurred,   this, &MainWindow::onApiError);

    // Wire WsClient signals
    connect(m_ws, &WsClient::wsEvent,      this, &MainWindow::onWsEvent);
    connect(m_ws, &WsClient::connected,    this, &MainWindow::onWsConnected);
    connect(m_ws, &WsClient::disconnected, this, &MainWindow::onWsDisconnected);
    connect(m_ws, &WsClient::wsError,      this, &MainWindow::onWsError);

    // Initial load
    m_api->fetchColumns();

    // Connect WebSocket
    m_ws->connectToServer(m_wsBaseUrl, m_user, m_pass);
}

MainWindow::~MainWindow()
{
    m_ws->disconnect();
}

void MainWindow::loadConfig()
{
    QFile file(QCoreApplication::applicationDirPath() + "/config.json");
    if (!file.open(QIODevice::ReadOnly)) {
        // Use defaults if config not found
        m_baseUrl   = "http://173.212.234.190:8000";
        m_wsBaseUrl = "ws://173.212.234.190:8000";
        m_user      = "admin";
        m_pass      = "admin123";
    } else {
        QJsonObject cfg = QJsonDocument::fromJson(file.readAll()).object();
        m_baseUrl   = cfg["api_base_url"].toString("http://173.212.234.190:8000");
        m_user      = cfg["basic_user"].toString("admin");
        m_pass      = cfg["basic_pass"].toString("admin123");
        // Derive WS URL from HTTP URL
        m_wsBaseUrl = m_baseUrl;
        m_wsBaseUrl.replace("http://", "ws://").replace("https://", "wss://");
    }
    m_api->setConfig(m_baseUrl, m_user, m_pass);
    statusBar()->showMessage(tr("Connecting to %1 ...").arg(m_baseUrl));
}

void MainWindow::buildToolbar()
{
    auto *toolbar = addToolBar(tr("Actions"));
    auto *actAdd  = toolbar->addAction(tr("+ Add Column"));
    auto *actRefresh = toolbar->addAction(tr("⟳ Refresh"));
    connect(actAdd,     &QAction::triggered, this, &MainWindow::onAddColumnClicked);
    connect(actRefresh, &QAction::triggered, this, &MainWindow::refreshBoard);
}

void MainWindow::onAddColumnClicked()
{
    ColumnDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    m_api->createColumn(dlg.title());
}

void MainWindow::refreshBoard()
{
    m_api->fetchColumns();
}

// ── REST slots ────────────────────────────────────────────────────────────────

void MainWindow::onColumnsFetched(const QList<Column> &columns)
{
    m_columns = columns;
    std::sort(m_columns.begin(), m_columns.end(),
              [](const Column &a, const Column &b) { return a.position < b.position; });
    rebuildBoard();
    statusBar()->showMessage(tr("Board loaded. %1 column(s).").arg(m_columns.size()));
}

void MainWindow::onColumnCreated(const Column &col)
{
    m_columns.append(col);
    std::sort(m_columns.begin(), m_columns.end(),
              [](const Column &a, const Column &b) { return a.position < b.position; });
    rebuildBoard();
}

void MainWindow::onColumnUpdated(const Column &col)
{
    int idx = findColumnIndex(col.id);
    if (idx >= 0) m_columns[idx] = col;
    std::sort(m_columns.begin(), m_columns.end(),
              [](const Column &a, const Column &b) { return a.position < b.position; });
    rebuildBoard();
}

void MainWindow::onColumnDeleted(int id)
{
    int idx = findColumnIndex(id);
    if (idx >= 0) m_columns.removeAt(idx);
    rebuildBoard();
}

void MainWindow::onCardCreated(const Card &card)
{
    int idx = findColumnIndex(card.columnId);
    if (idx < 0) return;
    m_columns[idx].cards.append(card);
    std::sort(m_columns[idx].cards.begin(), m_columns[idx].cards.end(),
              [](const Card &a, const Card &b) { return a.position < b.position; });
    // Update widget
    auto *w = findChild<ColumnWidget*>(QString("col_%1").arg(card.columnId));
    if (w) w->updateColumn(m_columns[idx]);
}

void MainWindow::onCardUpdated(const Card &card)
{
    int idx = findColumnIndex(card.columnId);
    if (idx < 0) return;
    for (auto &c : m_columns[idx].cards)
        if (c.id == card.id) { c = card; break; }
    auto *w = findChild<ColumnWidget*>(QString("col_%1").arg(card.columnId));
    if (w) w->updateCardInList(card);
}

void MainWindow::onCardDeleted(int id)
{
    for (auto &col : m_columns) {
        for (int i = 0; i < col.cards.size(); ++i) {
            if (col.cards[i].id == id) {
                col.cards.removeAt(i);
                auto *w = findChild<ColumnWidget*>(QString("col_%1").arg(col.id));
                if (w) w->updateColumn(col);
                return;
            }
        }
    }
}

void MainWindow::onCardMoved(const Card &card)
{
    // Remove card from old column in model
    for (auto &col : m_columns) {
        for (int i = 0; i < col.cards.size(); ++i) {
            if (col.cards[i].id == card.id && col.id != card.columnId) {
                col.cards.removeAt(i);
                auto *w = findChild<ColumnWidget*>(QString("col_%1").arg(col.id));
                if (w) w->updateColumn(col);
                break;
            }
        }
    }
    // Add to new column
    int idx = findColumnIndex(card.columnId);
    if (idx < 0) return;
    // Remove any stale entry for same card
    for (int i = 0; i < m_columns[idx].cards.size(); ++i)
        if (m_columns[idx].cards[i].id == card.id) { m_columns[idx].cards.removeAt(i); break; }
    m_columns[idx].cards.append(card);
    std::sort(m_columns[idx].cards.begin(), m_columns[idx].cards.end(),
              [](const Card &a, const Card &b) { return a.position < b.position; });
    auto *w = findChild<ColumnWidget*>(QString("col_%1").arg(card.columnId));
    if (w) w->updateColumn(m_columns[idx]);
}

void MainWindow::onCardReordered(int columnId, int cardId, int toPosition)
{
    int idx = findColumnIndex(columnId);
    if (idx < 0) return;
    // Move card in local list
    auto &cards = m_columns[idx].cards;
    int fromPos = -1;
    for (int i = 0; i < cards.size(); ++i)
        if (cards[i].id == cardId) { fromPos = i; break; }
    if (fromPos < 0) return;
    Card card = cards.takeAt(fromPos);
    int dest = qBound(0, toPosition, cards.size());
    cards.insert(dest, card);
    for (int i = 0; i < cards.size(); ++i) cards[i].position = i;
    auto *w = findChild<ColumnWidget*>(QString("col_%1").arg(columnId));
    if (w) w->updateColumn(m_columns[idx]);
}

void MainWindow::onApiError(const QString &msg)
{
    QMessageBox::warning(this, tr("API Error"), msg);
    statusBar()->showMessage(tr("Error: %1").arg(msg));
}

// ── WebSocket slots ───────────────────────────────────────────────────────────

void MainWindow::onWsConnected()
{
    statusBar()->showMessage(tr("WebSocket connected."));
}

void MainWindow::onWsDisconnected()
{
    statusBar()->showMessage(tr("WebSocket disconnected. Board may be out of sync."));
}

void MainWindow::onWsError(const QString &msg)
{
    statusBar()->showMessage(tr("WS error: %1").arg(msg));
}

void MainWindow::onWsEvent(const QString &type, const QJsonObject &data)
{
    // server_wins strategy: on snapshot or unknown events, do full refresh
    if (type == "snapshot" || type == "error") {
        refreshBoard();
        return;
    }

    auto toCard = [](const QJsonObject &o) -> Card {
        Card c;
        c.id          = o["id"].toInt();
        c.columnId    = o["column_id"].toInt();
        c.title       = o["title"].toString();
        c.description = o["description"].toString();
        c.position    = o["position"].toInt();
        return c;
    };

    auto toColumn = [](const QJsonObject &o) -> Column {
        Column col;
        col.id       = o["id"].toInt();
        col.title    = o["title"].toString();
        col.position = o["position"].toInt();
        return col;
    };

    if (type == "column_created") {
        onColumnCreated(toColumn(data));
    } else if (type == "column_updated") {
        // Refresh full to get cards too
        refreshBoard();
    } else if (type == "column_deleted") {
        onColumnDeleted(data["id"].toInt());
    } else if (type == "card_created") {
        onCardCreated(toCard(data));
    } else if (type == "card_updated") {
        onCardUpdated(toCard(data));
    } else if (type == "card_deleted") {
        onCardDeleted(data["id"].toInt());
    } else if (type == "card_moved") {
        onCardMoved(toCard(data));
    } else if (type == "card_reordered") {
        onCardReordered(data["column_id"].toInt(),
                        data["card_id"].toInt(),
                        data["to_position"].toInt());
    } else {
        // Unknown event — full refresh
        refreshBoard();
    }
}

// ── Board rendering ───────────────────────────────────────────────────────────

void MainWindow::clearBoard()
{
    QLayoutItem *item;
    while ((item = m_boardLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void MainWindow::rebuildBoard()
{
    clearBoard();
    for (const Column &col : m_columns)
        addColumnWidget(col);
}

void MainWindow::addColumnWidget(const Column &col)
{
    auto *w = new ColumnWidget(col, m_api, m_boardWidget);
    w->setObjectName(QString("col_%1").arg(col.id));
    connect(w, &ColumnWidget::columnDeleteRequested, this, [this](int colId) {
        m_api->deleteColumn(colId);
    });
    m_boardLayout->addWidget(w);

    // Update neighbour references for all widgets
    int count = m_boardLayout->count();
    for (int i = 0; i < count; ++i) {
        auto *cw = qobject_cast<ColumnWidget*>(m_boardLayout->itemAt(i)->widget());
        if (!cw) continue;
        int leftId  = (i > 0)
            ? qobject_cast<ColumnWidget*>(m_boardLayout->itemAt(i-1)->widget())->columnId()
            : -1;
        int rightId = (i < count - 1)
            ? qobject_cast<ColumnWidget*>(m_boardLayout->itemAt(i+1)->widget())->columnId()
            : -1;
        cw->setNeighbours(leftId, rightId);
    }
}

// ── Model helpers ─────────────────────────────────────────────────────────────

int MainWindow::findColumnIndex(int colId) const
{
    for (int i = 0; i < m_columns.size(); ++i)
        if (m_columns[i].id == colId) return i;
    return -1;
}

int MainWindow::findCardIndex(int colId, int cardId) const
{
    int ci = findColumnIndex(colId);
    if (ci < 0) return -1;
    const auto &cards = m_columns[ci].cards;
    for (int i = 0; i < cards.size(); ++i)
        if (cards[i].id == cardId) return i;
    return -1;
}

// Required for Q_OBJECT in a .cpp-defined class (ColumnWidget)
#include "mainwindow.moc"
