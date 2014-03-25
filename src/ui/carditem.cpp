/********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Hegemony Team	
*********************************************************************/
#include "carditem.h"
#include "engine.h"
#include "skill.h"
#include "clientplayer.h"
#include "settings.h"

#include <cmath>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFocusEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

void CardItem::_initialize() {
    setFlag(QGraphicsItem::ItemIsMovable);
    m_opacityAtHome = 1.0;
    m_currentAnimation = NULL;
    _m_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    _m_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    _m_showFootnote = true;
    m_isSelected = false;
    _m_isUnknownGeneral = false;
    auto_back = true;
    frozen = false;
    resetTransform();
    setTransform(QTransform::fromTranslate(-_m_width / 2, -_m_height / 2), true);
}

CardItem::CardItem(const Card *card) {
    _initialize();
    setCard(card);
    setAcceptHoverEvents(true);
}

CardItem::CardItem(const QString &general_name) {
    m_cardId = Card::S_UNKNOWN_CARD_ID;
    _initialize();
    changeGeneral(general_name);
    m_currentAnimation = NULL;
    m_opacityAtHome = 1.0;
}

QRectF CardItem::boundingRect() const{
    return G_COMMON_LAYOUT.m_cardFrameArea;
}

void CardItem::setCard(const Card *card) {
    if (card != NULL) {
        m_cardId = card->getId();
        const Card *engineCard = Sanguosha->getEngineCard(m_cardId);
        Q_ASSERT(engineCard != NULL);
        setObjectName(engineCard->objectName());
        setToolTip(engineCard->getDescription());
    } else {
        m_cardId = Card::S_UNKNOWN_CARD_ID;
        setObjectName("unknown");
    }
}

void CardItem::setEnabled(bool enabled) {
     QSanSelectableItem::setEnabled(enabled);
}

CardItem::~CardItem() {
    m_animationMutex.lock();
    if (m_currentAnimation != NULL) {
        delete m_currentAnimation;
        m_currentAnimation = NULL;
    }
    m_animationMutex.unlock();
}

void CardItem::changeGeneral(const QString &general_name) {
    setObjectName(general_name);
    const General *general = Sanguosha->getGeneral(general_name);
    if (general) {
        _m_isUnknownGeneral = false;
        setToolTip(general->getSkillDescription(true));
    } else {
        _m_isUnknownGeneral = true;
        setToolTip(QString());
    }
    emit general_changed();
}

const Card *CardItem::getCard() const{
    return Sanguosha->getCard(m_cardId);
}

void CardItem::setHomePos(QPointF home_pos) {
    this->home_pos = home_pos;
}

QPointF CardItem::homePos() const{
    return home_pos;
}

void CardItem::goBack(bool playAnimation, bool doFade) {
    if (playAnimation) {
        getGoBackAnimation(doFade);
        if (m_currentAnimation != NULL)
            m_currentAnimation->start();
    } else {
        m_animationMutex.lock();
        if (m_currentAnimation != NULL) {
            m_currentAnimation->stop();
            delete m_currentAnimation;
            m_currentAnimation = NULL;
        }
        setPos(homePos());
        m_animationMutex.unlock();
    }
}

QAbstractAnimation *CardItem::getGoBackAnimation(bool doFade, bool smoothTransition, int duration) {
    m_animationMutex.lock();
    if (m_currentAnimation != NULL) {
        m_currentAnimation->stop();
        delete m_currentAnimation;
        m_currentAnimation = NULL;
    }
    QPropertyAnimation *goback = new QPropertyAnimation(this, "pos");
    goback->setEndValue(home_pos);
    goback->setEasingCurve(QEasingCurve::OutQuad);
    goback->setDuration(duration);

    if (doFade) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        QPropertyAnimation *disappear = new QPropertyAnimation(this, "opacity");
        double middleOpacity = qMax(opacity(), m_opacityAtHome);
        if (middleOpacity == 0) middleOpacity = 1.0;
        disappear->setEndValue(m_opacityAtHome);
        if (!smoothTransition) {
            disappear->setKeyValueAt(0.2, middleOpacity);
            disappear->setKeyValueAt(0.8, middleOpacity);
            disappear->setDuration(duration);
        }

        group->addAnimation(goback);
        group->addAnimation(disappear);

        m_currentAnimation = group;
    } else {
        m_currentAnimation = goback;
    }
    m_animationMutex.unlock();
    connect(m_currentAnimation, SIGNAL(finished()), this, SIGNAL(movement_animation_finished()));
    return m_currentAnimation;
}

