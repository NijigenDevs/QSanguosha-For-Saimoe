/********************************************************************
Copyright (c) 2013-2015 - Mogara

This file is part of QSanguosha-Hegemony.

This game is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3.0
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

See the LICENSE file for more details.

Mogara
*********************************************************************/

#include "audio.h"
#include "settings.h"
#include <QString>
#include <QCache>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QDir>
QMediaPlayer *Audio::BGMPlayer = nullptr;
QCache<QString, QMediaPlayer> *Audio::SoundCache = nullptr;

void Audio::init()
{
    SoundCache = new QCache<QString, QMediaPlayer>(30);
}

void Audio::quit()
{
    if (BGMPlayer != nullptr)
        delete BGMPlayer;

    if (SoundCache != nullptr)
    {
        SoundCache->clear();
        SoundCache = nullptr; // Do not use delete here. Because server-side will exec play system audio function too, then it will crash in Audio::play if soundcache is deleted
    }
}

void Audio::play(const QString &filename, const bool doubleVolume)
{
    if (SoundCache == nullptr)
        return;
    QMediaPlayer *sound = nullptr;
    if (!SoundCache->contains(filename))
    {
        sound = new QMediaPlayer;
        sound->setMedia(QUrl(filename));
        SoundCache->insert(filename, sound);
    }
    else
    {
        sound = SoundCache->object(filename);
        if (sound->state() == QMediaPlayer::PlayingState)
        {
            return;
        }
    }

    if (sound == nullptr)
        return;

    sound->setVolume((doubleVolume ? 2 : 1) * Config.EffectVolume * 100);
    sound->play();
}

void Audio::playAudioOfMoxuan()
{
    play("audio/system/moxuan.mp3", true);
}

void Audio::stop()
{
    stopBGM();

    if (SoundCache == nullptr || SoundCache->isEmpty())
        return;

    SoundCache->clear();
}

void Audio::playBGM(const QString &filename)
{
    if (!BGMPlayer)
        BGMPlayer = new QMediaPlayer;
    QMediaPlaylist *BGMList = new QMediaPlaylist(BGMPlayer);
    BGMList->addMedia(QUrl(filename));
    QDir BGMDir("audio/system/bgm");
    QStringList mp3Filters("*.mp3");
    QFileInfoList mp3List = BGMDir.entryInfoList(mp3Filters, QDir::Files);
    foreach (QFileInfo file, mp3List)
    {
        BGMList->addMedia(QUrl(file.filePath()));
    }
    BGMList->setPlaybackMode(QMediaPlaylist::Random);

    BGMPlayer->setPlaylist(BGMList);
    BGMPlayer->setVolume(Config.BGMVolume * 100);
    BGMPlayer->play();
}

void Audio::setBGMVolume(int volume)
{
    if (BGMPlayer)
        BGMPlayer->setVolume(volume);
}

void Audio::stopBGM()
{
    if (BGMPlayer)
        BGMPlayer->stop();
}
