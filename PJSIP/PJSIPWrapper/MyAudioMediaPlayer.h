#pragma once

#include <pjsua2.hpp>
#include <windows.h>

using namespace pj;

class MyAudioMediaPlayer : public AudioMediaPlayer
{
private:
	HANDLE m_PlaybackComplete;
public:

	void transmit(const AudioMedia &sink);

    virtual bool onEof();
};