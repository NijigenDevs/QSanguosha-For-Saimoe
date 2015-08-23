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

#ifndef ABOUTUS_H
#define ABOUTUS_H

#include "flatdialog.h"

class QListWidget;
class QTextBrowser;

class AboutUsDialog : public FlatDialog
{
    Q_OBJECT

public:

    //************************************
    // Method:    AboutUsDialog
    // FullName:  AboutUsDialog::AboutUsDialog
    // Access:    public
    // Returns:
    // Qualifier:
    // Parameter: QWidget * parent
    // Description: Construct a dialog to provide information about developers.
    //              The dialog can also be used to introduce this program.
    //
    // Last Updated By Yanguam Siliagim
    // To make the characters clearer
    //
    // Mogara
    // March 14 2014
    //************************************
    AboutUsDialog(QWidget *parent);

private:
    QListWidget *list;
    QTextBrowser *content_box;

private slots:
    void loadContent(int row);
};

#endif // ABOUTUS_H
