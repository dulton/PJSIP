// PjSipServer.h

#pragma once

#include "stdafx.h"
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjnath.h>
#include <pjsip.h>
#include <pjsip_ua.h>
#include <pjsip_simple.h>
#include <pjsua-lib/pjsua.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <vector>
#include <pjsip-ua\sip_regc.h>
#include <pjsip_ua.h>

#define SIP_PORT 5060 /* Listening SIP port */
#define RTP_PORT 4000 /* RTP port */
#define MAX_MEDIA_CNT 1 /* Media count, set to 1 for audio or 2 for audio and video */
#define MAX_CALLS	8
#define THIS_FILE "PjSipServer.cpp"

using namespace System;

int call_id=-1;
unsigned int slot[10];
struct sip_channel_t
{	
	int callID;
    pjsip_inv_session *inv;
	pjmedia_transport *transport;
	pjmedia_stream *stream;
	pj_pool_t *conf_pool;
	pjmedia_port *cport;
	pjmedia_port *null;
	pjmedia_port *file_port;
	pjmedia_master_port *m;
	pjmedia_port *port;
	pjmedia_conf *conf;
	//slots 0 to 5 - for calls
	// 9 - file port
	unsigned int slot[10];
};
int count=0;
int seqnum;
int expseq;

struct wav_eof_t
{
	pj_pool_t *pool;
	pjmedia_conf *conf;
	pjmedia_port *player;
	unsigned int slot;
};

public ref class IncomingCallEventArgs : EventArgs
{
public:
	int CallID;
	System::String^ DNIS;
	System::String^ ANI;
};

public ref class DtmfReceivedEventArgs : EventArgs
{
public:
	int CallID;
	System::String^ DTMF;
};


public ref class CallDisconnectedEventArgs : EventArgs
{
public:
	int CallID;
};

public delegate void IncomingCallDelegate();
public delegate void IncomingCallEventHandler(IncomingCallEventArgs^ e);
public delegate void CallDisconnectedDelegate(int callID);
public delegate void CallDisconnectedEventHandler(CallDisconnectedEventArgs^ e);
public delegate void DtmfReceivedDelegate(int callID, char dtmf);
public delegate void DtmfReceivedEventHandler(DtmfReceivedEventArgs^ e);

typedef void (__stdcall *INCOMINGCALLCB)();
typedef void (__stdcall *CALLDISCONNECTEDCB)(int callID);
typedef void (__stdcall *DTMFRECEIVEDCB)(int callID, char dtmf);

static INCOMINGCALLCB g_incoming_call;
static DTMFRECEIVEDCB g_dtmf_received;
static CALLDISCONNECTEDCB g_call_disconnected;
//static SAVERECORDEDCALLCB g_save_recorded_call;

static pj_thread_t* g_main_thread;
static pj_bool_t g_main_thread_stop; /* Quit flag. */
static pj_caching_pool cp; /* Global pool factory. */
static pjsip_endpoint *g_endpt; /* SIP endpoint. */
static pjmedia_endpt *g_med_endpt; /* Media endpoint. */
static pj_pool_t *g_pool;

static void call_on_state_changed(pjsip_inv_session *inv, pjsip_event *e);
static void call_on_media_update(pjsip_inv_session *inv, pj_status_t status);
static void call_on_forked(pjsip_inv_session *inv, pjsip_event *e);
static int wave_player_on_eof(pjmedia_port *port, void *user_data);
static pj_bool_t on_rx_request( pjsip_rx_data *rdata );
static pj_bool_t logging_on_rx_msg(pjsip_rx_data *rdata);
static pj_status_t logging_on_tx_msg(pjsip_tx_data *tdata);
static void on_dtmf_digit(pjmedia_stream * stream, void *user_data, int digit);
static void play_file(int callID, unsigned play_index, char *filename);
static void make_call(int callID, char* to_uri, char* request_uri);
static pj_status_t wav_play_cb(int callID, void *user_data, pjmedia_frame *frame);
static void destroy_call(sip_channel_t* channel);
static int main_worker_thread(void *arg);
static sip_channel_t* get_channel(int callID);



static sip_channel_t channels[MAX_CALLS];

static struct app_t
{
    pj_caching_pool	 cp;
    pj_pool_t		*pool;

    pjsip_endpoint	*sip_endpt;
    //pjmedia_endpt	*med_endpt;

    //call_t		 calls[MAX_CALLS];

    pj_bool_t		 quit;
    pj_thread_t		*worker_thread;

    pj_bool_t		 enable_msg_logging;
} app;

static pjsip_module mod_simpleua =
{
	NULL, NULL, /* prev, next. */
	{ "mod-simpleua", 12 }, /* Name. */
	-1, /* Id */
	PJSIP_MOD_PRIORITY_APPLICATION, /* Priority */
	NULL, /* load() */
	NULL, /* start() */
	NULL, /* stop() */
	NULL, /* unload() */
	&on_rx_request, /* on_rx_request() */
	NULL, /* on_rx_response() */
	NULL, /* on_tx_request. */
	NULL, /* on_tx_response() */
	NULL, /* on_tsx_state() */
};

static pjsip_module msg_logger =
{
	NULL, NULL, /* prev, next. */
	{ "mod-msg-log", 13 }, /* Name. */
	-1, /* Id */
	PJSIP_MOD_PRIORITY_TRANSPORT_LAYER-1,/* Priority */
	NULL, /* load() */
	NULL, /* start() */
	NULL, /* stop() */
	NULL, /* unload() */
	&logging_on_rx_msg, /* on_rx_request() */
	&logging_on_rx_msg, /* on_rx_response() */
	&logging_on_tx_msg, /* on_tx_request. */
	&logging_on_tx_msg, /* on_tx_response() */
	NULL, /* on_tsx_state() */ 
};

public ref class PjSipServer
{
private:

	void OnIncomingCall();
	void OnDtmfReceived(int callID, char dtmf);
	void OnCallDisconnected(int callID);
	

public:

	event IncomingCallEventHandler^ IncomingCall;
	event DtmfReceivedEventHandler^ DtmfReceived;
	event CallDisconnectedEventHandler^ CallDisconnected;

	int Start();

	int Answer(int callID);

	int HangUp(int callID,int statusCode);

	int PlayPrompt(int callID);

	int MakeCall(int callID);

	void RecordCall(int callID);

	bool CheckRecorder(int callID);

	void StopRecording(int callID);

	void CreateConference(int callID);

	void RemoveConference(int callID);
};