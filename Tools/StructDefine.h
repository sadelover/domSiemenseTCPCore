#pragma once

enum E_MODBUS_VALUE_TYPE
{
	E_MODBUS_SIGNED = 0,
	E_MODBUS_UNSIGNED,
	E_MODBUS_BITE,
	E_MODBUS_LONG,
	E_MODBUS_LONG_INVERSE,
	E_MODBUS_FLOAT,
	E_MODBUS_FLOAT_INVERSE,
	E_MODBUS_DOUBLE,
	E_MODBUS_DOUBLE_INVERSE,
	E_MODBUS_STRING,
	E_MODBUS_STRING_INVERSE,
	E_MODBUS_POWERLINK,			//11 争对powerLink 3位寄存器，前两位可读，后一位只写
};

struct _ModbusReadUnit
{
	wstring strPointName;
	unsigned int nSlaveId;
	DWORD dwAddFrom;
	DWORD dwAddTo;
	DWORD dwFuncCode;
	int nReadSuccessCount;
	bool	bHasErrPoint;
	bool	bMultiRead;			//true 是批量读取  false 是单点读取   批量读取错误 切换到单点读一次 再切回批量
	bool	bModbusEnd;
};

struct _ModbusWriteCmd
{
	WORD dwAddFrom;
	WORD dwFuncCode;
	WORD dwValue;
};

enum E_CO3P_PARAMS_TYPE
{
	E_CO3P_IP = 1,
	E_CO3P_ADDR1,
	E_CO3P_ADDR2,
	E_CO3P_TYPE,
	E_CO3P_CMD,
	E_CO3P_INTEGER,
	E_CO3P_DECIMAL,
};

struct _CKECO3PReadUnit
{
	_CKECO3PReadUnit()
	{
		memset(cRecBuffer,0,1024);
		nLength = 0;
		bResponseSuccess = false;
	}
	wstring strPointName;
	WORD	wAddr1;				//管理器地址
	WORD	wAddr2;				//仪表地址
	WORD	wType;				//仪表类型
	WORD	wCmd;				//仪表操作命令
	char	cRecBuffer[1024];	//收到的数据包内容
	int		nLength;			//收到的数据包长度
	int		nReadSuccessCount;
	bool	bResponseSuccess;	//是否成功返回
};

struct _MysqlCrossTableUnit
{
	_MysqlCrossTableUnit()
	{
		nColumnCount = 0;
		nFilterColumnIndex = 0;
		vecFilter.clear();
	}
	wstring strTableName;
	wstring strOrderColumn;
	wstring strFilterColumn;
	int		nColumnCount;
	int		nFilterColumnIndex;
	vector<wstring> vecFilter;
};

enum DTU_CMD_TYPE								//DTU命令类型
{
	DTU_CMD_DEFINE = 0,							//默认
	DTU_CMD_REAL_DATA_SEND,						//实时数据（发送）
	DTU_CMD_REAL_DATA_SYN,						//实时数据（同步）
	DTU_CMD_REAL_DATA_EDIT,						//实时数据（修改）
	DTU_CMD_REAL_DATA_EDIT_MUL,					//实时数据（批量修改）
	DTU_CMD_WARNING_DATA_SEND,					//报警数据(发送)
	DTU_CMD_WARNING_DATA_SYN,					//报警数据(同步)
	DTU_CMD_WARNING_OPERATION_DATA_SEND,		//报警操作(发送)
	DTU_CMD_WARNING_OPERATION_DATA_SYN,			//报警操作(发送)
	DTU_CMD_OPERATION_SEND,						//操作记录（发送）
	DTU_CMD_OPERATION_SYN,						//操作记录（同步）
	DTU_CMD_UNIT_SEND,							//unit01(发送)
	DTU_CMD_UNIT_SYN,							//unit01(同步)
	DTU_CMD_UNIT_EDIT,							//unit01(修改)
	DTU_CMD_UNIT_EDIT_MUL,						//unit01(批量修改)
	DTU_CMD_LOG_SEND,							//Log（发送）
	DTU_CMD_LOG_SYN,							//Log（同步）
	DTU_CMD_HEART_SEND,							//心跳包(发送)
	DTU_CMD_HEART_SYN,							//心跳包(同步)
	DTU_CMD_ERR_LIST,							//ERROR(错误列表)
	DTU_CMD_ERR_CODE,							//ERROR(错误代码)
	DTU_CMD_ERROR_SEND,							//ERROR（发送）
	DTU_CMD_ERROR_SYN,							//ERROR（同步）
	DTU_CMD_REAL_FILE_SEND,						//实时文件（发送）
	DTU_CMD_REAL_FILE_SYN,						//实时文件（同步）
	DTU_CMD_HISTORY_FILE_SEND,					//历史文件（发送）
	DTU_CMD_HISTORY_FILE_SYN,					//历史文件（同步）
	DTU_CMD_UPDATE_EXE,							//更新程序
	DTU_CMD_UPDATE_POINT_CSV,					//更新CSV点表
	DTU_CMD_UPDATE_POINT_EXCEL,					//更新EXCEL点表
	DTU_CMD_UPDATE_DLL,							//更新DLL
	DTU_CMD_UPDATE_S3DB,						//更新s3db
	DTU_CMD_UPLOAD_POINT_CSV,					//上传CSV点表
	DTU_CMD_DELETE_POINT_MUL,					//批量删除点
	DTU_CMD_TIME_SYN,							//同步时钟
	DTU_CMD_RESTART_CORE,						//重启Core
	DTU_CMD_RESTART_DOG,						//重启Dog
	DTU_CMD_RESTART_LOGIC,						//重启Logic
	DTU_CMD_RESTART_DTU,						//重启DTU
	DTU_CMD_RESTART_DTU_ENGINE,					//重启DTUEngine
	DTU_CMD_CORE_STATUS,						//获取服务器状态
	DTU_CMD_VERSION_CORE,						//Core版本号
	DTU_CMD_VERSION_DOG,						//Dog版本号
	DTU_CMD_VERSION_DTU							//DTU版本号
};

//DTU发送结构
typedef struct DTU_DATA_INFO_TAG
{
	DTU_DATA_INFO_TAG()
	{
		strID = "0";
		nSubType = 0;
		nTryCount = 0;
		strTime = "";
		strFileName = "";
		strContent = "";
		nCmdType = DTU_CMD_DEFINE;
	}
	string		strID;							//编号
	string		strTime;						//时间
	string		strFileName;					//文件名
	int			nSubType;						//子类型   0表示数据文件  1表示其他文件
	int			nTryCount;						//尝试次数
	string		strContent;						//文本内容  Ansi
	DTU_CMD_TYPE nCmdType;						//命令类型
} DTU_DATA_INFO;