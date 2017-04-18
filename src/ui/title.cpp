

#include "title.h"
#include "skinbank.h"

Title::Title(QGraphicsObject *parent, const QString &text, const QString &font_name, const int &font_size)
    : QGraphicsObject(parent), text(text), font_name(font_name), font_size(font_size)
{
}

QRectF Title::boundingRect() const
{
    return QRectF(0, 0, font_size * text.length(), font_size + 1);
}

void Title::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QColor textColor = Qt::white;
    IQSanComponentSkin::QSanSimpleTextFont ft;
    JsonArray val;
    val << font_name;
    val << font_size;
    val << 2;

    JsonArray val3;
    val3 << textColor.red();
    val3 << textColor.green();
    val3 << textColor.blue();
    val << QVariant(val3);

    ft.tryParse(val);
    ft.paintText(painter, boundingRect().toRect(), Qt::AlignCenter, text);
}
