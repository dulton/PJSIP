//completed tasks
//1. calling
//2. Audio recorder
//3. Audio player
//4. Conference
//5. Sip registation



#include "stdafx.h"
#include "PjSipServer.h"
#include <iostream>


using namespace System;
using namespace System::Net;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

int PjSipServer::Start()
{
	//pj_pool_t *pool = NULL;
	pj_status_t status;
	// Initialising all the channel ids
	for (int j=0;j<MAX_CALLS;j++)
	{
		channels[j].callID = j;
	}

	/* Must init PJLIB first: */
	status = pj_init();
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	pj_log_set_level(5);

	/* Then init PJLIB-UTIL: */
	status = pjlib_util_init();
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	/* Must create a pool factory before we can allocate any memory. */
	pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);


	/* Create global endpoint: */
	{
		const pj_str_t *hostname;
		const char *endpt_name;

		/* Endpoint MUST be assigned a globally unique name.
		* The name will be used as the hostname in Warning header.
		*/

		/* For this implementation, we'll use hostname for simplicity */
		hostname = pj_gethostname();
		endpt_name = hostname->ptr;

		/* Create the endpoint: */

		status = pjsip_endpt_create(&cp.factory, endpt_name, &g_endpt);
		PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
	}


	/*
	* Add UDP transport, with hard-coded port
	* Alternatively, application can use pjsip_udp_transport_attach() to
	* start UDP transport, if it already has an UDP socket (e.g. after it
	* resolves the address with STUN).
	*/
	{
		pj_sockaddr addr;

		pj_sockaddr_init(pj_AF_INET(), &addr, NULL, (pj_uint16_t)SIP_PORT);

		status = pjsip_udp_transport_start(g_endpt, &addr.ipv4, NULL, 1, NULL);

		if (status != PJ_SUCCESS)
		{
			//app_perror(THIS_FILE, "Unable to start UDP transport", status);
			return 1;
		}
	}

	/*
	* Init transaction layer.
	* This will create/initialize transaction hash tables etc.
	*/
	status = pjsip_tsx_layer_init_module(g_endpt);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	/*
	* Initialize UA layer module.
	* This will create/initialize dialog hash tables etc.
	*/
	status = pjsip_ua_init_module(g_endpt, NULL );
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	/*
	* Init invite session module.
	* The invite session module initialization takes additional argument,
	* i.e. a structure containing callbacks to be called on specific
	* occurence of events.
	*
	* The on_state_changed and on_new_session callbacks are mandatory.
	* Application must supply the callback function.
	*
	* We use on_media_update() callback in this application to start
	* media transmission.
	*/
	{
		pjsip_inv_callback inv_cb;

		/* Init the callback for INVITE session: */
		pj_bzero(&inv_cb, sizeof(inv_cb));
		inv_cb.on_state_changed = &call_on_state_changed;
		inv_cb.on_new_session = &call_on_forked;
		inv_cb.on_media_update = &call_on_media_update;		

		/* Initialize invite session module: */
		status = pjsip_inv_usage_init(g_endpt, &inv_cb);
		PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
	}

	/* Initialize 100rel support */
	status = pjsip_100rel_init_module(g_endpt);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);

	/*
	* Register our module to receive incoming requests.
	*/
	status = pjsip_endpt_register_module( g_endpt, &mod_simpleua);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	/*
	* Register message logger module.
	*/
	status = pjsip_endpt_register_module( g_endpt, &msg_logger);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	/*
	* Initialize media endpoint.
	* This will implicitly initialize PJMEDIA too.
	*/
	status = pjmedia_endpt_create(&cp.factory, pjsip_endpt_get_ioqueue(g_endpt), 0, &g_med_endpt);

	/*
	* Add PCMA/PCMU codec to the media endpoint.
	*/
	status = pjmedia_codec_g711_init(g_med_endpt);

	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	// Events in start function to handle incoming call, DTMF recieved, call disconnected
	IncomingCallDelegate^ newCallDel = gcnew IncomingCallDelegate(this, &PjSipServer::OnIncomingCall);
	GCHandle gch = GCHandle::Alloc(newCallDel);
	IntPtr ip = Marshal::GetFunctionPointerForDelegate(newCallDel);
	g_incoming_call = static_cast<INCOMINGCALLCB>(ip.ToPointer());

	DtmfReceivedDelegate^ dtmfDel = gcnew DtmfReceivedDelegate(this, &PjSipServer::OnDtmfReceived);
	gch = GCHandle::Alloc(dtmfDel);
	ip = Marshal::GetFunctionPointerForDelegate(dtmfDel);
	g_dtmf_received = static_cast<DTMFRECEIVEDCB>(ip.ToPointer());

	CallDisconnectedDelegate^ discDel = gcnew CallDisconnectedDelegate(this, &PjSipServer::OnCallDisconnected);
	gch = GCHandle::Alloc(discDel);
	ip = Marshal::GetFunctionPointerForDelegate(discDel);
	g_call_disconnected = static_cast<CALLDISCONNECTEDCB>(ip.ToPointer());

	/* Create main worker thread, suspended. */		

	g_pool = pjmedia_endpt_create_pool(g_med_endpt, "main worker", 512, 512);

	status = pj_thread_create(g_pool, "main_worker_thread", &main_worker_thread, NULL, 0, 0, &g_main_thread);

	if (status != PJ_SUCCESS)
	{
		//app_perror("   error: unable to create thread", status);
		return -620;
	}

	app.worker_thread = g_main_thread;

	return 0;
}

