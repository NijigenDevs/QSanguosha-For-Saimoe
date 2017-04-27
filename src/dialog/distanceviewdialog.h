#ifndef DISTANCEVIEWDIALOG_H
#define DISTANCEVIEWDIALOG_H

#include "flatdialog.h"

class DistanceViewDialogUI;

class DistanceViewDialog : public FlatDialog
{
    Q_OBJECT

public:
    DistanceViewDialog(QWidget *parent = 0);
    ~DistanceViewDialog();

private:
    DistanceViewDialogUI *ui;

private slots:
    void showDistance();
};

#endif // DISTANCEVIEWDIALOG_H

