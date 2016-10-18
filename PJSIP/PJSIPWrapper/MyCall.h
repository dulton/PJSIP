#pragma once

#include <pjsua2.hpp>
#include <pjmedia.h>
#include <pjsua-lib/pjsua_internal.h>

using namespace System;
using namespace pj;

struct call_data
{
  pj_pool_t           *pool;
  pjmedia_conf        *conf;
  pjmedia_port        *cport;
  pjmedia_port        *null;
  pjmedia_master_port *m;
  int                  call_slot;
};

typedef void (__stdcall *CALLDISCONNECTEDCB)(Call* call);

void onDtmfReceived(pjmedia_stream * stream, void *user_data, int digit);

class MyCall : public Call
{
private:
	pjmedia_stream* m_Stream;
	vector<char> m_DigitBuffer;
	CALLDISCONNECTEDCB m_CallDisconnectedCallback;

public:
    MyCall(Account &acc, int call_id = PJSUA_INVALID_ID) : Call(acc, call_id)
    {
		//m_DigitBuffer = vector<char>();
	}

	~MyCall()
    {
	}

	// Notification when call's state has changed.
    //virtual void onCallState(OnCallStateParam &prm);

	void setCallDisconnectedCallback(CALLDISCONNECTEDCB cb);

    // Notification when call's media state has changed.
    virtual void onCallMediaState(OnCallMediaStateParam &prm);

	virtual void onCallSdpCreated(OnCallSdpCreatedParam &prm);

	//virtual void onDtmfDigit(OnDtmfDigitParam &prm);

	virtual void onStreamCreated(OnStreamCreatedParam &prm);	

	void playPrompt();

	void appendDigit(char digitChar);

	void answer();
};