void PjSipServer::OnIncomingCall()
{
	std::cout << "*** incoming call ***" << std::endl;

	// create event args
	IncomingCallEventArgs^ e = gcnew IncomingCallEventArgs();

	// set the call id property
	e->CallID = call_id;
	//e->DNIS = gcnew System::String(callInfo.localUri.c_str());
	//e->ANI = gcnew System::String(callInfo.remoteUri.c_str());

	// raise the incoming call event
	IncomingCall(e);
}


void PjSipServer::OnCallDisconnected(int callID)
{
	std::cout << "*** call disconnected ***" << std::endl;

	// create event args
	CallDisconnectedEventArgs^ e = gcnew CallDisconnectedEventArgs();

	// set the call id property
	//e->CallID = callID;
	//e->DNIS = gcnew System::String(callInfo.localUri.c_str());
	//e->ANI = gcnew System::String(callInfo.remoteUri.c_str());

	// raise the call disconnected event
	CallDisconnected(e);
}

void PjSipServer::OnDtmfReceived(int callID, char dtmf)
{
	std::cout << "*** dtmf received! ***" << std::endl;

	// create event args
	DtmfReceivedEventArgs^ e = gcnew DtmfReceivedEventArgs();

	// set the call id property
	e->CallID = callID;
	e->DTMF = gcnew String(dtmf, 1);

	// raise the call disconnected event
	DtmfReceived(e);
}


//stop recording and save the data
void PjSipServer::StopRecording(int callID)
{
	sip_channel_t* channel = get_channel(call_id);
	pjmedia_port_destroy(channel->file_port);
}


// check if record call button is already pressed or not
bool PjSipServer::CheckRecorder(int callID)
{
	sip_channel_t* channel = get_channel(call_id);
	if(channel->file_port != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int PjSipServer::Answer(int callID)
{
	pjsip_tx_data *tdata;
	sip_channel_t* channel=get_channel(callID);	

	/*
	* Now create 200 response.
	*/
	pj_status_t status = pjsip_inv_answer(channel->inv, 200, NULL, NULL, &tdata);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, PJ_TRUE);

	/*
	* Send the 200 response.
	*/
	status = pjsip_inv_send_msg(channel->inv, tdata);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, PJ_TRUE);
}

int PjSipServer::HangUp( int callID, int statusCode)
{
	//Gopi this has to be fixed 
	call_id=callID-1;
	sip_channel_t* channel = get_channel(callID);

	pjsip_tx_data *tdata;
	const pj_str_t reason = pj_str("Normal Clearing");
	int code;

	if (code==0)
	{
		if (channel->inv->state == PJSIP_INV_STATE_CONFIRMED)
		{
			code = PJSIP_SC_OK;
		}
		else if (channel->inv->role == PJSIP_ROLE_UAS)
		{
			code = PJSIP_SC_DECLINE;
		}
		else
		{
			code = PJSIP_SC_REQUEST_TERMINATED;
		}
	}



	pj_status_t status = pjsip_inv_end_session(channel->inv, statusCode, NULL, &tdata);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, PJ_TRUE);

	/*
	* Send the BYE
	*/
	status = pjsip_inv_send_msg(channel->inv, tdata);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, PJ_TRUE);	

}

