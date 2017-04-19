#ifndef WINDOW_H
#define WINDOW_H

#include <QGraphicsScale>
#include <QGraphicsObject>
#include "button.h"

class Window : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit Window(const QString &title, const QSizeF &size, const QString &path = QString());
    ~Window();

    void addContent(const QString &content);
    Button *addCloseButton(const QString &label);
    void shift(qreal pos_x = 0, qreal pos_y = 0);
    void shift(const QPointF &pos);
    void keepWhenDisappear();
    void setTitle(const QString &title);

    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    public slots:
    void appear();
    void disappear();

private:
    QGraphicsTextItem *titleItem;
    QGraphicsScale *scaleTransform;
    QSizeF size;
    bool keep_when_disappear;
    QPixmap *bg;
    QImage *outimg;
};

#endif // WINDOW_H

