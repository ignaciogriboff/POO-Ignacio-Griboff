#include "ventana.h"
#include "ui_ventana.h"

#include <QPalette>
#include <QResizeEvent>

Ventana::Ventana(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Ventana)
{
    ui->setupUi(this);
}

Ventana::~Ventana()
{
    delete ui;
}

void Ventana::setBackground(const QPixmap &pm)
{
    m_bgOriginal = pm;
    applyBackground();
}

void Ventana::applyBackground()
{
    if (m_bgOriginal.isNull())
        return;

    QPixmap scaled = m_bgOriginal.scaled(size(),
                                         Qt::KeepAspectRatioByExpanding,
                                         Qt::SmoothTransformation);

    QPalette pal = palette();
    pal.setBrush(QPalette::Window, scaled);
    setAutoFillBackground(true);
    setPalette(pal);
}

void Ventana::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyBackground();
}