int PjSipServer::PlayPrompt(int callID)
{
	play_file(callID, 0, "C:\\Users\\Bret\\Documents\\Welcome.wav");

	return 0;
}

int PjSipServer::MakeCall(int callID)
{
	make_call(callID, "sip:9998@192.168.3.87:5065","sip:9998@192.168.3.87:5065");
	//make_call(callID, "sip:2002@192.168.3.124:32768","sip:9998@192.168.3.124:32768");
	// callID is already increment in the above layer so we dont have to increment it again
	call_id=callID;
	return 0;
}


void PjSipServer::RecordCall(int callID)
{
	/*
	*Reference: http://www.pjsip.org/pjmedia/docs/html/page_pjmedia_samples_recfile_c.htm  
	*/

	sip_channel_t* channel = get_channel(callID);
	pj_status_t	status;
	unsigned int sample_rate = 8000;
	unsigned int channel_count = 2;
	unsigned int number_of_samples_per_frame = sample_rate/50;
	unsigned int bits_per_sample = 16;
	// use buffer size 320 bytes = bits per sample * number of samples per frame / 8
	unsigned int buffer_size = bits_per_sample * number_of_samples_per_frame /8; 
	// Use buffer else it will not record properly
	status=pjmedia_wav_writer_port_create(channel->conf_pool,"C:\\Users\\Bret\\Documents\\callRecording.wav",sample_rate,channel_count,number_of_samples_per_frame,bits_per_sample,0,buffer_size,&channel->file_port);
	//status=pjmedia_wav_writer_port_create(channel->conf_pool,"C:\\Users\\Bret\\Documents\\callRecording.wav",sample_rate,channel_count,number_of_samples_per_frame,bits_per_sample,0,0,&channel->file_port);
	if (status != PJ_SUCCESS)
	{
		std::cout<<"error" <<std::endl;
	}
	pjmedia_conf_add_port(channel->conf, channel->conf_pool, channel->file_port, NULL, &channel->slot[9]);

	//Connects all the ports in the conference to the file recorder
	for(int i=0;i<10;i++)
	{
		if (channel->slot[i] >0)
		{
			pjmedia_conf_connect_port(channel->conf, channel->slot[i], channel->slot[9], 0);
		}
		else
		{
			break;
		}
	}


}

// This has to be invoked only after conference is created else application will break
void PjSipServer::RemoveConference(int callID)
{
	int i=0;
	pj_status_t status;
	// Value is already decremented in above layer so copy the same
	call_id =callID-1;
	for (i=0;i<callID;i++)
	{
		// This adds individual ports back to original call
		pjmedia_conf_add_port(channels[i].conf, channels[i].conf_pool, channels[i].port, NULL, &channels[i].slot[0]);
		status = pjmedia_conf_connect_port(channels[i].conf,0,channels[i].slot[0],0);
		if (status != PJ_SUCCESS)
		{
			//app_perror("Error opening WAV file", status);
			std::cout <<"error"<< std::endl;
		}
		// This removes ports that were added to the conference
		status=pjmedia_conf_remove_port(channels[callID].conf,channels[callID].slot[i]);
		if (status != PJ_SUCCESS)
		{
			//app_perror("Error opening WAV file", status);
			std::cout <<"error"<< std::endl;
		}
	}

	// destoys conference call channel after removing two incoming call ports out 
	//of conferece and adding them back to their original calls
	if(channels[callID].conf != NULL)
		destroy_call(&channels[callID]);

}

