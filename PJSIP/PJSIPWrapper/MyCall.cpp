#include "stdafx.h"
#include "MyCall.h"
#include "MyAudioMediaPlayer.h"
#include <pjsua2.hpp>
#include <iostream>
#include <pjmedia.h>;

using namespace pj;

void MyCall::onCallSdpCreated(OnCallSdpCreatedParam &prm)
{
	CallInfo ci = getInfo();
}


void MyCall::onCallMediaState(OnCallMediaStateParam &prm)
{
	CallInfo ci = getInfo();

	
}

void MyCall::onStreamCreated(OnStreamCreatedParam &prm)
{
	pjsua_call_info info;

	CallInfo ci = getInfo();

	/*
	pjsua_call *call;
    pjsip_dialog *dlg;
    unsigned mi;

	pjsua_var.med_endpt

    call = &pjsua_var.calls[ci.id];

	call->

	return;
	*/

	pj_pool_t *pool;
	struct call_data *cd;
	pj_status_t status;

	pool = pjsua_pool_create("mycall", 1000, 1000);
	cd = PJ_POOL_ZALLOC_T(pool, struct call_data);
	cd->pool = pool;

	pjsua_call_set_user_data(ci.id, (void*)cd);

	status = pjmedia_conf_create(pool, 2, 16, 1, 2, 16, PJMEDIA_CONF_NO_DEVICE, &cd->conf);
	cd->cport = pjmedia_conf_get_master_port(cd->conf);
	status = pjmedia_null_port_create(pool, 16, 1, 2, 16, &cd->null);
	status = pjmedia_master_port_create(pool, cd->null, cd->cport, 0, &cd->m);
	status = pjmedia_master_port_start(cd->m);

	//struct call_data* cd;

	//cd = (struct call_data*) pjsua_call_get_user_data(ci.id);

	unsigned int slot = cd->call_slot;

	pjmedia_port* mediaPort = (pjmedia_port*)prm.pPort;

	pjmedia_conf_add_port(cd->conf, cd->pool, mediaPort, NULL, &slot);

	std::cout << "*** media port added to the conference bridge ***" << std::endl;

	pjmedia_stream* stream = (pjmedia_stream*)prm.stream;

	pj_status_t result = pjmedia_stream_set_dtmf_callback(stream, &onDtmfReceived, this);

	if (result == 0)
	{
		std::cout << "*** dtmf callback set ***" << std::endl;
	}
}

//void MyCall::onCallState(OnCallStateParam &prm)
//{
//    CallInfo ci = getInfo();
//    if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
//        m_CallDisconnectedCallback(this);
//    }
//}

void MyCall::setCallDisconnectedCallback(CALLDISCONNECTEDCB cb)
{
	m_CallDisconnectedCallback = cb;
}

void MyCall::answer()
{
	CallOpParam callOp;
    callOp.statusCode = PJSIP_SC_OK;
    Call::answer(callOp);
}

void MyCall::playPrompt()
{
	m_DigitBuffer.clear();

	CallInfo ci = getInfo();
	AudioMedia* aud_med = NULL;

	// Find out which media index is the audio
	for (unsigned i=0; i<ci.media.size(); ++i) {
		if (ci.media[i].type == PJMEDIA_TYPE_AUDIO) {
			aud_med = (AudioMedia *)getMedia(i);
			break;
		}
	}

	MyAudioMediaPlayer player;

	try
	{
		player.createPlayer("C:\\Users\\brad\\Desktop\\Demo_Survey_2.wav", PJMEDIA_FILE_NO_LOOP);
		player.transmit(*aud_med);
	}
	catch (Error& err)
	{
	}
}

void MyCall::appendDigit(char digitChar)
{
	m_DigitBuffer.push_back(digitChar);
}

void onDtmfReceived(pjmedia_stream * stream, void *user_data, int digit)
{
	std::cout << "*** digit press ***" << std::endl;
	MyCall* call = (MyCall*)user_data;
	char digitChar = (char)digit;
	call->appendDigit(digitChar);
}