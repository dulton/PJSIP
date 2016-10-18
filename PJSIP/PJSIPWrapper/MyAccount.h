#pragma once

#include <pjsua2.hpp>
#include "MyCall.h"

using namespace System;
using namespace pj;

typedef void (__stdcall *INCOMINGCALLCB)(OnIncomingCallParam &prm);

class MyAccount : public Account 
{
private:

	INCOMINGCALLCB m_IncomingCallCallback;

public:
	 
	virtual void onRegState(OnRegStateParam &prm);
	virtual void onIncomingCall(OnIncomingCallParam &prm);
	void setIncomingCallCallback(INCOMINGCALLCB cb);
};