void PjSipServer::CreateConference(int callID)
{	
	//copying the value of callID as it is already decremented
	call_id = callID;
	sip_channel_t* channel = get_channel(callID);
	pj_status_t status; 
	unsigned int sample_rate = 8000;
	int i,j=-1;
	channel->conf_pool = pjsip_endpt_create_pool(g_endpt, "conf", 4096, 4096);
	status = pjmedia_conf_create(channel->conf_pool, 10, sample_rate, 1, sample_rate / 50, 16, PJMEDIA_CONF_NO_DEVICE, &channel->conf);	
	channel->cport = pjmedia_conf_get_master_port(channel->conf);

	status = pjmedia_null_port_create(channel->conf_pool, sample_rate, 1, sample_rate / 50, 16, &channel->null);

	status = pjmedia_master_port_create(channel->conf_pool, channel->null, channel->cport, 0, &channel->m);

	status = pjmedia_master_port_start(channel->m);


	//adding ports
	pjmedia_conf_add_port(channel->conf, channel->conf_pool, channels[0].port, NULL, &channel->slot[++j]);

	pjmedia_conf_add_port(channel->conf, channel->conf_pool, channels[1].port, NULL, &channel->slot[++j]);


	// Joining two ports to conference bridge
	for(i = 0 ;i < 10;i++)
	{
		if(channel->slot[i] >0)
		{
			pjmedia_conf_connect_port(channel->conf,0,channel->slot[i],0);
			pjmedia_conf_connect_port(channel->conf,channel->slot[i],0,0);
		}
		else
		{
			break;
		}

	}

	// joining the calls together
	for(i=0;i<=10;i++)
	{
		for(j = 0 ;j < 10;j++)
		{
			if(i!=j)
			{
				if(channel->slot[i] > 0 || channel->slot[j] > 0)		
					pjmedia_conf_connect_port(channel->conf,channel->slot[i],channel->slot[j],0);
				else		
					break;				
			}
		}
	}

	//removes ports from original calls
	for(i=0;i<callID;i++)
	{
		pjmedia_conf_remove_port(channels[i].conf,channels[i].slot[0]);
		if (status != PJ_SUCCESS)
		{
			//app_perror("Error opening WAV file", status);
			std::cout <<"error"<< std::endl;
		}
	}

}


static sip_channel_t* get_channel(int callID)
{
	for (int i=0;i<=MAX_CALLS;i++)
	{
		if (channels[i].callID == callID)
		{
			return &channels[i];
		}
	}
	return NULL;
}


static void play_file(int callID, unsigned play_index, char *filename)
{
	sip_channel_t* channel = get_channel(callID);

	pj_status_t status;

	pjmedia_aud_param param;
	pjmedia_aud_stream *strm = NULL;
	char line[10], *dummy;

	wav_eof_t *user_data = new wav_eof_t();

	user_data->conf = channel->conf;

	if (filename == NULL)	filename = "";


	// create a memory pool to play a wave file (copied from sipsimple: https://github.com/AGProjects/python-sipsimple/blob/7c8bb984bd9f0251f0892ffd85df76c2740b34de/sipsimple/core/_core.ua.pxi)
	user_data->pool = pjsip_endpt_create_pool(g_endpt, "wav", 4096, 4096);

	status = pjmedia_wav_player_port_create(user_data->pool, filename, 0, PJMEDIA_FILE_NO_LOOP, 0, &user_data->player);

	pjmedia_wav_player_set_eof_cb(user_data->player, (void*)user_data, &wave_player_on_eof);

	pjmedia_conf_add_port(channel->conf, g_pool, user_data->player, NULL, &user_data->slot);

	if (status != PJ_SUCCESS)
	{
		//app_perror("Error opening WAV file", status);
		goto on_return;
	}

	// this works!
	// Important step in joining all the ports together
	for(int i=0;i<10;i++)
	{
		if (channel->slot[i] > 0)
		{
			pjmedia_conf_connect_port(channel->conf, user_data->slot, channel->slot[i], 0);
		}
		else
		{
			break;
		}
	}
	return;


	//PJ_LOG(3,(THIS_FILE, "Playback started, press ENTER to stop"));
	//dummy = fgets(line, sizeof(line), stdin);

on_return:
	std::cout << "" << std::endl;
	//if (strm) {
	//pjmedia_aud_stream_stop(strm);
	//pjmedia_aud_stream_destroy(strm);
	//}
	//if (wav)
	//pjmedia_port_destroy(wav);
	//if (pool)pjsip_dlg_inc_lock
	//pj_pool_release(pool);
}

