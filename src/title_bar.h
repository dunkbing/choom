//
// Created by Bùi Đặng Bình on 18/5/25.
//

#ifndef TITLE_BAR_H
#define TITLE_BAR_H

#include <QWidget>

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);

    signals:
        void minimizeClicked();
    void maximizeClicked();
    void closeClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *closeAction;
};

#endif //TITLE_BAR_H
