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

#ifndef _UI_UTILS_H
#define _UI_UTILS_H

#include <QImage>
#include <QColor>
#include <QRect>
#include <QPainter>

namespace QSanUiUtils {
    // This is in no way a generic diation fuction. It is some dirty trick that
    // produces a shadow image for a pixmap whose foreground mask is binaryImage
    void paintShadow(QPainter *painter, const QImage &image, QColor shadowColor, int radius, double decade, const QRect boundingBox);
    void paintShadow(QPainter *painter, const QImage &image, QColor shadowColor, int radius, double decade, const int x, const int y);
    void makeGray(QPixmap &pixmap);

    namespace QSanFreeTypeFont {
        bool init();
        void quit();

        typedef int *QSanFont;


        QSanFont loadFont(const QString &fontPath);
        QString resolveFont(const QString &fontName);
        // @param painter
        //        Device to be painted on
        // @param font
        //        Pointer returned by loadFont used to index a font
        // @param text
        //        Text to be painted
        // @param fontSize [IN, OUT]
        //        Suggested width and height of each character in pixels. If the
        //        bounding box cannot contain the text using the suggested font
        //        size, font size may be shrinked. The output value will be the
        //        actual font size used.
        // @param boundingBox
        //        Text will be painted in the center of the bounding box on the device
        // @param orient
        //        Suggest whether the text is laid out horizontally or vertically.
        // @return True if succeed.
        bool paintQString(QPainter *painter, QString text,
            QSanFont font, QColor color,
            QSize &fontSize, int spacing, int weight, QRect boundingBox,
            Qt::Orientation orient, Qt::Alignment align);

        // Currently, we online support horizotal layout for multiline text
        bool paintQStringMultiLine(QPainter *painter, QString text,
            QSanFont font, QColor color,
            QSize &fontSize, int spacing, QRect boundingBox,
            Qt::Alignment align);
    }
}

#endif
