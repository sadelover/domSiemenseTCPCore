#include "StdAfx.h"
#include "DTUServerHandlerPool.h"

#include "../Log/Log/IConsoleLog.h"



DTUServerHandlerPool::DTUServerHandlerPool(void)
{
}

DTUServerHandlerPool::~DTUServerHandlerPool(void)
{
	/*for (unsigned int i = 0; i < m_handlerlist.size(); i++)
	{
		CDTUServerHandler* phandle = m_handlerlist[i];
		if(phandle)
		{
			delete phandle;
			phandle = NULL;
		}
	}
	m_handlerlist.clear();*/
}

CDTUServerHandler* DTUServerHandlerPool::GetHandlerByDTUName(string strName) const
{
	for (unsigned int i = 0; i < m_handlerlist.size(); i++)
	{
		CDTUServerHandler* handler = m_handlerlist[i];
		if (strName == handler->GetDTUHandleName()){
			return handler;
		}
	}

	return NULL;
}

void DTUServerHandlerPool::CreateDTUServerHandler( mapentry entry )
{
	CDTUServerHandler* phandler = new CDTUServerHandler;
	phandler->Init(entry);
	m_handlerlist.push_back(phandler);
}
