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

#ifndef ANALOGCLOCK_H
#define ANALOGCLOCK_H

#include <QtGui/QWidget>
#include <QtCore/QTime>
#include <Qt/qtimer.h>
#include <Qt/qpainter.h>
#include <Qt/qapplication.h>

class AnalogClock : public QWidget
{
//    Comment this Q_OBJECT thing out, or you get "no vtable" linker error.
//    Q_OBJECT
public:
    AnalogClock(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
};


class MasterTimer
{
private:
    QTime m_prev;
    int   m_budget;
    time_t m_budgetFileTime;
public:
    MasterTimer();
    void update();
    int budget() const { return m_budget; }
    QTime budgetAsQtime() const;
};


int operator-(QTime const& t1, QTime const& t2);

#endif
