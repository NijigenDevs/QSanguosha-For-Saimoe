#ifndef _SCENARIO_OVERVIEW_H
#define _SCENARIO_OVERVIEW_H

#include "flatdialog.h"

class QListWidget;
class QTextEdit;

class RuleSummary : public FlatDialog
{
    Q_OBJECT

public:
    RuleSummary(QWidget *parent);

private:
    QListWidget *list;
    QTextEdit *content_box;

    private slots:
    void loadContent(int row);
};

#endif

