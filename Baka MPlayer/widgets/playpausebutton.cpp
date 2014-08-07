#include "playpausebutton.h"

PlayPauseButton::PlayPauseButton(QWidget *parent):
    QPushButton(parent),
    play(true)
{
}

void PlayPauseButton::SetPlay(bool _play)
{
    play = _play;
    Update();
}

void PlayPauseButton::Update()
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
