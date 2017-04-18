

#ifndef FLATDIALOG_H
#define FLATDIALOG_H

#include <QDialog>

class QLabel;
class QVBoxLayout;

class FlatDialog : public QDialog
{
    Q_OBJECT

public:
    FlatDialog(QWidget *parent = 0, bool initialLayoutWithTitle = true);

    inline QVBoxLayout *mainLayout() const
    {
        return layout;
    }

    bool addCloseButton(QString name = QString());

protected:

#ifdef Q_OS_WIN
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
#endif

    QVBoxLayout *layout;

#ifdef Q_OS_WIN
private:
    QLabel *title;
    QPoint mousePressedPoint;
    bool mousePressed;
#endif
};

#endif // FLATDIALOG_H
