

#ifndef GENERALMODEL_H
#define GENERALMODEL_H

#include <QAbstractTableModel>

#include "general.h"

class GeneralModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ColumnType
    {
        TitleColumn,
        NameColumn,
        KingdomColumn,
        GenderColumn,
        MaxHpColumn,
        PackageColumn,

        ColumnTypesCount
    };

    explicit GeneralModel(const QMap<const General *, int> &list, QList<const General *> &keepOrderList);

    virtual int columnCount(const QModelIndex &parent) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    inline QMap<const General *, int> *generalMap()
    {
        return &all_generals;
    }

    QModelIndex firstIndex();

private:
    QMap<const General *, int> all_generals;
    QList<const General*> keep_order_list;
};

#endif // GENERALMODEL_H
