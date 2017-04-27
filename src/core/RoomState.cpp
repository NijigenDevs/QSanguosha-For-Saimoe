#include "roomstate.h"
#include "engine.h"
#include "wrappedcard.h"

RoomState::~RoomState()
{
    foreach (WrappedCard *card, m_cards) {
        delete card;
    }
    m_cards.clear();
}

Card *RoomState::getCard(int cardId) const
{
    if (!m_cards.contains(cardId))
        return NULL;
    return m_cards[cardId];
}

void RoomState::resetCard(int cardId)
{
    Card *newCard = Card::Clone(Sanguosha->getEngineCard(cardId));
    if (newCard == NULL) return;
    newCard->setFlags(m_cards[cardId]->getFlags());
    m_cards[cardId]->copyEverythingFrom(newCard);
    newCard->clearFlags();
    m_cards[cardId]->setModified(false);
}

// Reset all cards, generals' states of the room instance
void RoomState::reset()
{
    foreach (WrappedCard *card, m_cards) {
        delete card;
    }
    m_cards.clear();

    int n = Sanguosha->getCardCount();
    for (int i = 0; i < n; i++) {
        Card *newCard = Card::Clone(Sanguosha->getEngineCard(i));
        m_cards[i] = new WrappedCard(newCard);
    }
}