static void make_call(int callID, char* to_uri, char* request_uri)
{
	// no need to increment as it is taken care above just assign call_id with new value
	call_id=callID;
	sip_channel_t* channel = &channels[callID];

	pj_status_t status;
	pj_str_t to = pj_str(to_uri);
	pj_str_t request_line = pj_str(request_uri);
	pj_str_t local_uri = pj_str("sip:simpleuas@192.168.3.87:5060");
	pjsip_dialog *dlg;
	pjmedia_sdp_session *local_sdp;
	pjsip_tx_data *tdata;

	status = pjsip_dlg_create_uac(
		pjsip_ua_instance(),
		&local_uri, /* local URI */
		&local_uri, /* local Contact */
		&to, /* remote URI */
		&request_line, /* remote target */
		&dlg); /* dialog */

	pjsip_dlg_inc_lock(dlg);

	if (status != PJ_SUCCESS)
	{  
		return;
	}

	status = pjmedia_transport_udp_create3(g_med_endpt, pj_AF_INET(), NULL, NULL, RTP_PORT, 0, &channel->transport);

	if (status != PJ_SUCCESS)
	{
		//app_perror(THIS_FILE, "Unable to create media transport", status);
		return;
	}

	/*
	* Get socket info (address, port) of the media transport. We will
	* need this info to create SDP (i.e. the address and port info in
	* the SDP).
	*/
	pjmedia_transport_info med_tpinfo;
	pjmedia_sock_info sock_info;

	pjmedia_transport_info_init(&med_tpinfo);
	pjmedia_transport_get_info(channel->transport, &med_tpinfo);

	pj_memcpy(&sock_info, &med_tpinfo.sock_info, sizeof(pjmedia_sock_info));

	/*
	* Get media capability from media endpoint:
	*/

	status = pjmedia_endpt_create_sdp(
		g_med_endpt, /* the media endpt */
		dlg->pool, /* pool. */
		MAX_MEDIA_CNT, /* # of streams */
		&sock_info, /* RTP sock info */
		&local_sdp); /* the SDP result */	

	status = pjsip_inv_create_uac(dlg, local_sdp, 0, &channel->inv);

	status = pjsip_inv_invite(channel->inv, &tdata);

	status = pjsip_inv_send_msg(channel->inv, tdata);

	/*
	* Invite session has been created, decrement & release dialog lock.
	*/
	pjsip_dlg_dec_lock(dlg);
}

static pj_status_t wav_play_cb(void *user_data, pjmedia_frame *frame)
{
	return pjmedia_port_get_frame((pjmedia_port*)user_data, frame);
}

/* Notification on incoming messages */
static pj_bool_t logging_on_rx_msg(pjsip_rx_data *rdata)
{
	PJ_LOG(4,(THIS_FILE, "RX %d bytes %s from %s %s:%d:\n"
		"%.*s\n"
		"--end msg--",
		rdata->msg_info.len,
		pjsip_rx_data_get_info(rdata),
		rdata->tp_info.transport->type_name,
		rdata->pkt_info.src_name,
		rdata->pkt_info.src_port,
		(int)rdata->msg_info.len,
		rdata->msg_info.msg_buf));

	/* Always return false, otherwise messages will not get processed! */
	return PJ_FALSE;
}

/* Notification on outgoing messages */
static pj_status_t logging_on_tx_msg(pjsip_tx_data *tdata)
{

	/* Important note:
	* tp_info field is only valid after outgoing messages has passed
	* transport layer. So don't try to access tp_info when the module
	* has lower priority than transport layer.
	*/

	PJ_LOG(4,(THIS_FILE, "TX %d bytes %s to %s %s:%d:\n"
		"%.*s\n"
		"--end msg--",
		(tdata->buf.cur - tdata->buf.start),
		pjsip_tx_data_get_info(tdata),
		tdata->tp_info.transport->type_name,
		tdata->tp_info.dst_name,
		tdata->tp_info.dst_port,
		(int)(tdata->buf.cur - tdata->buf.start),
		tdata->buf.start));

	/* Always return success, otherwise message will not get sent! */
	return PJ_SUCCESS;
}

static void call_on_state_changed(pjsip_inv_session *inv, pjsip_event *e)
{
	PJ_UNUSED_ARG(e);

	if (inv->state == PJSIP_INV_STATE_DISCONNECTED)
	{
		PJ_LOG(3,(THIS_FILE, "Call DISCONNECTED [reason=%d (%s)]", inv->cause, pjsip_get_status_text(inv->cause)->ptr));

		//call_t *call = (call_t*)inv->mod_data[mod_simpleua.id];
		sip_channel_t* channel = &channels[call_id];

		destroy_call(channel);

		g_call_disconnected(0);
	}
	else
	{
		PJ_LOG(3,(THIS_FILE, "Call state changed to %s", pjsip_inv_state_name(inv->state)));
	}
}

