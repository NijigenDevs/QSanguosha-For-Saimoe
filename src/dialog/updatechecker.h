#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QWidget>

class QLabel;
class QTextEdit;

struct UpdateInfoStruct;

class UpdateChecker : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateChecker();

    void fill(UpdateInfoStruct info);
    void clear();

private:
    QLabel *state_label;
    QLabel *address_label;
    QTextEdit *page;
};

#endif // UPDATECHECKER_H
