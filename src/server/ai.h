/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef _AI_H
#define _AI_H

class Room;
class ServerPlayer;
class TrickCard;
class ResponseSkill;

struct lua_State;

typedef int LuaFunction;

#include "card.h"
#include "structs.h"

#include <QString>
#include <QObject>

class AI : public QObject {
    Q_OBJECT
    Q_ENUMS(Relation)

public:
    AI(ServerPlayer *player);

    enum Relation { Friend, Enemy, Neutrality };
    static Relation GetRelationHegemony(const ServerPlayer *a, const ServerPlayer *b);
    Relation relationTo(const ServerPlayer *other) const;
    bool isFriend(const ServerPlayer *other) const;
    bool isEnemy(const ServerPlayer *other) const;

    QList<ServerPlayer *> getEnemies() const;
    QList<ServerPlayer *> getFriends() const;

    virtual void activate(CardUseStruct &card_use) = 0;
    virtual Card::Suit askForSuit(const QString &reason) = 0;
    virtual QString askForKingdom() = 0;
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data) = 0;
    virtual QString askForChoice(const QString &skill_name, const QString &choices, const QVariant &data) = 0;
    virtual QList<int> askForDiscard(const QString &reason, int discard_num, int min_num, bool optional, bool include_equip) = 0;
    virtual const Card *askForNullification(const Card *trick, ServerPlayer *from, ServerPlayer *to, bool positive) = 0;
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason, Card::HandlingMethod method) = 0;
    virtual const Card *askForCard(const QString &pattern, const QString &prompt, const QVariant &data) = 0;
    virtual QString askForUseCard(const QString &pattern, const QString &prompt, const Card::HandlingMethod method) = 0;
    virtual int askForAG(const QList<int> &card_ids, bool refusable, const QString &reason) = 0;
    virtual const Card *askForCardShow(ServerPlayer *requestor, const QString &reason) = 0;
    virtual const Card *askForPindian(ServerPlayer *requestor, const QString &reason) = 0;
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason) = 0;
    virtual const Card *askForSinglePeach(ServerPlayer *dying) = 0;
    virtual ServerPlayer *askForYiji(const QList<int> &cards, const QString &reason, int &card_id) = 0;
    virtual void askForGuanxing(const QList<int> &cards, QList<int> &up, QList<int> &bottom, int guanxing_type) = 0;
    virtual void filterEvent(TriggerEvent triggerEvent, ServerPlayer *player, const QVariant &data);

protected:
    Room *room;
    ServerPlayer *self;
};

class TrustAI : public AI {
    Q_OBJECT

public:
    TrustAI(ServerPlayer *player);
    ~TrustAI();

    virtual void activate(CardUseStruct &card_use);
    virtual Card::Suit askForSuit(const QString &);
    virtual QString askForKingdom();
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data);
    virtual QString askForChoice(const QString &skill_name, const QString &choices, const QVariant &data);
    virtual QList<int> askForDiscard(const QString &reason, int discard_num, int min_num, bool optional, bool include_equip);
    virtual const Card *askForNullification(const Card *trick, ServerPlayer *from, ServerPlayer *to, bool positive);
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason, Card::HandlingMethod method);
    virtual const Card *askForCard(const QString &pattern, const QString &prompt, const QVariant &data);
    virtual QString askForUseCard(const QString &pattern, const QString &prompt, const Card::HandlingMethod method);
    virtual int askForAG(const QList<int> &card_ids, bool refusable, const QString &reason);
    virtual const Card *askForCardShow(ServerPlayer *requestor, const QString &reason);
    virtual const Card *askForPindian(ServerPlayer *requestor, const QString &reason);
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason);
    virtual const Card *askForSinglePeach(ServerPlayer *dying);
    virtual ServerPlayer *askForYiji(const QList<int> &cards, const QString &reason, int &card_id);
    virtual void askForGuanxing(const QList<int> &cards, QList<int> &up, QList<int> &bottom, int guanxing_type);

    virtual bool useCard(const Card *card);

private:
    ResponseSkill *response_skill;
};

class LuaAI : public TrustAI {
    Q_OBJECT

public:
    LuaAI(ServerPlayer *player);

    virtual const Card *askForCardShow(ServerPlayer *requestor, const QString &reason);
    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data);
    virtual void activate(CardUseStruct &card_use);
    virtual QString askForUseCard(const QString &pattern, const QString &prompt, const Card::HandlingMethod method);
    virtual QList<int> askForDiscard(const QString &reason, int discard_num, int min_num, bool optional, bool include_equip);
    virtual const Card *askForNullification(const Card *trick, ServerPlayer *from, ServerPlayer *to, bool positive);
    virtual QString askForChoice(const QString &skill_name, const QString &choices, const QVariant &data);
    virtual int askForCardChosen(ServerPlayer *who, const QString &flags, const QString &reason, Card::HandlingMethod method);
    virtual const Card *askForCard(const QString &pattern, const QString &prompt, const QVariant &data);
    virtual ServerPlayer *askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason);
    virtual int askForAG(const QList<int> &card_ids, bool refusable, const QString &reason);
    virtual const Card *askForSinglePeach(ServerPlayer *dying);
    virtual const Card *askForPindian(ServerPlayer *requestor, const QString &reason);
    virtual Card::Suit askForSuit(const QString &reason);

    virtual ServerPlayer *askForYiji(const QList<int> &cards, const QString &reason, int &card_id);
    virtual void askForGuanxing(const QList<int> &cards, QList<int> &up, QList<int> &bottom, int guanxing_type);

    virtual void filterEvent(TriggerEvent triggerEvent, ServerPlayer *player, const QVariant &data);

    LuaFunction callback;

private:
    void pushCallback(lua_State *L, const char *function_name);
    void pushQIntList(lua_State *L, const QList<int> &list);
    void reportError(lua_State *L);
    bool getTable(lua_State *L, QList<int> &table);
};

#endif

