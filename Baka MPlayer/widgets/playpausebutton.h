#ifndef PLAYPAUSEBUTTON_H
#define PLAYPAUSEBUTTON_H

#include <QPushButton>

class PlayPauseButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PlayPauseButton(QWidget *parent = 0);

public slots:
    void SetPlay(bool play);
    void Update();

protected:
    bool play;
};

#endif // PLAYPAUSEBUTTON_H
