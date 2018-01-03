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

#include "calibratewindow.h"



#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME "event"

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define BIT(nr)                 (1UL << (nr))
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE           8
#define BITS_PER_LONG           (sizeof(long) * BITS_PER_BYTE)
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

static int is_event_device(const struct dirent *dir)
{
    return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
}


static char *scan_devices(void)
{
    struct dirent **namelist;
    int i, ndev;
    char *filename = NULL;
    int have_touchscreen = 0;
    long propbit[BITS_TO_LONGS(INPUT_PROP_MAX)] = {0};

#ifdef DEBUG
    printf("scanning for devices in %s\n", DEV_INPUT_EVENT);
#endif

    ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, alphasort);
    if (ndev <= 0)
        return NULL;

    for (i = 0; i < ndev; i++) {
        char fname[64];
        int fd = -1;

        snprintf(fname, sizeof(fname),
             "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);
        fd = open(fname, O_RDONLY);
        if (fd < 0)
            continue;

        if ((ioctl(fd, EVIOCGPROP(sizeof(propbit)), propbit) < 0) ||
            !(propbit[BIT_WORD(INPUT_PROP_DIRECT)] &
                  BIT_MASK(INPUT_PROP_DIRECT))) {
            continue;
        } else {
            have_touchscreen = 1;
        }

        close(fd);
        free(namelist[i]);

        if (have_touchscreen) {
            filename = (char *)malloc(strlen(DEV_INPUT_EVENT) +
                      strlen(EVENT_DEV_NAME) +
                      3);
            if (!filename)
                return NULL;

            sprintf(filename, "%s/%s%d",
                DEV_INPUT_EVENT, EVENT_DEV_NAME,
                i);
        }

        return filename;
    }

    return NULL;
}



int main(int argc, char **argv)
{
    //scan_devices();
    QApplication app(argc, argv);

    CalibrateWindow window;
    window.showMaximized();

    return app.exec();
}
