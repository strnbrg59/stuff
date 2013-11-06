// From http://doc.trolltech.com/4.2/widgets-analogclock.html

/****************************************************************************
**
** Copyright (C) 2004-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//#include <QtGui>
#include <iostream>
#include "analogclock.h"
#include "budget.h"
using std::cout;

extern MasterTimer g_masterTimer;

AnalogClock::AnalogClock(QWidget *parent)
    : QWidget(parent)
{
    // Determines how often paintEvent() is called.
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000); // milliseconds

    setWindowTitle(tr("Analog Clock"));
    resize(200, 200);
}

void AnalogClock::paintEvent(QPaintEvent *)
{
    static const QPoint hourHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -40)
    };
    static const QPoint minuteHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -70)
    };
    static const QPoint secondHand[3] = {
        QPoint(1, 1),
        QPoint(-1, 1),
        QPoint(0, -90)
    };

    QColor hourColor(255, 0, 0);
    QColor minuteColor(0, 0, 255, 191);
    QColor secondColor(0, 0, 0, 255);

    int side = qMin(width(), height());

    g_masterTimer.update();
    QTime budget(g_masterTimer.budgetAsQtime());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    //
    // Hours
    //
    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColor);
    painter.save();
    // Draw hour hand
    painter.rotate(30.0 * ((budget.hour() + budget.minute() / 60.0)));
    painter.drawConvexPolygon(hourHand, 3);
    painter.restore();
    // Draw hour tickmarks.
    painter.setPen(hourColor);
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(88, 0, 96, 0);
        painter.rotate(30.0);
    }

    //
    // Minutes
    //
    painter.setPen(Qt::NoPen);
    painter.setBrush(minuteColor);
    painter.save();
    // Draw minute hand
    painter.rotate(6.0 * (budget.minute() + budget.second() / 60.0));
    painter.drawConvexPolygon(minuteHand, 3);
    painter.restore();
    // Draw minute tickmarks
    painter.setPen(minuteColor);
    for (int j = 0; j < 60; ++j) {
        if ((j % 5) != 0)
            painter.drawLine(92, 0, 96, 0);
        painter.rotate(6.0);
    }

    //
    // Seconds
    //
    painter.setPen(Qt::NoPen);
    painter.setBrush(secondColor);
    painter.save();
    // Draw seconds hand.
    painter.rotate(6.0 * budget.second());
    painter.drawConvexPolygon(secondHand, 3);
    painter.restore();
}


int operator-(QTime const& t1, QTime const& t2)
{
    int result = 3600 *(t1.hour() - t2.hour())
               +   60 *(t1.minute() - t2.minute())
               +        t1.second() - t2.second();
    return result;
}


MasterTimer::MasterTimer()
  : m_prev(QTime::currentTime()),
    m_budget(budget::getBudget()),
    m_budgetFileTime(budget::budgetFileTime())
{
}


QTime
MasterTimer::budgetAsQtime() const
{
    int h = m_budget/3600;
    int m = (m_budget - h*3600)/60;
    int s = m_budget - h*3600 - m*60;
    return QTime(h, m, s);
}


bool
afterHours()
{
    time_t t = time(0);
    tm* now = localtime(&t);
    int minutes = now->tm_hour*60 + now->tm_min;
    // No using X between 10PM and 6:00AM.
    return (minutes > 22*60) || (minutes < 6*60);
}

void
MasterTimer::update()
{
    QTime now(QTime::currentTime());
    int elapsed = (now - m_prev);
    m_prev = now;
    if (m_budgetFileTime < budget::budgetFileTime()) {
        m_budgetFileTime = budget::budgetFileTime();
        m_budget = budget::getBudget();
    } else {
        m_budget -= elapsed;
    }

    if ((m_budget <= 0) || afterHours()) {
//      budget::killInternet();
//      budget::killX();
        if (m_budget % 5 == 0) {
            budget::killFirefox();
        }
    }        
}
