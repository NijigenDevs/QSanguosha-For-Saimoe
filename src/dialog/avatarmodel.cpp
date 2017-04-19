#include "avatarmodel.h"
#include "skinbank.h"

AvatarModel::AvatarModel(const GeneralList &list)
    : list(list)
{
}

int AvatarModel::rowCount(const QModelIndex &) const
{
    return list.size();
}

QVariant AvatarModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= list.length())
        return QVariant();

    const General *general = list.at(row);

    switch (role)
    {
        case Qt::UserRole: return general->objectName();
        case Qt::DisplayRole: return Sanguosha->translate(general->objectName());
        case Qt::DecorationRole:
        {
            QIcon icon(G_ROOM_SKIN.getGeneralPixmap(general->objectName(),
                QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE));
            return icon;
        }
    }

    return QVariant();
}
