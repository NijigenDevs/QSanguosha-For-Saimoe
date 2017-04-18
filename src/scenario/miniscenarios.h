

#ifndef _MINI_SCENARIOS_H
#define _MINI_SCENARIOS_H

#include "scenario.h"
#include "engine.h"
#include "room.h"

class MiniSceneRule : public ScenarioRule
{
    Q_OBJECT

public:
    static const char *S_EXTRA_OPTION_RANDOM_ROLES;
    static const char *S_EXTRA_OPTION_REST_IN_DISCARD_PILE;

    MiniSceneRule(Scenario *scenario);
    void assign(QStringList &generals, QStringList &generals2, QStringList &kingdoms, Room *room) const;
    virtual int getPlayerCount() const;
    QStringList existedGenerals() const;

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who = NULL) const;

    void addNPC(QString feature);
    void setOptions(QStringList option);
    void setPile(QString cardList);
    void loadSetting(QString path);

protected:
    QList< QMap<QString, QString> > players;
    QString setup;
    QMap<QString, QVariant> ex_options;
    static const QString _S_DEFAULT_HERO;
    QList<int> m_fixedDrawCards;
};

class MiniScene : public Scenario
{
    Q_OBJECT

public:
    static const char *S_KEY_MINISCENE;
    MiniScene(const QString &name);
    ~MiniScene();
    void setupCustom(QString name) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual void assign(QStringList &generals, QStringList &generals2, QStringList &roles, Room *room) const
    {
        MiniSceneRule *rule = qobject_cast<MiniSceneRule *>(getRule());
        rule->assign(generals, generals2, roles, room);
    }
    virtual int getPlayerCount() const
    {
        MiniSceneRule *rule = qobject_cast<MiniSceneRule *>(getRule());
        return rule->getPlayerCount();
    }
};

class CustomScenario : public MiniScene
{
    Q_OBJECT

public:
    CustomScenario() : MiniScene("custom_scenario")
    {
        setupCustom(NULL);
    }
};

class LoadedScenario : public MiniScene
{
    Q_OBJECT

public:
    LoadedScenario(const QString &name) : MiniScene(QString(MiniScene::S_KEY_MINISCENE).arg(name))
    {
        setupCustom(name);
    }
};

#endif

