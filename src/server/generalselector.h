#ifndef GENERALSELECTOR_H
#define GENERALSELECTOR_H

#include <QObject>
#include <QHash>
#include <QGenericMatrix>
#include <QStringList>

class ServerPlayer;

// singleton class
class GeneralSelector : public QObject
{
    Q_OBJECT

public:
    static GeneralSelector *getInstance();
    QStringList selectGenerals(ServerPlayer *player, const QStringList &candidates);
    inline void resetValues()
    {
        m_privatePairValueTable.clear();
    }

private:
    GeneralSelector();
    void loadGeneralTable();
    void loadPairTable();
    void calculatePairValues(const ServerPlayer *player, const QStringList &candidates);
    void calculateDeputyValue(const ServerPlayer *player, const QString &first, const QStringList &candidates, const QStringList &kingdom_list = QStringList());

    QHash<QString, int> m_singleGeneralTable;
    QHash<QString, int> m_pairTable;
    QHash<const ServerPlayer *, QHash<QString, int> > m_privatePairValueTable;
};

#endif // GENERALSELECTOR_H
