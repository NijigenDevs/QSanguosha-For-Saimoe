

#ifndef GRAPHICSBOX_H
#define GRAPHICSBOX_H

#include <QGraphicsObject>

class GraphicsBox : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit GraphicsBox(const QString &title = QString());
    virtual ~GraphicsBox();

    static void paintGraphicsBoxStyle(QPainter *painter, const QString &title, const QRectF &rect);
    static void stylize(QGraphicsObject *target);
    static void moveToCenter(QGraphicsObject *target);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual QRectF boundingRect() const = 0;

    virtual void paintLayout(QPainter *painter)
    {
        Q_UNUSED(painter)
    }

    void moveToCenter();
    void disappear();

    QString title;
};

#endif // GRAPHICSBOX_H
