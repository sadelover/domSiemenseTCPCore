#include "stdafx.h"
#include "DTUDataSender.h"

CDTUtDataSender::CDTUtDataSender(UINT uPort, UINT uBaud, char cParity, UINT uDatabits, UINT uStopbits, DWORD dwCommEvents, UINT uWritebuffersize)
{
}

CDTUtDataSender::~CDTUtDataSender()
{
}

bool CDTUtDataSender::Init()
{
	return false;
}

bool CDTUtDataSender::Exit()
{
	return false;
}

void CDTUtDataSender::PrintLog(const wstring & strLog)
{
}
