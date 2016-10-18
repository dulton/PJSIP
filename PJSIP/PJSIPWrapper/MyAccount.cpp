#include "stdafx.h"
#include "MyAccount.h"
#include "MyCall.h"
#include <pjsua2.hpp>
#include <iostream>

using namespace pj;

void MyAccount::onRegState(OnRegStateParam &prm)
{
    AccountInfo ai = getInfo();
    std::cout << (ai.regIsActive? "*** Register:" : "*** Unregister:")
                << " code=" << prm.code << std::endl;
}

void MyAccount::setIncomingCallCallback(INCOMINGCALLCB cb)
{
	m_IncomingCallCallback = cb;
}

void MyAccount::onIncomingCall(OnIncomingCallParam &prm)
{
	//MyCall* call = new MyCall(*this, prm.callId);

	if (m_IncomingCallCallback)
	{
		m_IncomingCallCallback(prm);
	}
}