void CardItem::showFrame(const QString &result) {
    _m_frameType = result;
}

void CardItem::hideFrame() {
    _m_frameType = QString();
}

void CardItem::showAvatar(const General *general) {
    _m_avatarName = general->objectName();
}

void CardItem::hideAvatar() {
    _m_avatarName = QString();
}

void CardItem::setAutoBack(bool auto_back) {
    this->auto_back = auto_back;
}

bool CardItem::isEquipped() const{
    const Card *card = getCard();
    Q_ASSERT(card);
    return Self->hasEquip(card);
}

void CardItem::setFrozen(bool is_frozen) {
    frozen = is_frozen;
}

CardItem *CardItem::FindItem(const QList<CardItem *> &items, int card_id) {
    foreach (CardItem *item, items) {
        if (item->getCard() == NULL) {
            if (card_id == Card::S_UNKNOWN_CARD_ID)
                return item;
            else
                continue;
        }
        if (item->getCard()->getId() == card_id)
            return item;
    }

    return NULL;
}

const int CardItem::_S_CLICK_JITTER_TOLERANCE = 1600;
const int CardItem::_S_MOVE_JITTER_TOLERANCE = 200;

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (frozen) return;
    _m_lastMousePressScenePos = mapToParent(mouseEvent->pos());
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (frozen) return;

    QPointF totalMove = mapToParent(mouseEvent->pos()) - _m_lastMousePressScenePos;
    if (totalMove.x() * totalMove.x() + totalMove.y() * totalMove.y() < _S_MOVE_JITTER_TOLERANCE)
        emit clicked();
    else
        emit released();

    if (auto_back) {
        goBack(true, false);
    }
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (!(flags() & QGraphicsItem::ItemIsMovable)) return;
    QPointF newPos = mapToParent(mouseEvent->pos());
    QPointF totalMove = newPos - _m_lastMousePressScenePos;
    if (totalMove.x() * totalMove.x() + totalMove.y() * totalMove.y() >= _S_CLICK_JITTER_TOLERANCE) {
        QPointF down_pos = mouseEvent->buttonDownPos(Qt::LeftButton);
        setPos(newPos - this->transform().map(down_pos));
    }
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (frozen) return;

    if (hasFocus()) {
        event->accept();
        emit double_clicked();
    } else
        emit toggle_discards();
}

void CardItem::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
    emit enter_hover();
}

void CardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
    emit leave_hover();
}


void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!_m_frameType.isEmpty())
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardFrameArea, G_ROOM_SKIN.getCardAvatarPixmap(_m_frameType));

    if (!isEnabled()) {
        painter->fillRect(G_COMMON_LAYOUT.m_cardMainArea, QColor(100, 100, 100, 255 * opacity()));
        painter->setOpacity(0.7 * opacity());
    }

    if (!_m_isUnknownGeneral)
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardMainArea, G_ROOM_SKIN.getCardMainPixmap(objectName()));
    else
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardMainArea, G_ROOM_SKIN.getPixmap("generalCardBack"));
    const Card *card = Sanguosha->getEngineCard(m_cardId);
    if (card) {
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardSuitArea, G_ROOM_SKIN.getCardSuitPixmap(card->getSuit()));
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardNumberArea, G_ROOM_SKIN.getCardNumberPixmap(card->getNumber(), card->isBlack()));
        QRect rect = G_COMMON_LAYOUT.m_cardFootnoteArea;
        // Deal with stupid QT...
        if (_m_showFootnote) painter->drawImage(rect, _m_footnoteImage);
    }

    if (!_m_avatarName.isEmpty())
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardAvatarArea, G_ROOM_SKIN.getCardAvatarPixmap(_m_avatarName));
}

void CardItem::setFootnote(const QString &desc) {
    const IQSanComponentSkin::QSanShadowTextFont &font = G_COMMON_LAYOUT.m_cardFootnoteFont;
    QRect rect = G_COMMON_LAYOUT.m_cardFootnoteArea;
    rect.moveTopLeft(QPoint(0, 0));
    _m_footnoteImage = QImage(rect.size(), QImage::Format_ARGB32);
    _m_footnoteImage.fill(Qt::transparent);
    QPainter painter(&_m_footnoteImage);
    font.paintText(&painter, QRect(QPoint(0, 0), rect.size()),
                   (Qt::AlignmentFlag)((int)Qt::AlignHCenter | Qt::AlignBottom | Qt::TextWrapAnywhere), desc);
}

