#ifndef MPVTYPES_H
#define MPVTYPES_H

#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QList>

namespace Mpv
{
    const QStringList audio_filetypes = {"*.mp3","*.ogg","*.wav","*.wma","*.m4a","*.ac3","*.ape","*.flac","*.ra"},
                      video_filetypes = {"*.avi","*.divx","*.mpg","*.mpeg","*.m1v","*.m2v","*.mpv","*.dv","*.3gp","*.mov","*.mp4","*.m4v","*.mqv","*.dat","*.vcd","*.ogm","*.ogv","*.asf","*.wmv","*.vob","*.mkv","*.ram","*.flv","*.rm","*.ts","*.rmvb","*.dvr-ms","*.m2t","*.m2ts","*.rec","*.f4v","*.hdmov","*.webm","*.vp8"},
                      media_filetypes = audio_filetypes + video_filetypes;

    enum PlayState
    {
        Idle,
        Started,
        Loaded,
        Playing,
        Paused,
        Stopped,
        Ended
    };
    struct Chapter
    {
        QString title;
        int time;
    };
    struct Track
    {
        int id;
        QString type;
        int src_id;
        QString title;
        QString lang;
        unsigned albumart : 1,
                 _default : 1,
                 external : 1;
        QString external_filename;
        QString codec;
    };
    struct VideoParams
    {
        int width,
            height,
            dwidth,
            dheight;
        double aspect;
    };

    struct FileInfo
    {
        QString media_title;
        int length;
        VideoParams video_params;
        QList<Track> tracks; // audio, video, and subs
        QList<Chapter> chapters;
    };
}
Q_DECLARE_METATYPE(Mpv::PlayState) // so we can pass it with signals & slots
Q_DECLARE_METATYPE(Mpv::Chapter)
Q_DECLARE_METATYPE(Mpv::Track)
Q_DECLARE_METATYPE(Mpv::VideoParams)
Q_DECLARE_METATYPE(Mpv::FileInfo)


#endif // MPVTYPES_H
