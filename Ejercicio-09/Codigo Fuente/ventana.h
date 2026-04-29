#ifndef VENTANA_H
#define VENTANA_H

#include <QWidget>
#include <QPixmap>

QT_BEGIN_NAMESPACE
namespace Ui { class Ventana; }
QT_END_NAMESPACE

class Ventana : public QWidget
{
    Q_OBJECT

public:
    explicit Ventana(QWidget *parent = nullptr);
    ~Ventana();

    void setBackground(const QPixmap &pm);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void applyBackground();

    Ui::Ventana *ui = nullptr;
    QPixmap m_bgOriginal;
};

#endif // VENTANA_H