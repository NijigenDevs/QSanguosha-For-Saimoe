

#ifndef AVATARMODEL_H
#define AVATARMODEL_H

#include <QAbstractListModel>

#include "engine.h"
#include "general.h"

class AvatarModel : public QAbstractListModel
{
    Q_OBJECT
public:

    explicit AvatarModel(const GeneralList &list);

    virtual int rowCount(const QModelIndex &) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

private:
    GeneralList list;
};

#endif // AVATARMODEL_H
