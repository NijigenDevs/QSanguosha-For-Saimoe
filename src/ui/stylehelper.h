#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QObject>
#include <QMutex>
#include <QFont>
#include <QColor>

#include "settings.h"

class QPushButton;

class StyleHelper : public QObject
{

private:
    explicit StyleHelper(QObject *parent = 0);
    QFont iconFont;
    static StyleHelper *instance;

public:
    static StyleHelper *getInstance();

    void setIcon(QPushButton *button, QChar iconId, int size = 10);

    static QFont getFontByFileName(const QString &fileName);

    inline static QString styleSheetOfTooltip()
    {
        return QString("QToolTip{ border: 1px solid rgb(166, 150, 122); padding: 1px; "
            "border-radius: 3px; background: %1; opacity: 230; }")
            .arg(Config.ToolTipBackgroundColor.name());
    }

    static QString styleSheetOfScrollBar();

    static QColor backgroundColorOfFlatDialog();
};

#endif // STYLEHELPER_H
