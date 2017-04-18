

#ifndef TITLE_H
#define TITLE_H

#include <QGraphicsObject>

class Title : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit Title(QGraphicsObject *parent, const QString &text, const QString &font_name, const int &font_size);
    virtual QRectF boundingRect() const;
    void setText(const QString &text)
    {
        this->text = text;
    }

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:

    QString text;
    QString font_name;
    int font_size;
};

#endif // TITLE_H
