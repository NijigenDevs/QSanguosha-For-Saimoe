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

#ifndef _TIMED_PROGRESS_BAR_H
#define _TIMED_PROGRESS_BAR_H

#include <QProgressBar>
#include <QTimerEvent>
#include <QShowEvent>
#include <QPaintEvent>
#include <QMutex>

class TimedProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    inline TimedProgressBar()
        : m_hasTimer(false), m_autoHide(false), m_timer(0),
        m_step(0), m_max(0), m_val(0), m_mutex(QMutex::Recursive)
    {
        this->setTextVisible(false);
        this->setWindowFlags(Qt::Tool);
    }
    inline void setTimerEnabled(bool enabled)
    {
        m_mutex.lock();
        m_hasTimer = enabled;
        m_mutex.unlock();
    }
    inline void setCountdown(time_t maximum, time_t startVal = 0)
    {
        m_mutex.lock();
        m_max = maximum;
        m_val = startVal;
        m_mutex.unlock();
    }
    inline void setAutoHide(bool enabled)
    {
        m_autoHide = enabled;
    }
    inline void setUpdateInterval(time_t step)
    {
        m_step = step;
    }
    virtual void show();
    virtual void hide();

signals:
    void timedOut();

protected:
    virtual void timerEvent(QTimerEvent *);
    bool m_hasTimer;
    bool m_autoHide;
    int m_timer;
    time_t m_step, m_max, m_val;
    QMutex m_mutex;
};

#include "protocol.h"
#include "settings.h"

class QSanCommandProgressBar : public TimedProgressBar
{
    Q_OBJECT

public:
    QSanCommandProgressBar();
    inline void setInstanceType(QSanProtocol::ProcessInstanceType type)
    {
        m_instanceType = type;
    }
    void setCountdown(QSanProtocol::CommandType command);
    void setCountdown(QSanProtocol::Countdown countdown);

protected:
    virtual void paintEvent(QPaintEvent *);
    QSanProtocol::ProcessInstanceType m_instanceType;
};

#endif

