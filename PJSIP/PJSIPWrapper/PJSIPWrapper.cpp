// This is the main DLL file.

#include "stdafx.h"
#include "PJSIPWrapper.h"
#include "MyAccount.h"
#include "MyCall.h"
#include <pjsua2.hpp>
#include <iostream>
#include <pjmedia\sound.h>

using namespace pj;
using namespace System;
using namespace System::Net;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

PJSIPWrapper::PJSIPWrapper(void)
{
	//m_Calls = gcnew List<System::Int32>();
	m_Calls = new std::unordered_map<int, Call*>();
}

void PJSIPWrapper::OnIncomingCall(OnIncomingCallParam &prm)
{
	std::cout << "*** incoming call ***" << std::endl;

	MyCall* call = new MyCall(*PJSIPWrapper::m_Account, prm.callId);

	pjsip_rx_data* rx = (pjsip_rx_data*)prm.rdata.pjRxData;

	// get the call ID
	int callID = call->getId();
	CallInfo callInfo = call->getInfo();	

	const pj_str_t reason = pj_str("Ringing");

	pjsua_call_answer(callInfo.id, 180, &reason, NULL);

	//call_media_init(callInfo.id);

	//CallDisconnectedDelegate^ callDiscDel = gcnew CallDisconnectedDelegate(this, &PJSIPWrapper::OnCallDisconnected);
	//GCHandle gch = GCHandle::Alloc(callDiscDel);
	//IntPtr ip = Marshal::GetFunctionPointerForDelegate(callDiscDel);
	//CALLDISCONNECTEDCB cb = static_cast<CALLDISCONNECTEDCB>(ip.ToPointer());

	//call->setCallDisconnectedCallback(cb);

	// create event args
	IncomingCallEventArgs^ e = gcnew IncomingCallEventArgs();

	// set the call id property
	e->CallID = callID;
	e->DNIS = gcnew System::String(callInfo.localUri.c_str());
	e->ANI = gcnew System::String(callInfo.remoteUri.c_str());

	// raise the incoming call event
	//IncomingCall(e);
}

void PJSIPWrapper::OnCallDisconnected(MyCall *call)
{
	std::cout << "*** call disconnected ***" << std::endl;

	// get the call ID
	int callID = call->getId();

	// create event args
	//CallDisconnectedEventArgs^ e = gcnew CallDisconnectedEventArgs();

	// set the call id property
	//e->CallID = callID;

	// raise the call disconnected event
	//CallDisconnected(e);
}

void PJSIPWrapper::AnswerCall(int callID)
{
	Call* call = Call::lookup(callID);



	CallInfo info = call->getInfo();
	
	const pj_str_t reason = pj_str("OK");

	pjsua_call_answer(info.id, 200, &reason, NULL);

	//call_media_init(info.id);

	/*CallOpParam callOp;
    callOp.statusCode = PJSIP_SC_OK;
    call->answer(callOp);*/
}

void PJSIPWrapper::PlayPrompt(int callID)
{
	MyCall* call = (MyCall*)Call::lookup(callID);

	call->playPrompt();
}

void PJSIPWrapper::HangUp(int callID)
{
	MyCall* call = (MyCall*)Call::lookup(callID);

	CallOpParam callOp;
	callOp.reason = "Normal Clearing";
	call->hangup(callOp);
}

