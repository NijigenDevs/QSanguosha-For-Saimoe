

#ifndef _ROOM_THREAD_H
#define _ROOM_THREAD_H

#include "structs.h"

#include <QThread>
#include <QSemaphore>
#include <QVariant>

class GameRule;

class EventTriplet
{
public:
    inline EventTriplet(TriggerEvent triggerEvent, Room *room, ServerPlayer *target)
        : _m_event(triggerEvent), _m_room(room), _m_target(target)
    {
    }
    QString toString() const;

private:
    TriggerEvent _m_event;
    Room *_m_room;
    ServerPlayer *_m_target;
};

class RoomThread : public QThread
{
    Q_OBJECT

public:
    explicit RoomThread(Room *room);
    void constructTriggerTable();
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *target, QVariant &data);
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *target);

    void addPlayerSkills(ServerPlayer *player, bool invoke_game_start = false);

    void addTriggerSkill(const TriggerSkill *skill);
    void delay(long msecs = -1);
    void actionNormal(GameRule *game_rule);

    const QList<EventTriplet> *getEventStack() const;

protected:
    virtual void run();

private:
    void _handleTurnBrokenNormal(GameRule *game_rule);

    Room *room;
    QString order;

    QList<const TriggerSkill *> skill_table[NumOfEvents];
    QSet<QString> skillSet;

    QList<EventTriplet> event_stack;
    GameRule *game_rule;
};

#endif

