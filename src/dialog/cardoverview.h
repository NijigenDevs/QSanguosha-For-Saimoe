

#ifndef _CARD_OVERVIEW_H
#define _CARD_OVERVIEW_H

#include "flatdialog.h"

class Card;
class QTableWidgetItem;

namespace Ui
{
    class CardOverview;
}

class CardOverview : public FlatDialog
{
    Q_OBJECT

public:
    static CardOverview *getInstance(QWidget *main_window);

    CardOverview(QWidget *parent = 0);
    void loadFromAll();
    void loadFromList(const QList<const Card *> &list);

    ~CardOverview();

private:
    Ui::CardOverview *ui;

    void addCard(int i, const Card *card);

    private slots:
    void on_femalePlayButton_clicked();
    void on_malePlayButton_clicked();
    void on_playAudioEffectButton_clicked();
#ifdef Q_OS_IOS
    void comboBoxChanged();
#endif
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);
    void on_tableWidget_itemSelectionChanged();
    void askCard();

protected:
    void showEvent(QShowEvent *);
};

#endif

