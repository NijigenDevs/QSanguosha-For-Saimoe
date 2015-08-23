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

#ifndef _WRAPPED_CARD_H
#define _WRAPPED_CARD_H

#include "card.h"

// This is a wrapper class around a card. Each card id should have one and only one WrappedCard
// copy in each room after game initialization is done. Each room's WrappedCards are isolated,
// but inside the room they are shared and synced between server/client.
//
// WrappedCard's internal card is only intended to provide CardEffect (the card face). The suit,
// number should not be modified to reflect the updated suit/number of WrappedCard. The modified
// suit/number/flags/... are maintained in WrappedCard's own member variables.
//
// All WrappedCard's member function that takes a Card as parameter will take over the Card passed
// in, meaning that the caller is resposible for allocating the memory, but WrappedCard is responsible
// for destroying it. No caller should ever delete any card that has been passed in to any member function
// of WrappedCard that takes Card * as parameter (unless the parameter is (const Card *)).
//
// WrappedCard should never have any subcard!!! It's a concrete, single piece card in the room no matter when.

class WrappedCard : public Card
{
    Q_OBJECT

public:
    Q_INVOKABLE WrappedCard(Card *card);
    ~WrappedCard();

    inline virtual void setId(int id)
    {
        m_id = id;
        Q_ASSERT(m_card != NULL);
        m_card->setId(id);
    }

    inline virtual void setNumber(int number)
    {
        m_number = number;
        Q_ASSERT(m_card != NULL);
        m_card->setNumber(number);
    }

    inline virtual void setSuit(Suit suit)
    {
        m_suit = suit;
        Q_ASSERT(m_card != NULL);
        m_card->setSuit(suit);
    }

    inline virtual void setSkillName(const QString &skillName)
    {
        m_skillName = skillName;
        Q_ASSERT(m_card != NULL);
        m_card->setSkillName(skillName);
    }

    // Set the internal card to be the new card, update everything related
    // to CardEffect including objectName.
    void takeOver(Card *card);
    void copyEverythingFrom(Card *card);
    void setModified(bool modified)
    {
        m_isModified = modified;
    }

    // Inherited member functions
    inline virtual void onNullified(ServerPlayer *target) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->onNullified(target);

    }
    inline virtual bool isModified() const
    {
        return m_isModified;
    }
    inline virtual QString getClassName() const
    {
        Q_ASSERT(m_card != NULL);
        Q_ASSERT(m_card->metaObject() != NULL);
        return m_card->getClassName();
    }

    inline virtual const Card *getRealCard() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card;
    }

    inline virtual bool isMute() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->isMute();
    }

    inline virtual bool willThrow() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->willThrow();
    }

    inline virtual bool canRecast() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->canRecast();
    }

    inline virtual Card::HandlingMethod getHandlingMethod() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->getHandlingMethod();
    }

    inline virtual bool hasPreAction() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->hasPreAction();
    }

    inline virtual QString getPackage() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->getPackage();
    }

    inline virtual bool isVirtualCard() const
    {
        return false;
    }
    //inline virtual bool isEquipped() const{ return m_card->isEquipped(); }
    inline virtual QString getCommonEffectName() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->getCommonEffectName();
    }

    inline virtual bool match(const QString &pattern) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->match(pattern);
    }

    virtual void setFlags(const QString &flag) const;
    inline virtual void addSubcard(int /*card_id*/)
    {
        Q_ASSERT(false);
    }
    inline virtual void addSubcard(const Card * /*card*/)
    {
        Q_ASSERT(false);
    }
    inline virtual void addSubcards(const QList<const Card *> & /*cards*/)
    {
        Q_ASSERT(false);
    }
    inline virtual void addSubcards(const QList<int> & /*subcards_list*/)
    {
        Q_ASSERT(false);
    }
    // inline virtual QList<int> getSubcards() const;
    // inline virtual void clearSubcards();
    // inline virtual QString subcardString() const;
    // inline virtual int subcardsLength() const;

    inline virtual QString getType() const
    {
        return m_card->getType();
    }
    inline virtual QString getSubtype() const
    {
        return m_card->getSubtype();
    }
    inline virtual CardType getTypeId() const
    {
        return m_card->getTypeId();
    }
    inline virtual QString toString(bool hidden = false) const
    {
        Q_UNUSED(hidden)
            return QString::number(m_id);
    }
    inline virtual bool isNDTrick() const
    {
        return m_card->isNDTrick();
    }

    // card target selection
    inline virtual bool targetFixed() const
    {
        return m_card->targetFixed();
    }
    inline virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
    {
        return m_card->targetsFeasible(targets, Self);
    }

    // @todo: the following two functions should be merged into one.
    inline virtual bool targetFilter(const QList<const Player *> &targets,
        const Player *to_select, const Player *Self) const
    {
        return m_card->targetFilter(targets, to_select, Self);
    }

    inline virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select,
        const Player *Self, int &maxVotes) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->targetFilter(targets, to_select, Self, maxVotes);
    }

    inline virtual bool isAvailable(const Player *player) const
    {
        return m_card->isAvailable(player);
    }

    inline virtual const Card *validate(CardUseStruct &cardUse) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->validate(cardUse);
    }

    inline virtual const Card *validateInResponse(ServerPlayer *user) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->validateInResponse(user);
    }

    inline virtual void doPreAction(Room *room, const CardUseStruct &cardUse) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->doPreAction(room, cardUse);
    }

    inline virtual void onUse(Room *room, const CardUseStruct &cardUse) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->onUse(room, cardUse);
    }

    inline virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->use(room, source, targets);
    }

    inline virtual void onEffect(const CardEffectStruct &effect) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->onEffect(effect);
    }

    inline virtual bool isCancelable(const CardEffectStruct &effect) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->isCancelable(effect);
    }

    inline virtual bool isKindOf(const char *cardType) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->isKindOf(cardType);
    }

    inline virtual bool isTransferable() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->isTransferable();
    }

    inline virtual void setTransferable(const bool transferbale)
    {
        Q_ASSERT(m_card != NULL);
        m_card->setTransferable(transferbale);
    }

    inline virtual QStringList checkTargetModSkillShow(const CardUseStruct &use) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->checkTargetModSkillShow(use);
    }

protected:
    Card *m_card;
    mutable bool m_isModified;
};

#endif

