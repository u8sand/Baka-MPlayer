#ifndef PLAYPAUSEBUTTON_H
#define PLAYPAUSEBUTTON_H

#include <QPushButton>

class PlayPauseButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PlayPauseButton(QWidget *parent = 0):
        QPushButton(parent),
        play(true)
    {
    }

    void SetPlay(bool _play)
    {
        play = _play;
        Update();
    }

    void Update()
    {
        if(play)
        {
            if(this->isEnabled())
                setIcon(QIcon(":/img/default_play.svg"));
            else
                setIcon(QIcon(":/img/disabled_play.svg"));
        }
        else
        {
            if(this->isEnabled())
                setIcon(QIcon(":/img/default_pause.svg"));
            else
                setIcon(QIcon(":/img/disabled_pause.svg"));
        }
    }

protected:
    bool play;
};

#endif // PLAYPAUSEBUTTON_H
