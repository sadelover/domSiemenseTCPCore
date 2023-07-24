#pragma once


#include <vector>
using std::vector;

#include "DTUServerHandler.h"
#include "DTUAddressMap.h"
/*
 *
 *
 *缓存处理池。
 *默认10个线程和10个连接来处理请求。
 *
 *
 *
 */

const int default_handler_num = 10;

class DTUServerHandlerPool
{
public:
	DTUServerHandlerPool(void);
	~DTUServerHandlerPool(void);

	void	CreateDTUServerHandler(mapentry entry);

	CDTUServerHandler* GetHandlerByDTUName(string strName) const;

public:
	vector<CDTUServerHandler*>	m_handlerlist;
};

