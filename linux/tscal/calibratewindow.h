/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QtDebug>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QList>
#include <QPoint>
#include <QScreen>
#include <QThread>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>

const int MAX_SAMPLES = 16;


typedef struct {
    int x[5], xfb[5];
    int y[5], yfb[5];
    int a[7];
} calibration;

class EventDeviceThread : public QThread
{
    Q_OBJECT
    const char *devicePath;

public:
    EventDeviceThread(const char *devicePath)
        : devicePath( devicePath )
    {
    }

    void run()
    {
        int fd = open(devicePath, O_RDONLY);
        if (!fd)
        {
            fprintf(stderr,"Error: cannot open event device: %s.\n", devicePath);
            exit(1);
        }
        fprintf(stderr,"Opened: %s.\n", devicePath);

        int x = 0;
        int y = 0;
        bool touch = false;
        for (;;)
        {
            struct input_event evnt;
            int red = read(fd, &evnt, sizeof(evnt) );

            //fprintf(stderr,"Read Event Device\n");
            if ( red == sizeof(evnt) )
            {
                if ( evnt.type == EV_ABS && evnt.code == ABS_X )
                    x = evnt.value;
                else if ( evnt.type == EV_ABS && evnt.code == ABS_Y )
                    y = evnt.value;
                else if ( evnt.type == EV_KEY && evnt.code == BTN_TOUCH )
                    touch = evnt.value == 0 ? false : true;
                else if ( evnt.type == EV_SYN )
                    emit event(x,y,touch);
            }
            else
            {
                fprintf(stderr,"Read %d. Expected %d.\n", red, (int)sizeof(evnt) );
            }
        }

        close(fd);
    }

signals:
    void event(int x,int y,bool touch);
};


class CalibrateWindow: public QMainWindow
{
    Q_OBJECT
    QList<QPoint> screenPoints;
    QList<QPoint> touchPoints;
    QPoint lastPoint;
    int currentPoint;
    int crossSize;
    EventDeviceThread events;

    int x_values[MAX_SAMPLES];
    int y_values[MAX_SAMPLES];
    int count;
    int ptr;
    int lastTouch;
    calibration cal;

    int lastpointX,lastpointY;

public slots:
    void event(int x, int y, bool touch)
    {
        qDebug() << "Event " << x << "," << y << " " << touch;

        // keep a rolling average of x/y and wait till steady for 1 sec.
        x_values[ptr] = x;
        y_values[ptr] = y;
        ptr = (ptr+1) % MAX_SAMPLES;
        if ( count < MAX_SAMPLES )
            count++;

        int xsum = 0, ysum = 0;
        for ( int i = 0; i < count; i++ )
        {
            xsum += x_values[i];
            ysum += y_values[i];
        }

        int xavg = xsum/count;
        int yavg = ysum/count;

        //fprintf(stderr, "%d,%d,%d\n", xavg, yavg, touch);

        if ( touch == 0 && lastTouch == 1)
            NextPoint(xavg,yavg);

        if ( lastTouch != touch )
            ptr = count = 0;

        lastTouch = touch;
    }

public:
    CalibrateWindow() : events("/dev/input/event3")
    {
        ptr = count = 0;
        lastTouch = 0;
        QObject::connect(&events,SIGNAL(event(int,int,bool)),
                         this, SLOT(event(int,int,bool)));
        events.start();
        QScreen *screen = QApplication::screens().at(0);
        setWindowState(Qt::WindowFullScreen);
        setAttribute(Qt::WA_AcceptTouchEvents);

        screenPoints.append( QPoint(0.1*screen->size().width(),0.1*screen->size().height()) );
        screenPoints.append( QPoint(0.9*screen->size().width(),0.1*screen->size().height()) );
        screenPoints.append( QPoint(0.9*screen->size().width(),0.9*screen->size().height()) );
        screenPoints.append( QPoint(0.1*screen->size().width(),0.9*screen->size().height()) );
        screenPoints.append( QPoint(0.5*screen->size().width(),0.5*screen->size().height()) );

        currentPoint = 0;
        crossSize = 0.08*screen->size().height();
        lastPoint = QPoint(width()/2,height()/2);

        for ( int i = 0; i < screenPoints.count(); i++ )
        {
            touchPoints.append(QPoint(0,0));
        }

    }

    ~CalibrateWindow()
    {
        events.quit();
    }

    virtual bool event(QEvent *ev)
    {
        qDebug() << "Event " << ev->type();
        return QMainWindow::event(ev);
    }

    virtual void mouseMoveEvent(QMouseEvent *ev)
    {
        qDebug() << "mouse move " << ev->globalPos().x() << "," << ev->globalPos().y();
    }

    virtual void paintEvent(QPaintEvent *event)
    {
        QPainter painter(this);
        painter.setPen(Qt::blue);
        if ( currentPoint < screenPoints.count())
        {
            QPoint pt = screenPoints[currentPoint];
            painter.drawLine(pt.x() - crossSize, pt.y(), pt.x() + crossSize, pt.y() );
            painter.drawLine(pt.x(), pt.y() - crossSize, pt.x(), pt.y() + crossSize );
        }
        else
        {
            //fprintf(stderr,"ellipse %d,%d\n", lastpointX, lastpointY);
            painter.drawEllipse(lastpointX-4,lastpointY-4,8,8);
        }
    }


