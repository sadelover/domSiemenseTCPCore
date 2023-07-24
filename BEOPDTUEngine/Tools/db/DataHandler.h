#pragma once

#include "DatabaseSession.h"
#include "Tools/CommonDefine.h"

typedef struct IOT_MSG_DATA_TAG
{
	string strMsgID;			//消息ID
	string strMsgTime;			//消息时间
	string strMsgRemark;		//备注信息（短）
	string strFileName;			//文件名 （若为文本数据，则为空）
	string strMsgContent;		//消息内容
	DWORD  dwMsgFailCount;		//消息失败次数
}IOT_MSG_DATA;

typedef struct IOT_MSG_REC_DATA_TAG
{
	string strMsgID;			//消息ID
	string strMsgTime;			//消息时间
	string strMsgContent;		//消息内容
	DWORD  dwMsgFailCount;		//消息失败次数
}IOT_MSG_REC_DATA;

class CDataHandler: public CDatabaseSeesion
{
public:
	CDataHandler(void);
	~CDataHandler(void);

public:
	wstring ReadOrCreateCoreDebugItemValue_Defalut(wstring strItemName, wstring strValue = L"0");
	bool    WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemValue);

	bool	GetOneLatestNormalFile(DTU_DATA_INFO & data, int nTryLimit = 3);			// 其次发送时间最新的文件数据（尝试失败次数达到3次后，不再处理）
	bool	GetOneLatestDataFile(DTU_DATA_INFO & data);									// 发送时间最新的文件数据（可一直尝试）
	bool	GetLatestString(DTU_DATA_INFO & data, int nTryLimit = 3);					// 发送时间最新的文本数据（尝试失败次数达到3次后，不再处理）
	bool	GetLatestReceiveData(DTU_RECEIVE_INFO& data, int nMinute=60);				// 获取最新的一个数据（时间是60分钟内的）
	bool	GetUpdateData(DTU_UPDATE_INFO& data);										// 获取待更新数据

	bool	HandlerOneData(DTU_DATA_INFO & data,bool bResult);							//根据发送结果处理一个数据
	bool	SaveOneSendData(DTU_DATA_INFO & data);										//保存一个待发送的消息
	bool	SaveMulSendData(vector<DTU_DATA_INFO>& vecData);							//保存多个待发送的消息
	bool	SaveOneRecData(DTU_RECEIVE_INFO& data);										//保存收到的消息
	bool	SaveOneUpdateData(DTU_UPDATE_INFO& data);									//保存一个更新消息

	bool	ClearExpireData(string strExpireTime);										//根据时间清理过期数据
	bool	DeleteOneRecData(int nID);													//删除一个数据
	bool	DeleteOneUpdateData(DTU_UPDATE_INFO& data);									//删除一个待更新数据

	//////////////////////////////////////////////////////////////////////////
	bool	ReadUnit01(vector<UNIT01_INFO>& unitList);									//获取Unit01表数据
	bool	ChangeValues(vector<REAL_DATA_INFO>& vecValue);								//修改点
	string	ReadErrCodeRecentMinute(string nMintue = "10");								//获取最近的错误代码
	string	ReadMysqlVariable(string strVariableName);									//获取配置参数

private:
	bool	DeleteOneData(DTU_DATA_INFO & data);										//删除一个数据
	bool	SetDataFailCount(DTU_DATA_INFO & data);										//设置数据失败次数

private:
	Project::Tools::Mutex	m_lock;

};