static void destroy_call(sip_channel_t* channel)
{
	//handle the case if the channel is not initialized i.e. m,conf,null,stream,transport empty	
	if (channel->conf)
	{
		pjmedia_conf_destroy(channel->conf);	

		if (channel->m)
		{
			pjmedia_master_port_stop(channel->m);
			pjmedia_master_port_destroy(channel->m, PJ_FALSE);
		}

		if (channel->null)
		{
			pjmedia_port_destroy(channel->null);
		}

		if (channel->stream)
		{
			pjmedia_stream_destroy(channel->stream);
		}

		if (channel->transport)
		{
			pjmedia_transport_close(channel->transport);
		}

		pj_pool_release(channel->conf_pool);
	}
}

/* This callback is called when dialog has forked. */
static void call_on_forked(pjsip_inv_session *inv, pjsip_event *e)
{
	/* To be done... */
	PJ_UNUSED_ARG(inv);
	PJ_UNUSED_ARG(e);
}

/*
* Callback when incoming requests outside any transactions and any
* dialogs are received. We're only interested to hande incoming INVITE
* request, and we'll reject any other requests with 500 response.
*/
static pj_bool_t on_rx_request (pjsip_rx_data *rdata)
{
	if(rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD){
	pj_sockaddr hostaddr;
	char temp[80], hostip[PJ_INET6_ADDRSTRLEN];
	pj_str_t local_uri;
	pjsip_dialog *dlg;
	pjmedia_sdp_session *local_sdp;
	pjsip_tx_data *tdata;
	unsigned options = 0;
	pj_status_t status;
	// It is important to increment the call_id as it was holding negative value or previous call id


	/*
	* Respond (statelessly) any non-INVITE requests with 500
	*/
	if (rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD) {

		seqnum = rdata->msg_info.cseq->cseq;

		if (rdata->msg_info.msg->line.req.method.id == PJSIP_REGISTER_METHOD && count==0 ) 
		{
			pj_str_t trying = pj_str("OK");
			count++;
			pjsip_hdr *old_hdr,*new_hdr,*hdr_list;
			pjsip_expires_hdr *expr_hdr;
			expr_hdr = pjsip_expires_hdr_create(g_pool,3200);			
			
			hdr_list = (pjsip_hdr *)expr_hdr;
			hdr_list->next  = (pjsip_hdr *)pjsip_expires_hdr_create(g_pool,4800);
			hdr_list->next->next = hdr_list;

			if (hdr_list) {
				const pjsip_hdr *hdr = hdr_list->next;
				while (hdr != hdr_list) {					
						(pjsip_hdr*) pjsip_hdr_clone(g_pool, hdr) ;
					hdr = hdr->next;
					break;
				}
			}
			status = pjsip_endpt_respond_stateless( g_endpt, rdata, PJSIP_SC_OK, &trying, hdr_list, NULL);

			if(status!=PJ_SUCCESS){
				int test = 1;
			}
			return PJ_TRUE;

		}
		if (rdata->msg_info.msg->line.req.method.id == PJSIP_REGISTER_METHOD && count!=0) 
		{
			return PJ_FALSE;

		}
		if (rdata->msg_info.msg->line.req.method.id != PJSIP_ACK_METHOD) {
			pj_str_t reason = pj_str("Simple UA unable to handle this request");

			pjsip_endpt_respond_stateless( g_endpt, rdata,
				500, &reason,
				NULL, NULL);
		}
		return PJ_TRUE;
	}
	//call_id has to be incremented only for new invite but not for register requests
	sip_channel_t *channel = get_channel(++call_id);


	pj_str_t trying = pj_str("Trying");

	pjsip_endpt_respond_stateless( g_endpt, rdata, 100, &trying, NULL, NULL);

	/* Verify that we can handle the request. */
	status = pjsip_inv_verify_request(rdata, &options, NULL, NULL, g_endpt, NULL);

	if (status != PJ_SUCCESS)
	{
		pj_str_t reason = pj_str("Sorry Simple UA can not handle this INVITE");

		pjsip_endpt_respond_stateless(g_endpt, rdata, 500, &reason,	NULL, NULL);

		return PJ_TRUE;
	}

	/*
	* Generate Contact URI
	*/
	if (pj_gethostip(pj_AF_INET(), &hostaddr) != PJ_SUCCESS) {
		//app_perror(THIS_FILE, "Unable to retrieve local host IP", status);
		return PJ_TRUE;
	}
	pj_sockaddr_print(&hostaddr, hostip, sizeof(hostip), 2);

	pj_ansi_sprintf(temp, "<sip:simpleuas@%s:%d>", hostip, SIP_PORT);
	local_uri = pj_str(temp);

	/*
	* Create UAS dialog.
	*/
	status = pjsip_dlg_create_uas( pjsip_ua_instance(), rdata, &local_uri, &dlg);

	if (status != PJ_SUCCESS) {
		pjsip_endpt_respond_stateless(g_endpt, rdata, 500, NULL, NULL, NULL);
		return PJ_TRUE;
	}

	pjsip_dlg_inc_lock(dlg);

	/*
	* Create media transport used to send/receive RTP/RTCP socket.
	* One media transport is needed for each call. Application may
	* opt to re-use the same media transport for subsequent calls.
	*/

	//pjmedia_transport *transport = new pjmedia_transport();


	status = pjmedia_transport_udp_create3(g_med_endpt, pj_AF_INET(), NULL, NULL, RTP_PORT*(call_id+1), 0, &channel->transport);

	if (status != PJ_SUCCESS)
	{
		//app_perror(THIS_FILE, "Unable to create media transport", status);
		return 1;
	}

	//call->transport = transport;

	/*
	* Get socket info (address, port) of the media transport. We will
	* need this info to create SDP (i.e. the address and port info in
	* the SDP).
	*/
	pjmedia_transport_info med_tpinfo;
	pjmedia_sock_info sock_info;

	pjmedia_transport_info_init(&med_tpinfo);
	pjmedia_transport_get_info(channel->transport, &med_tpinfo);

	pj_memcpy(&sock_info, &med_tpinfo.sock_info, sizeof(pjmedia_sock_info));

	/*
	* Get media capability from media endpoint:
	*/

	status = pjmedia_endpt_create_sdp(g_med_endpt, rdata->tp_info.pool, MAX_MEDIA_CNT, &sock_info, &local_sdp);
	pj_assert(status == PJ_SUCCESS);
	if (status != PJ_SUCCESS) {
		pjsip_dlg_dec_lock(dlg);
		return PJ_TRUE;
	}

	/*
	* Create invite session, and pass both the UAS dialog and the SDP
	* capability to the session.
	*/
	//pjsip_inv_session* inv = new pjsip_inv_session();

	status = pjsip_inv_create_uas(dlg, rdata, local_sdp, 0, &channel->inv);

	pj_assert(status == PJ_SUCCESS);

	if (status != PJ_SUCCESS)
	{
		pjsip_dlg_dec_lock(dlg);
		return PJ_TRUE;
	}

	/*
	* Invite session has been created, decrement & release dialog lock.
	*/
	pjsip_dlg_dec_lock(dlg);

	/*
	* Initially send 180 response.
	*
	* The very first response to an INVITE must be created with
	* pjsip_inv_initial_answer(). Subsequent responses to the same
	* transaction MUST use pjsip_inv_answer().
	*/
	status = pjsip_inv_initial_answer(channel->inv, rdata, 180, NULL, NULL, &tdata);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, PJ_TRUE);

	/* Send the 180 response. */
	status = pjsip_inv_send_msg(channel->inv, tdata);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, PJ_TRUE);


	g_incoming_call();

	/* Done.
	* When the call is disconnected, it will be reported via the callback.
	*/}
	
	if(count == 0){
	make_call(0, "sip:2002@192.168.3.124:32768","sip:2002@192.168.3.124:32768");
	count++;
	return PJ_TRUE;
	}
	return PJ_TRUE;
}