    void NextPoint( int x, int y )
    {
        if (currentPoint < touchPoints.count())
        {
            touchPoints[currentPoint].setX(x);
            touchPoints[currentPoint].setY(y);

            currentPoint++;
            update();
        }

        if ( currentPoint == screenPoints.count() )
        {
            currentPoint++;

            QScreen *screen = QApplication::screens().at(0);
            const int tsMax = 0x7fff;

            memset( &cal, 0, sizeof(cal));
            for ( int i = 0; i < screenPoints.count(); i++ )
            {
                cal.xfb[i] = screenPoints[i].x()*tsMax/screen->size().width();
                cal.yfb[i] = screenPoints[i].y()*tsMax/screen->size().height();
                cal.x[i] = touchPoints[i].x();
                cal.y[i] = touchPoints[i].y();
            }

            perform_calibration(&cal);
            fprintf(stderr,"calibration: %d,%d,%d %d,%d,%d %d\n", cal.a[0], cal.a[1], cal.a[2], cal.a[3], cal.a[4], cal.a[5], cal.a[6]);
        }
        else if ( currentPoint > screenPoints.count() )
        {
            int sx = (x * cal.a[1] + y * cal.a[2] + cal.a[0]) / cal.a[6];
            int sy = (x * cal.a[4] + y * cal.a[5] + cal.a[3]) / cal.a[6];

            //fprintf(stderr,"%d,%d => %d,%d\n",x,y,sx,sy);

            QScreen *screen = QApplication::screens().at(0);
            const int tsMax = 0x7fff;
            sx = sx * screen->size().width()/tsMax;
            sy = sy * screen->size().height()/tsMax;

            //fprintf(stderr,"%d,%d => %d,%d\n",x,y,sx,sy);
            lastpointX = sx;
            lastpointY = sy;
            update();
        }

    }

    /*  From https://github.com/kergoth/tslib/blob/master/tests/ts_calibrate_common.c
     *
     *  tslib/tests/ts_calibrate_common.c
     *
     *  Copyright (C) 2001 Russell King.
     *
     * This file is placed under the GPL.  Please see the file
     * COPYING for more details.
     *
     * SPDX-License-Identifier: GPL-2.0+
     *
     *
     * common functions for calibration
     */
    int perform_calibration(calibration *cal)
    {
        int j;
        float n, x, y, x2, y2, xy, z, zx, zy;
        float det, a, b, c, e, f, i;
        float scaling = 65536.0;

        /* Get sums for matrix */
        n = x = y = x2 = y2 = xy = 0;
        for (j = 0; j < 5; j++) {
            n += 1.0;
            x += (float)cal->x[j];
            y += (float)cal->y[j];
            x2 += (float)(cal->x[j]*cal->x[j]);
            y2 += (float)(cal->y[j]*cal->y[j]);
            xy += (float)(cal->x[j]*cal->y[j]);
        }

        /* Get determinant of matrix -- check if determinant is too small */
        det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
        if (det < 0.1 && det > -0.1) {
            fprintf(stderr,"ts_calibrate: determinant is too small -- %f\n", det);
            return 0;
        }

        /* Get elements of inverse matrix */
        a = (x2*y2 - xy*xy)/det;
        b = (xy*y - x*y2)/det;
        c = (x*xy - y*x2)/det;
        e = (n*y2 - y*y)/det;
        f = (x*y - n*xy)/det;
        i = (n*x2 - x*x)/det;

        /* Get sums for x calibration */
        z = zx = zy = 0;
        for (j = 0; j < 5; j++) {
            z += (float)cal->xfb[j];
            zx += (float)(cal->xfb[j]*cal->x[j]);
            zy += (float)(cal->xfb[j]*cal->y[j]);
        }

        /* Now multiply out to get the calibration for framebuffer x coord */
        cal->a[0] = (int)((a*z + b*zx + c*zy)*(scaling));
        cal->a[1] = (int)((b*z + e*zx + f*zy)*(scaling));
        cal->a[2] = (int)((c*z + f*zx + i*zy)*(scaling));

        //fprintf(stderr,"%f %f %f\n", (a*z + b*zx + c*zy),
        //             (b*z + e*zx + f*zy),
        //             (c*z + f*zx + i*zy));

        /* Get sums for y calibration */
        z = zx = zy = 0;
        for (j = 0; j < 5; j++) {
            z += (float)cal->yfb[j];
            zx += (float)(cal->yfb[j]*cal->x[j]);
            zy += (float)(cal->yfb[j]*cal->y[j]);
        }

        /* Now multiply out to get the calibration for framebuffer y coord */
        cal->a[3] = (int)((a*z + b*zx + c*zy)*(scaling));
        cal->a[4] = (int)((b*z + e*zx + f*zy)*(scaling));
        cal->a[5] = (int)((c*z + f*zx + i*zy)*(scaling));

        //fprintf(stderr,"%f %f %f\n", (a*z + b*zx + c*zy),
        //             (b*z + e*zx + f*zy),
        //             (c*z + f*zx + i*zy));

        /* If we got here, we're OK, so assign scaling to a[6] and return */
        cal->a[6] = (int)scaling;

        return 1;
    }

};


// calibration: -98401536,71080,-127 -207798528,-587,79398 65536
