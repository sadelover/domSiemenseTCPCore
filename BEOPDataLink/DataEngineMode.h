
#ifndef Beop_DATA_ENGINE_MODE_H__
#define Beop_DATA_ENGINE_MODE_H__

#ifdef WIN32
#pragma once
#endif // WIN32


//冗余处理的角色

class BeopDataEngineMode
{
public:
	enum EngineMode{
		Mode_Slave,	 //从机,slave mode, read only,may copy database from master.
		Mode_Master,  //主机, master mode, 
		Mode_NULL
	};
};


#endif
