#pragma once

#include <pjsua2.hpp>
#include <pjmedia.h>
#include <windows.h>

using namespace pj;

class my_pjsua_callback : public pjsua_callback
{
public:

	static void on_stream_created(pjsua_call_id call_id, pjmedia_session *sess, unsigned stream_idx, pjmedia_port **p_port);
};