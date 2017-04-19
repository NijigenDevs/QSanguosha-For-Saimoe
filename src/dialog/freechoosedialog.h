#ifndef FREECHOOSEDIALOG_H
#define FREECHOOSEDIALOG_H

#include "flatdialog.h"

class General;
class QButtonGroup;
class QAbstractButton;

class FreeChooseDialog : public FlatDialog
{
    Q_OBJECT
        Q_ENUMS(ButtonGroupType)

public:
    enum ButtonGroupType
    {
        Exclusive, Pair, Multi
    };

    explicit FreeChooseDialog(QWidget *parent, ButtonGroupType type = Exclusive);

private:
    QButtonGroup *group;
    ButtonGroupType type;
    QWidget *createTab(const QList<const General *> &generals);

    private slots:
    void chooseGeneral();
    void disableButtons(QAbstractButton *button);

signals:
    void general_chosen(const QString &name);
    void pair_chosen(const QString &first, const QString &second);
};

#endif // FREECHOOSEDIALOG_H

