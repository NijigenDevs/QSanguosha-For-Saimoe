#ifndef _ENGINE_H
#define _ENGINE_H

#include "RoomState.h"
#include "card.h"
#include "general.h"
#include "skill.h"
#include "package.h"
#include "exppattern.h"
#include "protocol.h"
#include "util.h"

#include <QHash>
#include <QStringList>
#include <QMetaObject>
#include <QThread>
#include <QList>
#include <QMutex>

class AI;
class Scenario;
class LuaBasicCard;
class LuaTrickCard;
class LuaWeapon;
class LuaArmor;

struct lua_State;

class Engine: public QObject {
    Q_OBJECT

public:
    Engine();
    ~Engine();

    void addTranslationEntry(const char *key, const char *value);
    QString translate(const QString &to_translate) const;
    lua_State *getLuaState() const;

    int getMiniSceneCounts();

    void addPackage(Package *package);
    void addBanPackage(const QString &package_name);
    QStringList getBanPackages() const;
    Card *cloneCard(const Card *card) const;
    Card *cloneCard(const QString &name, Card::Suit suit = Card::SuitToBeDecided, int number = -1, const QStringList &flags = QStringList()) const;
    SkillCard *cloneSkillCard(const QString &name) const;
    QString getVersionNumber() const;
    QString getVersion() const;
    QString getVersionName() const;
    QString getMODName() const;
    QStringList getExtensions() const;
    QStringList getKingdoms() const;
    QColor getKingdomColor(const QString &kingdom) const;
    QMap<QString, QColor> getSkillColorMap() const;
    QColor getSkillColor(const QString &skill_type) const;
    QStringList getChattingEasyTexts() const;
    QString getSetupString() const;

    QMap<QString, QString> getAvailableModes() const;
    QString getModeName(const QString &mode) const;
    int getPlayerCount(const QString &mode) const;
    QString getRoles(const QString &mode) const;
    QStringList getRoleList(const QString &mode) const;
    int getRoleIndex() const;

    const CardPattern *getPattern(const QString &name) const;
    bool matchExpPattern(const QString &pattern, const Player *player, const Card *card) const;
    Card::HandlingMethod getCardHandlingMethod(const QString &method_name) const;
    QList<const Skill *> getRelatedSkills(const QString &skill_name) const;
    const Skill *getMainSkill(const QString &skill_name) const;

    QStringList getModScenarioNames() const;
    void addScenario(Scenario *scenario);
    const Scenario *getScenario(const QString &name) const;
    void addPackage(const QString &name);

    const General *getGeneral(const QString &name) const;
    int getGeneralCount(bool include_banned = false) const;
    const Skill *getSkill(const QString &skill_name) const;
    const Skill *getSkill(const EquipCard *card) const;
    QStringList getSkillNames() const;
    const TriggerSkill *getTriggerSkill(const QString &skill_name) const;
    const ViewAsSkill *getViewAsSkill(const QString &skill_name) const;
    QList<const DistanceSkill *> getDistanceSkills() const;
    QList<const MaxCardsSkill *> getMaxCardsSkills() const;
    QList<const TargetModSkill *> getTargetModSkills() const;
    QList<const TriggerSkill *> getGlobalTriggerSkills() const;
    void addSkills(const QList<const Skill *> &skills);

    int getCardCount() const;
    const Card *getEngineCard(int cardId) const;
    // @todo: consider making this const Card *
    Card *getCard(int cardId);
    WrappedCard *getWrappedCard(int cardId);

    QStringList getLords(bool contain_banned = false) const;
    QStringList getRandomLords() const;
    void banRandomGods() const;
    QStringList getRandomGenerals(int count, const QSet<QString> &ban_set = QSet<QString>()) const;
    QList<int> getRandomCards() const;
    QString getRandomGeneralName() const;
    QStringList getLimitedGeneralNames() const;

    void playSystemAudioEffect(const QString &name) const;
    void playAudioEffect(const QString &filename) const;
    void playSkillAudioEffect(const QString &skill_name, int index) const;

    const ProhibitSkill *isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;
    int correctDistance(const Player *from, const Player *to) const;
    int correctMaxCards(const Player *target, bool fixed = false) const;
    int correctCardTarget(const TargetModSkill::ModType type, const Player *from, const Card *card) const;

    void registerRoom(QObject *room);
    void unregisterRoom();
    QObject *currentRoomObject();
    Room *currentRoom();
    RoomState *currentRoomState();

    QString getCurrentCardUsePattern();
    CardUseStruct::CardUseReason getCurrentCardUseReason();

    bool isGeneralHidden(const QString &general_name) const;

private:
    void _loadMiniScenarios();
    void _loadModScenarios();

    QMutex m_mutex;
    QHash<QString, QString> translations;
    QHash<QString, const General *> generals;
    QHash<QString, const QMetaObject *> metaobjects;
    QHash<QString, QString> className2objectName;
    QHash<QString, const Skill *> skills;
    QHash<QThread *, QObject *> m_rooms;
    QMap<QString, QString> modes;
    QMultiMap<QString, QString> related_skills;
    mutable QMap<QString, const CardPattern *> patterns;

    // special skills
    QList<const ProhibitSkill *> prohibit_skills;
    QList<const DistanceSkill *> distance_skills;
    QList<const MaxCardsSkill *> maxcards_skills;
    QList<const TargetModSkill *> targetmod_skills;
    QList<const TriggerSkill *> global_trigger_skills;

    QList<Card *> cards;
    QStringList lord_list;
    QSet<QString> ban_package;
    QHash<QString, Scenario *> m_scenarios;
    QHash<QString, Scenario *> m_miniScenes;
    Scenario *m_customScene;

    lua_State *lua;

    QHash<QString, QString> luaBasicCard_className2objectName;
    QHash<QString, const LuaBasicCard *> luaBasicCards;
    QHash<QString, QString> luaTrickCard_className2objectName;
    QHash<QString, const LuaTrickCard *> luaTrickCards;
    QHash<QString, QString> luaWeapon_className2objectName;
    QHash<QString, const LuaWeapon*> luaWeapons;
    QHash<QString, QString> luaArmor_className2objectName;
    QHash<QString, const LuaArmor *> luaArmors;

    QMultiMap<QString, QString> sp_convert_pairs;
};

static inline QVariant GetConfigFromLuaState(lua_State *L, const char *key) {
    return GetValueFromLuaState(L, "config", key);
}

extern Engine *Sanguosha;

#endif

