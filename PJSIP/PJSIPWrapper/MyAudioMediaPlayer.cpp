#include "stdafx.h"
#include "MyAudioMediaPlayer.h"
#include <pjsua2.hpp>

using namespace pj;

bool MyAudioMediaPlayer::onEof()
{
	SetEvent(m_PlaybackComplete);
	return false; // returning false stops looping playback
}

void MyAudioMediaPlayer::transmit(const AudioMedia &aud_med)
{
	m_PlaybackComplete = CreateEvent(NULL, TRUE, FALSE, TEXT("PlaybackComplete"));

	startTransmit(aud_med);

	WaitForSingleObject(m_PlaybackComplete, INFINITE);

	CloseHandle(m_PlaybackComplete);
}