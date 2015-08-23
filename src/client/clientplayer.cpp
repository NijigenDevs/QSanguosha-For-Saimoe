/********************************************************************
    Copyright (c) 2013-2015 - Mogara

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

    Mogara
    *********************************************************************/

#include "clientplayer.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "standard.h"
#include "settings.h"

#include <QTextDocument>
#include <QTextOption>

ClientPlayer *Self = NULL;

ClientPlayer::ClientPlayer(Client *client)
    : Player(client), handcard_num(0)
{
    mark_doc = new QTextDocument(this);
}

int ClientPlayer::aliveCount(bool includeRemoved) const
{
    int n = ClientInstance->alivePlayerCount();
    if (!includeRemoved) {
        if (isRemoved())
            n--;
        foreach(const Player *p, getAliveSiblings())
            if (p->isRemoved())
                n--;
    }
    return n;
}

int ClientPlayer::getHandcardNum() const
{
    return handcard_num;
}

void ClientPlayer::addCard(const Card *card, Place place)
{
    switch (place) {
        case PlaceHand: {
            if (card) known_cards << card;
            handcard_num++;
            break;
        }
        case PlaceEquip: {
            WrappedCard *equip = Sanguosha->getWrappedCard(card->getEffectiveId());
            setEquip(equip);
            break;
        }
        case PlaceDelayedTrick: {
            addDelayedTrick(card);
            break;
        }
        default:
            break;
    }
}

void ClientPlayer::addKnownHandCard(const Card *card)
{
    if (!known_cards.contains(card))
        known_cards << card;
}

bool ClientPlayer::isLastHandCard(const Card *card, bool contain) const
{
    if (!card->isVirtualCard()) {
        if (known_cards.length() != 1)
            return false;
        return known_cards.first()->getId() == card->getEffectiveId();
    } else if (card->getSubcards().length() > 0) {
        if (!contain) {
            foreach (int card_id, card->getSubcards()) {
                if (!known_cards.contains(Sanguosha->getCard(card_id)))
                    return false;
            }
            return known_cards.length() == card->getSubcards().length();
        } else {
            foreach (const Card *ncard, known_cards) {
                if (!card->getSubcards().contains(ncard->getEffectiveId()))
                    return false;
            }
            return true;
        }
    }
    return false;
}

void ClientPlayer::removeCard(const Card *card, Place place)
{
    switch (place) {
        case PlaceHand: {
            handcard_num--;
            if (card) {
                known_cards.removeOne(card);
                visible_cards.removeOne(card);
            }
            break;
        }
        case PlaceEquip:{
            WrappedCard *equip = Sanguosha->getWrappedCard(card->getEffectiveId());
            removeEquip(equip);
            break;
        }
        case PlaceDelayedTrick:{
            removeDelayedTrick(card);
            break;
        }
        default:
            break;
    }
}

QList<const Card *> ClientPlayer::getHandcards() const
{
    return known_cards;
}

void ClientPlayer::setCards(const QList<int> &card_ids)
{
    known_cards.clear();
    foreach(int cardId, card_ids)
        known_cards.append(Sanguosha->getCard(cardId));
}

QList<const Card *> ClientPlayer::getVisiblecards() const
{
    return visible_cards;
}

void ClientPlayer::addVisibleCards(const QList<int> &card_ids)
{
    foreach(int cardId, card_ids)
        visible_cards.append(Sanguosha->getCard(cardId));
}

void ClientPlayer::removeVisibleCards(const QList<int> &card_ids)
{
    foreach(int cardId, card_ids)
        visible_cards.removeOne(Sanguosha->getCard(cardId));
}

QTextDocument *ClientPlayer::getMarkDoc() const
{
    return mark_doc;
}

static bool compareByNumber(int c1, int c2)
{
    const Card *card1 = Sanguosha->getCard(c1);
    const Card *card2 = Sanguosha->getCard(c2);

    return card1->getNumber() < card2->getNumber();
}

void ClientPlayer::changePile(const QString &name, bool add, QList<int> card_ids)
{
    if (add) {
        piles[name].append(card_ids);
        if (name == "buqu") {
            qSort(piles["buqu"].begin(), piles["buqu"].end(), compareByNumber);
        }
    } else {
        foreach (int card_id, card_ids) {
            if (piles[name].isEmpty()) break;
            if (piles[name].contains(Card::S_UNKNOWN_CARD_ID) && !piles[name].contains(card_id))
                piles[name].removeOne(Card::S_UNKNOWN_CARD_ID);
            else if (piles[name].contains(card_id))
                piles[name].removeOne(card_id);
            else
                piles[name].takeLast();
        }
    }
    if (!name.startsWith("#") && !name.startsWith("^"))
        emit pile_changed(name);
}

