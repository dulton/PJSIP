// PJSIPWrapper.h

#pragma once

#include "stdafx.h"
#include "MyAccount.h"
#include "MyCall.h"
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjnath.h>
#include <pjsip.h>
#include <pjsip_ua.h>
#include <pjsip_simple.h>
#include <pjsua-lib/pjsua_internal.h>
#include <pjsua-lib/pjsua.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjsua2.hpp>
#include <unordered_map>
#include <pjmedia\types.h>
#include <pjmedia\conference.h>

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace pj;

#pragma managed

public ref class IncomingCallEventArgs : EventArgs
{
public:
	int CallID;
	System::String^ DNIS;
	System::String^ ANI;
};
//
//public ref class CallDisconnectedEventArgs : EventArgs
//{
//public:
//	int CallID;
//};

//public delegate void IncomingCallDelegate(OnIncomingCallParam &prm);
//public delegate void IncomingCallEventHandler(IncomingCallEventArgs^ e);
//public delegate void CallDisconnectedDelegate(MyCall *call);
//public delegate void CallDisconnectedEventHandler(CallDisconnectedEventArgs^ e);

static void on_call_media_state(pjsua_call_id call_id);
static void call_media_init(pjsua_call_id call_id);
static void call_media_deinit(pjsua_call_id call_id);
static void on_stream_created(pjsua_call_id call_id, pjmedia_stream *stream, unsigned stream_idx, pjmedia_port **p_port);
static void on_stream_destroyed(pjsua_call_id call_id, pjmedia_stream *stream, unsigned stream_idx);
static void on_call_state(pjsua_call_id call_id, pjsip_event *e);

public ref class PJSIPWrapper
{

private:

	MyAccount *m_Account;
	Endpoint *m_Endpoint;
	//List<System::Int32>^ m_Calls;
	std::unordered_map<int, Call*>* m_Calls;
	//std::vector<Call> m_Calls;

	void OnIncomingCall(OnIncomingCallParam &prm);
	void OnCallDisconnected(MyCall *call);

public:

	//event IncomingCallEventHandler^ IncomingCall;
	//event CallDisconnectedEventHandler^ CallDisconnected;

	PJSIPWrapper(void);	

	void AnswerCall(int callID);

	void HangUp(int callID);

	void PlayPrompt(int callID);

	int Start();

	void Stop();
};

