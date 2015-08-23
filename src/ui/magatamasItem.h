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

#ifndef _MAGATAMAS_ITEM_H
#define _MAGATAMAS_ITEM_H

#include <QGraphicsObject>
#include <QPixmap>
#include "skinbank.h"

class MagatamasBoxItem : public QGraphicsObject
{
    Q_OBJECT

public:
    MagatamasBoxItem();
    MagatamasBoxItem(QGraphicsItem *parent);
    inline int getHp() const
    {
        return m_hp;
    }
    void setHp(int hp);
    inline int getMaxHp() const
    {
        return m_maxHp;
    }
    void setMaxHp(int maxHp);
    void setOrientation(Qt::Orientation orientation);
    inline Qt::Orientation getOrientation() const
    {
        return m_orientation;
    }
    inline void setBackgroundVisible(bool visible)
    {
        m_showBackground = visible;
    }
    inline bool isBackgroundVisible() const
    {
        return m_showBackground;
    }
    void setAnchor(QPoint anchor, Qt::Alignment align);
    inline void setAnchorEnable(bool enabled)
    {
        anchorEnabled = enabled;
    }
    inline bool isAnchorEnable()
    {
        return anchorEnabled;
    }
    void setIconSize(QSize size);
    inline void setImageArea(QRect rect)
    {
        m_imageArea = rect;
    }
    inline QSize getIconSize() const
    {
        return m_iconSize;
    }
    virtual QRectF boundingRect() const;
    virtual void update();
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

protected:
    void _autoAdjustPos();
    void _updateLayout();
    void _doHpChangeAnimation(int newHp);
    QPoint m_anchor;
    Qt::Alignment m_align;
    bool anchorEnabled;
    int m_hp;
    int m_maxHp;
    Qt::Orientation m_orientation;
    bool m_showBackground;
    QSize m_iconSize;
    QRect m_imageArea;
    QPixmap _icons[4];
};
#endif