QString ClientPlayer::getDeathPixmapPath() const
{
    QString basename = getRole() == "careerist" ? "careerist" : getKingdom();

    if (basename.isEmpty())
        basename = "unknown";

    return QString("image/system/death/%1.png").arg(basename);
}

void ClientPlayer::setHandcardNum(int n)
{
    handcard_num = n;
}

QString ClientPlayer::getGameMode() const
{
    return ServerInfo.GameMode;
}

void ClientPlayer::setFlags(const QString &flag)
{
    Player::setFlags(flag);

    if (flag.endsWith("actioned"))
        emit action_taken();

    //emit skill_state_changed(flag);
}

void ClientPlayer::setMark(const QString &mark, int value)
{
    if (marks[mark] == value)
        return;
    marks[mark] = value;

    if (mark == "drank")
        emit drank_changed();

    if (!mark.startsWith("@"))
        return;

    // @todo: consider move all the codes below to PlayerCardContainerUI.cpp
    // set mark doc
    QString text = "";
    QMapIterator<QString, int> itor(marks);
    while (itor.hasNext()) {
        itor.next();
        if (itor.key().startsWith("@") && itor.value() > 0) {
            QString mark_text = QString("<img src='image/mark/%1.png' />").arg(itor.key());
            if (itor.value() != 1)
                mark_text.append(QString("%1").arg(itor.value()));
            if (this != Self)
                mark_text.append("<br>");
            text.append(mark_text);
        }
    }
    mark_doc->setHtml(text);

    if (mark == "@duanchang")
        emit duanchang_invoked();
}

QStringList ClientPlayer::getBigKingdoms(const QString &, MaxCardsType::MaxCardsCount type) const
{
    QMap<QString, int> kingdom_map;
    kingdom_map.insert("wei", 0);
    kingdom_map.insert("shu", 0);
    kingdom_map.insert("wu", 0);
    kingdom_map.insert("qun", 0);
    QList<const Player *> players = getAliveSiblings();
    players.prepend(this);
    foreach (const Player *p, players) {
        if (!p->hasShownOneGeneral())
            continue;
        if (p->getRole() == "careerist") {
            kingdom_map["careerist"] = 1;
            continue;
        }
        ++kingdom_map[p->getKingdom()];
    }
    if (type == MaxCardsType::Max && hasLordSkill("hongfa") && !getPile("heavenly_army").isEmpty())
        kingdom_map["qun"] += getPile("heavenly_army").length();
    QStringList big_kingdoms;
    foreach (const QString &key, kingdom_map.keys()) {
        if (kingdom_map[key] == 0)
            continue;
        if (big_kingdoms.isEmpty()) {
            if (kingdom_map[key] > 1)
                big_kingdoms << key;
            continue;
        }
        if (kingdom_map[key] == kingdom_map[big_kingdoms.first()]) {
            big_kingdoms << key;
        } else if (kingdom_map[key] > kingdom_map[big_kingdoms.first()]) {
            big_kingdoms.clear();
            big_kingdoms << key;
        }
    }
    const Player *jade_seal_owner = NULL;
    foreach (const Player *p, players) {
        if (p->hasTreasure("JadeSeal") && p->hasShownOneGeneral()) {
            jade_seal_owner = p;
            break;
        }
    }
    if (jade_seal_owner != NULL) {
        if (jade_seal_owner->getRole() == "careerist") {
            big_kingdoms.clear();
            big_kingdoms << jade_seal_owner->objectName(); // record player's objectName who has JadeSeal.
        } else { // has shown one general but isn't careerist
            QString kingdom = jade_seal_owner->getKingdom();
            big_kingdoms.clear();
            big_kingdoms << kingdom;
        }
    }
    return big_kingdoms;
}

void ClientPlayer::setHeadSkinId(int id)
{
    if (headSkinId == id || (this != Self && Config.IgnoreOthersSwitchesOfSkin))
        return;

    if (id <= general->skinCount()) {
        headSkinId = id;
        emit headSkinIdChanged(general->objectName());
    }

    if (this == Self && !Self->hasFlag("marshalling"))
        ClientInstance->onPlayerChangeSkin(id);
}

void ClientPlayer::setDeputySkinId(int id)
{
    if (deputySkinId == id || (this != Self && Config.IgnoreOthersSwitchesOfSkin))
        return;

    if (id <= general2->skinCount()) {
        deputySkinId = id;
        emit deputySkinIdChanged(general2->objectName());
    }

    if (this == Self && !Self->hasFlag("marshalling"))
        ClientInstance->onPlayerChangeSkin(id, false);
}
