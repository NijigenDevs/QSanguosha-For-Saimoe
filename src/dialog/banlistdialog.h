

#ifndef BANLISTDIALOG_H
#define BANLISTDIALOG_H

#include "flatdialog.h"

#include <QMap>

class QListWidget;

class BanListDialog : public FlatDialog
{
    Q_OBJECT

public:
    BanListDialog(QWidget *parent, bool view = false);

private:
    QList<QListWidget *>lists;
    QListWidget *list;
    int item;
    QStringList ban_list;
    QMap<QString, QStringList> banned_items;

    private slots:
    void addGeneral(const QString &name);
    void addPair(const QString &first, const QString &second);
    void doAddButton();
    void doRemoveButton();
    void save();
    void saveAll();
    void switchTo(int item);
};

#endif // BANLISTDIALOG_H
