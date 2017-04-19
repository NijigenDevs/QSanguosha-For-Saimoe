#ifndef _CHOOSE_OPTIONS_BOX_H
#define _CHOOSE_OPTIONS_BOX_H

#include "graphicsbox.h"

class Button;
class QSanCommandProgressBar;
class QGraphicsProxyWidget;

class ChooseOptionsBox : public GraphicsBox
{
    Q_OBJECT

public:
    explicit ChooseOptionsBox();

    QRectF boundingRect() const;

    inline void setSkillName(const QString &skillName)
    {
        this->skillName = skillName;
    }
    void clear();

    public slots:
    void chooseOption(const QStringList &options);
    void reply();

private:
    QStringList options;
    QString skillName;
    QList<Button *> buttons;
    static const int minButtonWidth = 100;
    static const int defaultButtonHeight = 30;
    static const int topBlankWidth = 42;
    static const int bottomBlankWidth = 25;
    static const int interval = 15;
    static const int outerBlankWidth = 37;

    QGraphicsProxyWidget *progressBarItem;
    QSanCommandProgressBar *progressBar;

    int getButtonWidth() const;

    QString translate(const QString &option) const;
};

#endif // _CHOOSE_OPTIONS_BOX_H