int PJSIPWrapper::Start()
{
	m_Endpoint = new Endpoint;
	m_Endpoint->libCreate();

	// Initialize endpoint
	EpConfig ep_cfg;

	ep_cfg.uaConfig.userAgent = "Voice4Net";

	m_Endpoint->libInit( ep_cfg );

	// Create SIP transport. Error handling sample is shown
	TransportConfig tcfg;
	tcfg.port = 5060;
	try
	{
		m_Endpoint->transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
	}
	catch (Error &err)
	{
		std::cout << err.info() << std::endl;
		return 1;
	}

	// Start the library (worker threads etc)
	m_Endpoint->libStart();
	std::cout << "*** PJSUA2 STARTED ***" << std::endl;

	// Configure an AccountConfig
	AccountConfig acfg;
	acfg.idUri = "sip:1@192.168.3.87:5060";

	//pjsua_set_no_snd_dev();

	pjsua_config cfg;
	
	//cfg.cb.on_call_media_state = &on_call_media_state;
	//cfg.cb.on_stream_created = &on_stream_created;
	cfg.cb.on_stream_destroyed = &on_stream_destroyed;
	cfg.cb.on_call_state = &on_call_state;

	//IncomingCallDelegate^ newCallDel = gcnew IncomingCallDelegate(this, &PJSIPWrapper::OnIncomingCall);
	//GCHandle gch = GCHandle::Alloc(newCallDel);
	//IntPtr ip = Marshal::GetFunctionPointerForDelegate(newCallDel);
	//INCOMINGCALLCB cb = static_cast<INCOMINGCALLCB>(ip.ToPointer());

	// Create the account
	PJSIPWrapper::m_Account = new MyAccount;
	PJSIPWrapper::m_Account->create(acfg, true);
	//PJSIPWrapper::m_Account->setIncomingCallCallback(cb);




	return 0;
}

void PJSIPWrapper::Stop()
{
	if (PJSIPWrapper::m_Account)
	{
		// Delete the account. This will unregister from server
		delete PJSIPWrapper::m_Account;
	}
}

static void on_call_media_state(pjsua_call_id call_id)
{
	call_media_init(call_id);
}

static void call_media_init(pjsua_call_id call_id)
{
   pj_pool_t *pool;
   struct call_data *cd;
   pj_status_t status;

   pool = pjsua_pool_create("mycall", 1000, 1000);
   cd = PJ_POOL_ZALLOC_T(pool, struct call_data);
   cd->pool = pool;

   pjsua_call_set_user_data(call_id, (void*)cd);

   status = pjmedia_conf_create(pool, 2, 16, 1, 2, 16, PJMEDIA_CONF_NO_DEVICE, &cd->conf);
   cd->cport = pjmedia_conf_get_master_port(cd->conf);
   status = pjmedia_null_port_create(pool, 16, 1, 2, 16, &cd->null);
   status = pjmedia_master_port_create(pool, cd->null, cd->cport, 0, &cd->m);
   status = pjmedia_master_port_start(cd->m);
}

static void on_stream_created(pjsua_call_id call_id, pjmedia_stream *stream, unsigned stream_idx, pjmedia_port **p_port)
{
	struct call_data* cd;

	cd = (struct call_data*) pjsua_call_get_user_data(call_id);

	unsigned int slot = cd->call_slot;

	pjmedia_conf_add_port(cd->conf, cd->pool, *p_port, NULL, &slot);
}

static void on_stream_destroyed(pjsua_call_id call_id, pjmedia_stream *stream, unsigned stream_idx)
{
	struct call_data* cd;

	cd = (struct call_data*) pjsua_call_get_user_data(call_id);

	pjmedia_conf_remove_port(cd->conf, cd->call_slot);
}

static void call_media_deinit(pjsua_call_id call_id)
{
	struct call_data *cd;

	cd = (struct call_data*) pjsua_call_get_user_data(call_id);
	if (!cd)
		return;

	pjmedia_master_port_stop(cd->m);
	pjmedia_master_port_destroy(cd->m, PJ_FALSE);

	pjmedia_conf_destroy(cd->conf);
	pjmedia_port_destroy(cd->null);

	//...

	pjsua_call_set_user_data(call_id, NULL);
}

static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
   pjsua_call_info call_info;

   pjsua_call_get_info(call_id, &call_info);

   if (call_info.state == PJSIP_INV_STATE_DISCONNECTED) {
      call_media_deinit(call_id);
   }
}