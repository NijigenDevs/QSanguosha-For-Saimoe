

#ifndef CHOOSESUITBOX_H
#define CHOOSESUITBOX_H

#include "graphicsbox.h"

class Button;
class QGraphicsProxyWidget;
class QSanCommandProgressBar;

class ChooseSuitBox : public GraphicsBox
{
    Q_OBJECT

public:
    ChooseSuitBox();

    QRectF boundingRect() const;

    void chooseSuit(const QStringList &suits);
    void clear();

private:
    int suitNumber;
    QStringList m_suits;
    QList<Button *> buttons;

    static const int outerBlankWidth;
    static const int buttonWidth;
    static const int buttonHeight;
    static const int interval;
    static const int topBlankWidth;
    static const int bottomBlankWidth;

    QGraphicsProxyWidget *progressBarItem;
    QSanCommandProgressBar *progressBar;

    void reply();
};

#endif // CHOOSESUITBOX_H
