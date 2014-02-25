#ifndef _SCENARIO_H
#define _SCENARIO_H

#include "package.h"
#include "ai.h"

class Room;
class ScenarioRule;

#include <QMap>

class Scenario: public Package {
    Q_OBJECT

public:
    explicit Scenario(const QString &name);
    ScenarioRule *getRule() const;

    virtual bool exposeRoles() const;
    virtual int getPlayerCount() const;
    virtual QString getRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
    virtual void onTagSet(Room *room, const QString &key) const = 0;
    virtual bool generalSelection() const;

protected:
    QString lord;
    QStringList loyalists, rebels, renegades;
    ScenarioRule *rule;
};

#endif