/*
* Callback when SDP negotiation has completed.
* We are interested with this callback because we want to start media
* as soon as SDP negotiation is completed.
*/
static void call_on_media_update(pjsip_inv_session *inv, pj_status_t status)
{
	pjmedia_stream_info stream_info;
	const pjmedia_sdp_session *local_sdp;
	const pjmedia_sdp_session *remote_sdp;
	pjmedia_port *media_port;
	unsigned int sample_rate = 8000;
	unsigned int samples_per_frame = sample_rate / 50;

	if (status != PJ_SUCCESS)
	{

		//app_perror(THIS_FILE, "SDP negotiation has failed", status);

		/* Here we should disconnect call if we're not in the middle
		* of initializing an UAS dialog and if this is not a re-INVITE.
		*/
		return;
	}

	sip_channel_t *channel = get_channel(call_id);

	/* Get local and remote SDP.
	* We need both SDPs to create a media session.
	*/
	status = pjmedia_sdp_neg_get_active_local(inv->neg, &local_sdp);

	status = pjmedia_sdp_neg_get_active_remote(inv->neg, &remote_sdp);

	channel->conf_pool = pjsip_endpt_create_pool(g_endpt, "conf", 4096, 4096);

	/* Create stream info based on the media audio SDP. */
	status = pjmedia_stream_info_from_sdp(&stream_info, channel->conf_pool, g_med_endpt, local_sdp, remote_sdp, 0);
	if (status != PJ_SUCCESS) {
		//app_perror(THIS_FILE,"Unable to create audio stream info",status);
		return;
	}

	/* If required, we can also change some settings in the stream info,
	* (such as jitter buffer settings, codec settings, etc) before we
	* create the stream.
	*/

	/* Create new audio media stream, passing the stream info, and also the
	* media socket that we created earlier.
	*/	

	status = pjmedia_stream_create(g_med_endpt, channel->conf_pool, &stream_info, channel->transport, NULL, &channel->stream);
	if (status != PJ_SUCCESS) {
		//app_perror( THIS_FILE, "Unable to create audio stream", status);
		return;
	}

	/* Start the audio stream */
	status = pjmedia_stream_start(channel->stream);
	if (status != PJ_SUCCESS) {
		//app_perror( THIS_FILE, "Unable to start audio stream", status);
		return;
	}

	pjmedia_stream_set_dtmf_callback(channel->stream, &on_dtmf_digit, channel);

	/* Get the media port interface of the audio stream.
	* Media port interface is basicly a struct containing get_frame() and
	* put_frame() function. With this media port interface, we can attach
	* the port interface to conference bridge, or directly to a sound
	* player/recorder device.
	*/
	pjmedia_stream_get_port(channel->stream, &media_port);

	channel->port = media_port;

	// create a conference with NO SOUND DEVICE
	status = pjmedia_conf_create(channel->conf_pool, 10, sample_rate, 1, sample_rate / 50, 16, PJMEDIA_CONF_NO_DEVICE, &channel->conf);	

	// create a conference with sound ???
	//status = pjmedia_conf_create(inv->pool, 3, 16, 1, 2, 16, 0, &call->conf);

	/* Get the port0 of the conference bridge. */
	channel->cport = pjmedia_conf_get_master_port(channel->conf);

	/* Create master port, connecting port0 of the conference bridge to a null port. */
	status = pjmedia_null_port_create(channel->conf_pool, sample_rate, 1, sample_rate / 50, 16, &channel->null);

	// create a master port for the conference with the null media port and the conference port
	status = pjmedia_master_port_create(channel->conf_pool, channel->null, channel->cport, 0, &channel->m);

	// start the media flow
	status = pjmedia_master_port_start(channel->m);

	// add the media port interface to the conference
	pjmedia_conf_add_port(channel->conf, channel->conf_pool, media_port, NULL, &channel->slot[0]);

	// connect the media port to the conference (this will start streaming media)
	pjmedia_conf_connect_port(channel->conf, channel->slot[0], 0, 0);	

}

static void on_dtmf_digit(pjmedia_stream * stream, void *user_data, int digit)
{
	PJ_UNUSED_ARG(stream);

	std::cout << "*** digit press ***" << std::endl;
	sip_channel_t* channel = (sip_channel_t*)user_data;
	char digitChar = (char)digit;
	//call->appendDigit(digitChar);

	g_dtmf_received(0, digitChar);
}

static int wave_player_on_eof(pjmedia_port *port, void *user_data)
{
	wav_eof_t *e = (wav_eof_t*)user_data;

	pjmedia_conf_remove_port(e->conf, e->slot);
	pjmedia_port_destroy(e->player);
	pj_pool_release(e->pool);

	return 1;
}

static int main_worker_thread(void *arg)
{
	int i;
	pj_time_val poll_delay = { 0, 10 };

	PJ_UNUSED_ARG(arg);

	/* Sleep to allow main threads to run. */
	pj_thread_sleep(10);

	while (!g_main_thread_stop)
	{
		pjsip_endpt_handle_events(g_endpt, &poll_delay);
	}

	/* Exhaust responses. */
	for (i=0; i<100; ++i)
	{
		pjsip_endpt_handle_events(g_endpt, &poll_delay);
	}

	return 0;
}