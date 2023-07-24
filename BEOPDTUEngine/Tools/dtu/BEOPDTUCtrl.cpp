#include "stdafx.h"
#include "BEOPDTUCtrl.h"
#include "Tools/point/sqlite/SqliteAcess.h"

CBEOPDTUCtrl::CBEOPDTUCtrl(CDataHandler * pDataHandler)
	: m_pDataHandler(pDataHandler)
	, m_pDTUtDataSender(NULL)
	, m_pTCPDataSender(NULL)
	//, m_pIotDataSender(NULL)
	, m_pRabbitMQSender(NULL)
	, m_wstrDTUType(L"0")
	, m_bDTUConnectSuccess(false)
	, m_wstrDTUMinType(L"3")
	, m_wstrDTUDisableSendAll(L"1")
	, m_wstrDTUSendAll(L"1")
	, m_wstrDTUEnabled(L"0")
	, m_wstrDTUComPort(L"1")
	, m_wstrDTUChecked(L"0")
	, m_wstrTCPIP(L"beopdata.rnbtech.com.hk")
	, m_wstrTCPPort(L"9500")
	, m_wstrTCPName(L"")
	, m_wstrTCPSendFilePackageSize(L"4000")
	, m_wstrTCPSendFilePackageInterval(L"1000")
	, m_wstrTcpSenderEnabled(L"0")
	, m_wstrIotEnable(L"0")
	, m_wstrIotConnection(L"0")
	, m_wstrRunDTU(L"0")
	, m_wstrRabbitMQSSL(L"0")
	, m_wstrRabbitMQHost(L"")
	, m_wstrRabbitMQUser(L"")
	, m_wstrRabbitMQPsw(L"")
	, m_wstrRabbitMQSend(L"")
	, m_wstrRabbitMQReveive(L"")
	, m_nDebugIndex(0)
{
	// 初始化临界区
	InitializeCriticalSection(&m_csDTUDataSync);
}

CBEOPDTUCtrl::~CBEOPDTUCtrl()
{
}

/*
	根据DTU传输类型 创建不同的类
*/
bool CBEOPDTUCtrl::Init()
{
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		m_wstrDTUMinType = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"DTUMinType", m_wstrDTUMinType);
		m_wstrDTUDisableSendAll = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"DTUDisableSendAll", m_wstrDTUDisableSendAll);
		m_wstrDTUSendAll = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"sendall", m_wstrDTUSendAll);
		m_wstrDTUType = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"DTUType", m_wstrDTUType);
		if (m_wstrDTUType == L"0")													//禁用
		{
			m_pDataHandler->WriteCoreDebugItemValue(L"DTUEnabled", L"0");
			m_pDataHandler->WriteCoreDebugItemValue(L"TcpSenderEnabled", L"0");
			m_pDataHandler->WriteCoreDebugItemValue(L"IotEnable", L"0");
			m_pDataHandler->WriteCoreDebugItemValue(L"RabbitMQEnable", L"0");

			PrintLog(L"ERROR: Transport disable.\r\n");
			return false;
		}
		//else if (m_wstrDTUType == L"1")												//2gDTU(UDP+DDP)
		//{
		//	m_pDataHandler->WriteCoreDebugItemValue(L"DTUEnabled", L"1");
		//	m_pDataHandler->WriteCoreDebugItemValue(L"TcpSenderEnabled", L"0");
		//	m_pDataHandler->WriteCoreDebugItemValue(L"IotEnable", L"0");
		//	m_pDataHandler->WriteCoreDebugItemValue(L"RabbitMQEnable", L"0");
		//	m_pDataHandler->WriteCoreDebugItemValue(L"rundtu", L"1");

		//	m_wstrDTUComPort = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"DTUComPort", m_wstrDTUComPort);
		//	m_wstrDTUChecked = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"DTUChecked", m_wstrDTUChecked);

		//	PrintLog(L"INFO : Transport enable(2gDTU(UDP+DDP)).\r\n");
		//	return true;
		//}
		//else if (m_wstrDTUType == L"2")												//2gDTU(TCP)
		//{
		//	m_pDataHandler->WriteCoreDebugItemValue(L"DTUEnabled", L"1");
		//	m_pDataHandler->WriteCoreDebugItemValue(L"TcpSenderEnabled", L"0");
		//	m_pDataHandler->WriteCoreDebugItemValue(L"IotEnable", L"0");
		//	m_pDataHandler->WriteCoreDebugItemValue(L"RabbitMQEnable", L"0");
		//	m_pDataHandler->WriteCoreDebugItemValue(L"rundtu", L"1");

		//	m_wstrDTUComPort = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"DTUComPort", m_wstrDTUComPort);
		//	m_wstrDTUChecked = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"DTUChecked", m_wstrDTUChecked);
		//	m_wstrTCPName = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderName", m_wstrTCPName);
		//	m_wstrTCPSendFilePackageSize = L"1000";
		//	m_wstrTCPSendFilePackageSize = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"TCPSendFilePackageSize", m_wstrTCPSendFilePackageSize);
		//	m_wstrTCPSendFilePackageInterval = L"2000";
		//	m_wstrTCPSendFilePackageInterval = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"TCPSendFilePackageInterval", m_wstrTCPSendFilePackageInterval);

		//	PrintLog(L"INFO : Transport enable(2gDTU(TCP)).\r\n");
		//	return true;
		//}
		else if (m_wstrDTUType == L"4")												//TCP
		{
			m_pDataHandler->WriteCoreDebugItemValue(L"DTUEnabled", L"0");
			m_pDataHandler->WriteCoreDebugItemValue(L"TcpSenderEnabled", L"1");
			m_pDataHandler->WriteCoreDebugItemValue(L"IotEnable", L"0");
			m_pDataHandler->WriteCoreDebugItemValue(L"RabbitMQEnable", L"0");
			m_pDataHandler->WriteCoreDebugItemValue(L"rundtu", L"0");

			m_wstrTCPName = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderName", m_wstrTCPName);
			m_wstrTCPIP = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderIP", m_wstrTCPIP);
			m_wstrTCPPort = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderPort", m_wstrTCPPort);

			m_wstrTCPSendFilePackageSize = L"4000";
			m_wstrTCPSendFilePackageSize = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"TCPSendFilePackageSize", m_wstrTCPSendFilePackageSize);
			m_wstrTCPSendFilePackageInterval = L"1000";
			m_wstrTCPSendFilePackageInterval = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"TCPSendFilePackageInterval", m_wstrTCPSendFilePackageInterval);
			m_wstrTCPAES256 = L"0";
			m_wstrTCPAES256 = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"TCPAES256", m_wstrTCPAES256);

			//校验参数有效性
			if (m_wstrTCPName.length() <= 1 || m_wstrTCPIP.length() <= 1)
			{
				PrintLog(L"ERROR: Transport (TCP) Param Valid.\r\n");
			}
			else
			{
				PrintLog(L"INFO : Transport enable(TCP)).\r\n");
				if (m_pTCPDataSender == NULL)
					m_pTCPDataSender = new CTCPDataSender(m_wstrTCPIP, m_wstrTCPPort, m_wstrTCPName, m_wstrTCPSendFilePackageSize, m_wstrTCPSendFilePackageInterval, m_wstrTCPAES256);

				if (m_pTCPDataSender)
					return m_pTCPDataSender->Init(OnRecData, (DWORD)this);
			}
			return false;
		}
		else if (m_wstrDTUType == L"5")												//Iot
		{
			
			return false;
		}
		else if (m_wstrDTUType == L"6")												//RabbitMQ
		{
			m_pDataHandler->WriteCoreDebugItemValue(L"DTUEnabled", L"0");
			m_pDataHandler->WriteCoreDebugItemValue(L"TcpSenderEnabled", L"0");
			m_pDataHandler->WriteCoreDebugItemValue(L"IotEnable", L"0");
			m_pDataHandler->WriteCoreDebugItemValue(L"RabbitMQEnable", L"1");
			m_pDataHandler->WriteCoreDebugItemValue(L"rundtu", L"0");

			m_wstrRabbitMQSend = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"RabbitMQSend", m_wstrRabbitMQSend);
			m_wstrRabbitMQReveive = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"RabbitMQReveive", m_wstrRabbitMQReveive);
			m_wstrRabbitMQHost = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"RabbitMQIP", m_wstrRabbitMQHost);
			m_wstrRabbitMQUser = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"RabbitMQUser", m_wstrRabbitMQUser);
			m_wstrRabbitMQPsw = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"RabbitMQPsw", m_wstrRabbitMQPsw);
			m_wstrRabbitMQSSL = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"RabbitMQSSL", m_wstrRabbitMQSSL);

			//校验参数有效性
			if (m_wstrRabbitMQHost.length() <= 1 || m_wstrRabbitMQUser.length() <= 1 || m_wstrRabbitMQPsw.length() <= 1 || m_wstrRabbitMQSend.length() <= 1 || m_wstrRabbitMQReveive.length() <= 1)
			{
				PrintLog(L"ERROR: Transport (RabbitMQ) Param Valid.\r\n");
			}
			else
			{
				PrintLog(L"INFO : Transport enable(RabbitMQ).\r\n");
				if (m_pRabbitMQSender == NULL)
					m_pRabbitMQSender = new CRabbitMQSender(Project::Tools::WideCharToAnsi(m_wstrRabbitMQHost.c_str()), Project::Tools::WideCharToAnsi(m_wstrRabbitMQUser.c_str()), Project::Tools::WideCharToAnsi(m_wstrRabbitMQPsw.c_str()), Project::Tools::WideCharToAnsi(m_wstrRabbitMQSend.c_str()), Project::Tools::WideCharToAnsi(m_wstrRabbitMQReveive.c_str()), _wtoi(m_wstrRabbitMQSSL.c_str()), m_pDataHandler);

				if (m_pRabbitMQSender)
					return m_pRabbitMQSender->Init(OnRecData, (DWORD)this);
			}
			return false;
		}
	}
	return false;
}

bool CBEOPDTUCtrl::Exit()
{
	if (m_wstrDTUType == L"0")													//禁用
	{
	}
	else if (m_wstrDTUType == L"1")												//2gDTU(UDP+DDP)
	{
	}
	else if (m_wstrDTUType == L"2")												//2gDTU(TCP)
	{
		if(m_pTCPDataSender)
		{
			m_pTCPDataSender->Exit();
			delete m_pTCPDataSender;
			m_pTCPDataSender = NULL;
		}
	}
	else if (m_wstrDTUType == L"4")												//TCP
	{
		if(m_pTCPDataSender)
		{
			m_pTCPDataSender->Exit();
			delete m_pTCPDataSender;
			m_pTCPDataSender = NULL;
		}
	}
	else if (m_wstrDTUType == L"5")												//Iot
	{
		
	}
	else if (m_wstrDTUType == L"6")												//RabbitMQ
	{
		if (m_pRabbitMQSender)
		{
			m_pRabbitMQSender->Exit();
			delete m_pRabbitMQSender;
			m_pRabbitMQSender = NULL;
		}
	}
	return true;
}

void CBEOPDTUCtrl::PrintLog(const wstring & strLog)
{
	_tprintf(strLog.c_str());
}

bool CBEOPDTUCtrl::IsConnected()
{
	if (m_wstrDTUType == L"0")													//禁用
	{
	}
	else if (m_wstrDTUType == L"1")												//2gDTU(UDP+DDP)
	{
	}
	else if (m_wstrDTUType == L"2")												//2gDTU(TCP)
	{
		//return m_pTCPDataSender->ReConnect();
	}
	else if (m_wstrDTUType == L"4")												//TCP
	{
		if (m_pTCPDataSender)
			return m_pTCPDataSender->IsConnected();
	}
	else if (m_wstrDTUType == L"5")												//Iot
	{
		//if(m_pIotDataSender)
		//	return m_pIotDataSender->IsConnected();
	}
	else if(m_wstrDTUType == L"6")												//RabbitMQ
	{
		if (m_pRabbitMQSender)
			return m_pRabbitMQSender->IsConnected();
	}
	return false;
}

LRESULT CBEOPDTUCtrl::OnRecData(const unsigned char * pRevData, DWORD dwLength, DWORD dwUserData)
{
	CBEOPDTUCtrl *pDTUCtrl = (CBEOPDTUCtrl *)dwUserData;
	if (pDTUCtrl != NULL)
	{
		//解析Json
		string strJson = (char*)pRevData;
		Json::Reader reader;
		Json::Value value;
		if (reader.parse(strJson, value))
		{
			Value arrayObj = value;
			DTU_RECEIVE_INFO data;
			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			data.strTime = Project::Tools::SystemTimeToString(stNow);
			if (!arrayObj["type"].isNull() && arrayObj["type"].isInt())
			{
				data.nType = arrayObj["type"].asInt();
			}

			if (!arrayObj["cmdtype"].isNull() && arrayObj["cmdtype"].isInt())
			{
				data.nCmdType = arrayObj["cmdtype"].asInt();
			}

			if (!arrayObj["cmdid"].isNull() && arrayObj["cmdid"].isString())
			{
				data.strCmdID = arrayObj["cmdid"].asString();
			}

			if (!arrayObj["content"].isNull() && arrayObj["content"].isString())
			{
				data.strContent = arrayObj["content"].asString();
			}
			pDTUCtrl->StoreDTUCmd(data);
		}
	}
	return 0;
}

bool CBEOPDTUCtrl::StoreDTUCmd(DTU_RECEIVE_INFO & data)
{
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		return m_pDataHandler->SaveOneRecData(data);
	}
	return false;
}

bool CBEOPDTUCtrl::SendOneData(DTU_DATA_INFO & data)
{
	bool bSendSuccess = false;
	if (m_wstrDTUType == L"0")													//禁用
	{

	}
	else if (m_wstrDTUType == L"1")												//2gDTU(UDP+DDP)
	{
	}
	else if (m_wstrDTUType == L"2")												//2gDTU(TCP)
	{
		
	}
	else if (m_wstrDTUType == L"4")												//TCP
	{
		if(m_pTCPDataSender)
			bSendSuccess = m_pTCPDataSender->SendOneData(data);

		if (m_pDataHandler && m_pDataHandler->IsConnected())
			m_pDataHandler->HandlerOneData(data, bSendSuccess);
	}
	else if (m_wstrDTUType == L"5")												//Iot  处理发送结果由自己完成
	{
		//if (m_pIotDataSender)
		//	bSendSuccess = m_pIotDataSender->SendOneData(data);
	}
	else if (m_wstrDTUType == L"6")												//RabbitMQ  处理发送结果由自己完成
	{
		if (m_pRabbitMQSender)
			bSendSuccess = m_pRabbitMQSender->SendOneData(data);

		if (m_pDataHandler && m_pDataHandler->IsConnected())
			m_pDataHandler->HandlerOneData(data, bSendSuccess);
	}
	return bSendSuccess;
}

bool CBEOPDTUCtrl::SendRegData()
{
	std::ostringstream sqlstream;
	SYSTEMTIME stNow;
	GetLocalTime(&stNow);
	string strTime = Project::Tools::SystemTimeToString(stNow);
	sqlstream << DTU_REG_TAG << Project::Tools::WideCharToAnsi(m_wstrTCPName.c_str()) << ";";

	if (m_wstrDTUType == L"0")													//禁用
	{
	}
	else if (m_wstrDTUType == L"1")												//2gDTU(UDP+DDP)
	{
	}
	else if (m_wstrDTUType == L"2")												//2gDTU(TCP)
	{
		return m_pTCPDataSender->SendString(sqlstream.str());
	}
	else if (m_wstrDTUType == L"4")												//TCP
	{
		return m_pTCPDataSender->SendString(sqlstream.str());
	}
	else if (m_wstrDTUType == L"5")												//Iot
	{
		return true;
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerRecCmd()
{
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		DTU_RECEIVE_INFO data;
		m_nDebugIndex++;
		CString strLog;
		strLog.Format(_T("DEBUG: GetLatestReceiveData(%d).\r\n"), m_nDebugIndex);
		//PrintLog(strLog.GetString());
		if (m_pDataHandler->GetLatestReceiveData(data))
		{
			CString strDebug;
			strDebug.Format(_T("DEBUG: HandlerRecCmd(%s).\r\n"), Project::Tools::AnsiToWideChar(data.strContent.c_str()).c_str());
			//PrintLog(strDebug.GetString());
			HandlerRecData(data);
			//PrintLog(L"DEBUG: HandlerRecCmd End.\r\n");
			m_pDataHandler->DeleteOneRecData(data.nID);
			//PrintLog(L"DEBUG: HandlerRecCmd Result End.\r\n");
		}

		DTU_UPDATE_INFO dataUpdate;
		if(m_pDataHandler->GetUpdateData(dataUpdate))
		{
			HandlerUpdateCmd(dataUpdate);
			m_pDataHandler->DeleteOneUpdateData(dataUpdate);
		}
	}
	return true;
}

bool CBEOPDTUCtrl::HandlerRecData(DTU_RECEIVE_INFO& data)
{
	if (data.nType == 0)				//文本数据
	{
		return HandlerString(data);
	}
	else if(data.nType == 1)           //更新文件数据
	{
		return HandlerFile(data);
	}
	else if (data.nType == 2)           //丢失文件数据
	{
		return HandlerLostFile(data);
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerString(DTU_RECEIVE_INFO & data)
{
	if (data.strContent.length() > 0)
	{
		vector<string>	vecCmd;
		Project::Tools::SplitString(data.strContent.c_str(), "|", vecCmd);

		if (vecCmd.size() == 2)
		{
			if (vecCmd[0] == "1")								//同步数据
			{
				int nType = ATOI(vecCmd[1].c_str());
				return HandlerSynRealData((POINT_STORE_CYCLE)nType,data.strCmdID);
			}
			else if (vecCmd[0] == "2"&& vecCmd[1] == "0")		//同步Unit01
			{
				return HandlerSynUnit01(data.strCmdID);
			}
			else if (vecCmd[0] == "3")							//根据文件重发包
			{
				//HandlerReadCSVFileAndSendHistory(vecCmd[1]);
			}
			else if (vecCmd[0] == "4")						//重发丢失实时文件包
			{
				return HandlerAckReSendFileData(vecCmd[1],data.strCmdID);
			}
			else if (vecCmd[0] == "5")						//设置Unit01
			{
				return HandlerChangeUnits(vecCmd[1],data.strCmdID);
			}
			else if (vecCmd[0] == "6")						//批量修改点
			{
				return HandlerChangeValues(vecCmd[1],data.strCmdID);
			}
			else if (vecCmd[0] == "7")							//sql语句
			{
				//ExecuteDTUCmd(vec[1]);
			}
			else if (vecCmd[0] == "8" && vecCmd[1] == "0")	//心跳包请求
			{
				return HandlerAckHeartBeat(data.strCmdID);
			}
			else if (vecCmd[0] == "9")						//回应服务器状态
			{
				return HandlerAckServerState(data.strCmdID);
			}
			else if (vecCmd[0] == "10")						//同步现场时钟
			{
				return HandlerAckSynSystemTime(vecCmd[1],data.strCmdID);
			}
			else if (vecCmd[0] == "11" && vecCmd[1] == "0")		//重启DTU
			{
				HandlerAckReStartDTU(data.strCmdID);
			}
		}
		else if (vecCmd.size() == 3)
		{
			if (vecCmd[0] == "1")													//历史数据
			{
				return HandlerSendHistoryDataFile(vecCmd[1], vecCmd[2],data.strCmdID);
			}
			else if (vecCmd[0] == "2" && vecCmd[1] == "1" && vecCmd[2] == "0")		//重启Logic
			{
				return HandlerRestartLogic(data.strCmdID);
			}
			else if (vecCmd[0] == "3" && vecCmd[1] == "0" && vecCmd[2] == "0")		//重启Core
			{
				return HandlerUpdateRestart(data.strCmdID);
			}
			else if (vecCmd[0] == "3" && vecCmd[1] == "0" && vecCmd[2] == "1")		//重启DTUEngine
			{
				return HandlerRestartDTUEngine(data.strCmdID);
			}
			else if (vecCmd[0] == "3" && vecCmd[1] == "1" && vecCmd[2] == "0")		//Core版本号
			{
				return HandlerAckCoreVersion(data.strCmdID);
			}
			else if (vecCmd[0] == "3" && vecCmd[1] == "1" && vecCmd[2] == "1")		//同步现场点表
			{
				return HandlerAckLocalPoint(data.strCmdID);
			}
			else if (vecCmd[0] == "3" && vecCmd[1] == "1" && vecCmd[2] == "2")		//回应错误文件列表
			{
				return HandlerAckErrFileList(data.strCmdID);
			}
			else if (vecCmd[0] == "3" && vecCmd[1] == "2")							//Core错误代码
			{
				return HandlerAckCoreErrCode(vecCmd[2],data.strCmdID);
			}
			else if (vecCmd[0] == "3" && vecCmd[1] == "3")							//回应错误文件
			{
				return HandlerAckErrFileByName(vecCmd[2],data.strCmdID);
			}
			else if (vecCmd[0] == "3" && vecCmd[1] == "4")							//回应数据库文件
			{
				return HandlerMysqlFileByTableName(vecCmd[2],data.strCmdID);
			}
			else if (vecCmd[0] == "4")													//设置单点
			{
				return HandlerChangeValueByDTUServer(vecCmd[1], vecCmd[2],data.strCmdID);
			}
			else if (vecCmd[0] == "5")												//设置Unit01
			{
				return HandlerChangeUnit(vecCmd[1], vecCmd[2],data.strCmdID);
			}
			else if (vecCmd[0] == "7")												//回应丢失掉线期间数据
			{
				return HandlerAckReSendLostZipData(vecCmd[1], vecCmd[2],data.strCmdID);
			}
			else if (vecCmd[0] == "9")													//回应丢失掉线期间数据
			{
				return HandlerAckReSendLostData(vecCmd[1], vecCmd[2],data.strCmdID);
			}
			else if (vecCmd[0] == "10" && vecCmd[1] == "0")							//增/修点表
			{
				return HandlerUpdateMutilPoint(vecCmd[2],data.strCmdID);
			}
			else if (vecCmd[0] == "10" && vecCmd[1] == "1")							//删除点
			{
				return HandlerDeleteMutilPoint(vecCmd[2],data.strCmdID);
			}
		}
		else if (vecCmd.size() == 4)
		{
			if (vecCmd[0] == "10" && vecCmd[1] == "0")								//增/修点表
			{
				return HandlerUpdateMutilPoint_(vecCmd[2], vecCmd[3],data.strCmdID);
			}
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerFile(DTU_RECEIVE_INFO & data)
{
	if (data.strContent.length() > 0)
	{
		wstring wstrFilePath = Project::Tools::AnsiToWideChar(data.strContent.c_str());
		if (Project::Tools::FindFileExist(wstrFilePath))
		{
			wstring wstrFileFolder;
			Project::Tools::GetSysPath(wstrFileFolder);
			wstrFileFolder += L"\\fileupdate";
			if (Project::Tools::FindOrCreateFolder(wstrFileFolder))
			{
				//解压文件 根据;划分值
				wstrFileFolder += L"\\update";

				//先删除update文件夹
				DelteFoder(wstrFileFolder);
				CString strFilePath = wstrFilePath.c_str();
				HZIP hz = OpenZip(strFilePath, 0);
				SetUnzipBaseDir(hz, wstrFileFolder.c_str());
				ZIPENTRY ze;
				GetZipItem(hz, -1, &ze);
				int numitems = ze.index;
				bool bUpdateDll = false;
				for (int j = 0; j < numitems; ++j)
				{
					GetZipItem(hz, j, &ze);
					ZRESULT zr = UnzipItem(hz, j, ze.name);
					if (zr == ZR_OK)
					{
						CString strExeName = ze.name;
						CString strLog;
						DTU_UPDATE_INFO dataUpdate;
						if (strExeName.Find(_T(".exe")) >= 0)
						{							
							//存储起来
							if(m_pDataHandler && m_pDataHandler->IsConnected())
							{
								dataUpdate.strCmdID = data.strCmdID;
								dataUpdate.strFileName = Project::Tools::WideCharToAnsi(strExeName);
								dataUpdate.strTime = data.strTime;
								m_pDataHandler->SaveOneUpdateData(dataUpdate);
							}
						}
						else if (strExeName.Find(_T(".dcg")) >= 0)
						{
							//存储起来
							if(m_pDataHandler && m_pDataHandler->IsConnected())
							{
								dataUpdate.strCmdID = data.strCmdID;
								dataUpdate.strFileName = Project::Tools::WideCharToAnsi(strExeName);
								dataUpdate.strTime = data.strTime;
								m_pDataHandler->SaveOneUpdateData(dataUpdate);
							}
						}
						else if (strExeName.Find(_T(".xls")) >= 0)
						{
							//存储起来
							if(m_pDataHandler && m_pDataHandler->IsConnected())
							{
								dataUpdate.strCmdID = data.strCmdID;
								dataUpdate.strFileName = Project::Tools::WideCharToAnsi(strExeName);
								dataUpdate.strTime = data.strTime;
								m_pDataHandler->SaveOneUpdateData(dataUpdate);
							}
						}
						else if (strExeName.Find(_T(".csv")) >= 0)
						{
							//存储起来
							if(m_pDataHandler && m_pDataHandler->IsConnected())
							{
								dataUpdate.strCmdID = data.strCmdID;
								dataUpdate.strFileName = Project::Tools::WideCharToAnsi(strExeName);
								dataUpdate.strTime = data.strTime;
								m_pDataHandler->SaveOneUpdateData(dataUpdate);
							}
						}
						else if (strExeName.Find(_T(".bat")) >= 0)
						{
							//存储起来
							if(m_pDataHandler && m_pDataHandler->IsConnected())
							{
								dataUpdate.strCmdID = data.strCmdID;
								dataUpdate.strFileName = Project::Tools::WideCharToAnsi(strExeName);
								dataUpdate.strTime = data.strTime;
								m_pDataHandler->SaveOneUpdateData(dataUpdate);
							}
						}
						else if (strExeName.Find(_T(".s3db")) >= 0)
						{
							//存储起来
							if(m_pDataHandler && m_pDataHandler->IsConnected())
							{
								dataUpdate.strCmdID = data.strCmdID;
								dataUpdate.strFileName = Project::Tools::WideCharToAnsi(strExeName);
								dataUpdate.strTime = data.strTime;
								m_pDataHandler->SaveOneUpdateData(dataUpdate);
							}
						}
					}
				}
				CloseZip(hz);
			}
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerLostFile(DTU_RECEIVE_INFO& data)
{
	if (data.strContent.length() > 0)
	{
		return HandlerReadLostFileAndSendHistory(data.strContent);
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerSynRealData(POINT_STORE_CYCLE nType,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(SynRealData:" << nType << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		return true;
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerSynUnit01(string strCmdID)
{
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		PrintLog(L"RECV : Receive DTUServer Cmd(SynUnit01).\r\n");

		vector<UNIT01_INFO> unitList;
		if (m_pDataHandler->ReadUnit01(unitList))
		{
			if (unitList.size() > 0)
			{
				std::ostringstream sqlstream;
				SYSTEMTIME stNow;
				GetLocalTime(&stNow);
				string strTime = Project::Tools::SystemTimeToString(stNow);
				sqlstream << DTU_UNIT_TAG << "time:" << strTime << ";";
				for (unsigned int i = 0; i < unitList.size(); ++i)
				{
					sqlstream << unitList[i].strProterty01 << "," << unitList[i].strProterty02 << ";";
				}

				Json::Value jsonBody;
				jsonBody["id"] = "";
				jsonBody["type"] = DTU_CMD_UNIT_SYN;
				jsonBody["cmdid"] = strCmdID;
				jsonBody["content"] = sqlstream.str();

				DTU_DATA_INFO data;
				data.strTime = strTime;
				data.strContent = jsonBody.toStyledString();
				return m_pDataHandler->SaveOneSendData(data);
			}
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerReadLostFileAndSendHistory(string strPath,string strCmdID)
{
	if (strPath.length() > 0)
	{
		wstring wstrFilePath = Project::Tools::AnsiToWideChar(strPath.c_str());
		if (Project::Tools::FindFileExist(wstrFilePath))			//csv文件存在
		{
			FILE* pfile = fopen(strPath.c_str(), "r");
			if (pfile)
			{
				fseek(pfile, 0, SEEK_END);
				int dwsize = ftell(pfile);
				rewind(pfile);
				char* filebuffer = new char[dwsize];
				fread(filebuffer, 1, dwsize, pfile);
				fclose(pfile);
				delete filebuffer;
				filebuffer = NULL;

				string strFileNames = filebuffer;
				return HandlerAckReSendLostZipData("1", strFileNames,strCmdID);
			}
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerChangeValues(string strReceive,string strCmdID)
{
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		if (strReceive.length() > 0)
		{
			vector<string> vecCmd;
			Project::Tools::SplitString(strReceive.c_str(), ",", vecCmd);

			std::ostringstream sqlstream;
			sqlstream << "RECV : Receive DTUServer Cmd(ChangeValues):\r\n";

			int nPointSize = vecCmd.size() / 2;
			vector<REAL_DATA_INFO> vecData;
			string strPacketIndex = "0";
			for (int i = 0; i < nPointSize; ++i)
			{
				REAL_DATA_INFO data;
				data.strPointName = vecCmd[2 * i];
				data.strPointValue = vecCmd[2 * i + 1];
				vecData.push_back(data);

				sqlstream << DTU_SPACE << data.strPointName << " --> " << data.strPointValue << "\r\n";

				if (data.strPointName == "DTU_P_INDEX")
				{
					strPacketIndex = data.strPointValue;
				}

			}
			PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

			if (m_pDataHandler->ChangeValues(vecData))
			{
				SYSTEMTIME stNow;
				GetLocalTime(&stNow);
				string strTime = Project::Tools::SystemTimeToString(stNow);

				Json::Value jsonBody;
				jsonBody["id"] = "";
				jsonBody["type"] = DTU_CMD_REAL_DATA_EDIT_MUL;
				jsonBody["cmdid"] = strCmdID;
				jsonBody["content"] = SendCmdState(strTime, 3, 4, strPacketIndex);

				DTU_DATA_INFO data;
				data.strTime = strTime;
				data.strContent = jsonBody.toStyledString();
				return m_pDataHandler->SaveOneSendData(data);
			}
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckHeartBeat(string strCmdID)
{
	PrintLog(L"RECV : Receive DTUServer Cmd(HeartBeat).\r\n");
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		std::ostringstream sqlstrean;
		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		string strTime = Project::Tools::SystemTimeToString(stNow);
		sqlstrean << DTU_HEART_TAG << "time:" << strTime << ";"  << strTime << ";";

		Json::Value jsonBody;
		jsonBody["id"] = "13";
		jsonBody["type"] = DTU_CMD_HEART_SYN;
		jsonBody["cmdid"] = strCmdID;
		jsonBody["content"] = sqlstrean.str();

		DTU_DATA_INFO data;
		data.strTime = strTime;
		data.strContent = jsonBody.toStyledString();
		return m_pDataHandler->SaveOneSendData(data);
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckReSendFileData(string strFileName,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(ReSendFileData:" << strFileName << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		if (strFileName.length() <= 0)
			return false;

		wstring strFolder;
		Project::Tools::GetSysPath(strFolder);
		strFolder += L"\\backup";
		std::ostringstream sqlstream_path;

		//判断文件是否存在的
		sqlstream_path.str("");
		sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str())
			<< "\\" << strFileName.substr(11, 4) << strFileName.substr(15, 2)
			<< strFileName.substr(17, 2) << "\\" << strFileName;
		string strFilePath = sqlstream_path.str();
		if (Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strFilePath.c_str())))
		{
			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["id"] = GetFileInfoPointCont(strFilePath, strFileName);
			jsonBody["type"] = DTU_CMD_REAL_FILE_SYN;
			jsonBody["cmdid"] = strCmdID;
			jsonBody["content"] = strFilePath;

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strFileName = strFileName;
			data.nSubType = 0;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckServerState(string strCmdID)
{
	PrintLog(L"RECV : Receive DTUServer Cmd(GetServerState).\r\n");
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strS3dbPath = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath");
		if (strS3dbPath.length() <= 0)
			return false;

		string strDisk = Project::Tools::WideCharToAnsi(strS3dbPath.c_str());
		strDisk = strDisk.substr(0, 2);

		//获取硬盘空间,内存,CPU利用率
		string strDiskInfo;
		std::ostringstream sqlstrean;
		if (CheckDiskFreeSpace(strDisk, strDiskInfo))
		{

		}
		sqlstrean << strDiskInfo << "|";
		sqlstrean << GetMemoryPervent() << "|" << GetCPUUseage();

		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		string strTime = Project::Tools::SystemTimeToString(stNow);
		
		Json::Value jsonBody;
		jsonBody["id"] = "";
		jsonBody["type"] = DTU_CMD_CORE_STATUS;
		jsonBody["cmdid"] = strCmdID;
		jsonBody["content"] = SendCmdState(strTime,3,8, sqlstrean.str().c_str());

		DTU_DATA_INFO data;
		data.strTime = strTime;
		data.nSubType = 0;
		data.strContent = jsonBody.toStyledString();
		return m_pDataHandler->SaveOneSendData(data);
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerChangeUnits(string strReceive,string strCmdID)
{
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		if (strReceive.length() > 0)
		{
			vector<wstring> vecCmd;
			Project::Tools::SplitStringByChar(Project::Tools::AnsiToWideChar(strReceive.c_str()).c_str(), L',', vecCmd);
			std::ostringstream sqlstream;
			sqlstream << "RECV : Receive DTUServer Cmd(ChangeUnits):\r\n";

			int nPointSize = vecCmd.size() / 2;
			for (int i = 0; i < nPointSize; ++i)
			{
				wstring strName = vecCmd[2 * i];
				wstring strValue = vecCmd[2 * i + 1];

				sqlstream << DTU_SPACE << Project::Tools::WideCharToAnsi(strName.c_str()) << " --> " << Project::Tools::WideCharToAnsi(strValue.c_str()) << "\r\n";
				m_pDataHandler->WriteCoreDebugItemValue(strName, strValue);
			}
			PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["id"] = "";
			jsonBody["type"] = DTU_CMD_UNIT_EDIT_MUL;
			jsonBody["cmdid"] = strCmdID;
			jsonBody["content"] = SendCmdState(strTime, 3, 2, "11");

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerSendHistoryDataFile(string strStart, string strEnd,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(SendHistoryDataFile from " << strStart << " to " << strEnd << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strPath;
		GetSysPath(strPath);
		strPath += L"\\backup";
		if (Project::Tools::FindOrCreateFolder(strPath))
		{
			//将全部数据文件打包成一个压缩包
			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			COleDateTime oleStart, oleEnd;
			COleDateTimeSpan oleSpan(0, 0, 1, 0);
			Project::Tools::String_To_OleTime(Project::Tools::AnsiToWideChar(strStart.c_str()), oleStart);
			Project::Tools::String_To_OleTime(Project::Tools::AnsiToWideChar(strEnd.c_str()), oleEnd);
			vector<FILE_INFO> vecHistory;
			while (oleStart <= oleEnd)
			{
				CString strFilePath, strFileName;
				strFileName.Format(_T("databackup_%d%02d%02d%02d%02d.zip"), oleStart.GetYear(), oleStart.GetMonth(), oleStart.GetDay(), oleStart.GetHour(), oleStart.GetMinute());
				strFilePath.Format(_T("%s\\%d%02d%02d\\%s"), strPath.c_str(), oleStart.GetYear(), oleStart.GetMonth(), oleStart.GetDay(), strFileName);
				if (Project::Tools::FindFileExist(strFilePath.GetString()))
				{
					//存储一个待发送文件
					FILE_INFO file;
					file.strFileName = Project::Tools::WideCharToAnsi(strFileName);
					file.strFilePath = Project::Tools::WideCharToAnsi(strFilePath);
					vecHistory.push_back(file);
				}
				oleStart = oleStart + oleSpan;
			}

			int nFileSize = vecHistory.size();
			for (int i = 0; i < nFileSize; ++i)
			{
				//
				Json::Value jsonBody;
				jsonBody["id"] = GetFileInfoPointCont(vecHistory[i].strFilePath, vecHistory[i].strFileName);
				jsonBody["type"] = DTU_CMD_HISTORY_FILE_SYN;
				jsonBody["cmdid"] = strCmdID;
				jsonBody["content"] = vecHistory[i].strFilePath;
				std::ostringstream sqlstream_remark;
				sqlstream_remark << nFileSize - i  << "/" << nFileSize;
				jsonBody["remark"] = sqlstream_remark.str();

				DTU_DATA_INFO data;
				data.strTime = strTime;
				data.strFileName = vecHistory[i].strFileName;
				data.nSubType = 0;
				data.strContent = jsonBody.toStyledString();
				m_pDataHandler->SaveOneSendData(data);
				Sleep(10);
			}
			return true;
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerChangeUnit(string strParamName, string strParamValue,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(ChangeUnits):\r\n";
	sqlstream << DTU_SPACE << strParamName << " --> " << strParamValue << "\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strName = Project::Tools::AnsiToWideChar(strParamName.c_str());
		wstring strValue = Project::Tools::AnsiToWideChar(strParamValue.c_str());
		m_pDataHandler->WriteCoreDebugItemValue(strName, strValue);

		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		string strTime = Project::Tools::SystemTimeToString(stNow);

		Json::Value jsonBody;
		jsonBody["id"] = "";
		jsonBody["type"] = DTU_CMD_UNIT_EDIT;
		jsonBody["cmdid"] = strCmdID;
		jsonBody["content"] = SendCmdState(strTime, 3, 2, "11");

		DTU_DATA_INFO data;
		data.strTime = strTime;
		data.strContent = jsonBody.toStyledString();
		return m_pDataHandler->SaveOneSendData(data);
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerDeleteMutilPoint(string strPoints,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(DeleteMutilPoint:" << strPoints << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		vector<wstring> vecPoint;
		Project::Tools::SplitStringByChar(Project::Tools::AnsiToWideChar(strPoints.c_str()).c_str(), L',', vecPoint);
		if (vecPoint.size() <= 0)
		{
			return false;
		}
		wstring strS3dbPath = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath");
		if (strS3dbPath.length() <= 0)
			return false;

		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath, strPath);
		CSqliteAcess access(strPath.c_str());
		access.BeginTransaction();
		std::ostringstream sqlstream_point;
		for (int i = 0; i < vecPoint.size(); ++i)
		{
			wstring strPointName = vecPoint[i];
			if (strPointName.length() <= 0)
				continue;
			string strName_Ansi = Project::Tools::WideCharToAnsi(strPointName.c_str());
			sqlstream_point << strName_Ansi << ",";
			if (access.DeletePointByName(strName_Ansi.c_str()) != 0)
			{
				access.RollbackTransaction();
				return false;
			}
		}
		access.CommitTransaction();

		//重启Core
		m_pDataHandler->WriteCoreDebugItemValue(L"restartcoretype", L"5");

		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		string strTime = Project::Tools::SystemTimeToString(stNow);

		Json::Value jsonBody;
		jsonBody["id"] = "";
		jsonBody["type"] = DTU_CMD_DELETE_POINT_MUL;
		jsonBody["cmdid"] = strCmdID;
		jsonBody["content"] = SendCmdState(strTime, 8, 10, "11");

		DTU_DATA_INFO data;
		data.strTime = strTime;
		data.strContent = jsonBody.toStyledString();
		return m_pDataHandler->SaveOneSendData(data);
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerUpdateRestart(string strCmdID)
{
	PrintLog(L"RECV : Receive DTUServer Cmd(Restart).\r\n");
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		m_pDataHandler->WriteCoreDebugItemValue(L"restartcoretype", L"9");
		m_pDataHandler->WriteCoreDebugItemValue(L"restartlogictype", L"9");
		return true;
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerRestartDTUEngine(string strCmdID)
{
	if (m_pRabbitMQSender)
	{
		m_pRabbitMQSender->SetConnectStatus(false);
	}
	return true;
}

bool CBEOPDTUCtrl::HandlerAckCoreVersion(string strCmdID)
{
	PrintLog(L"RECV : Receive DTUServer Cmd(GetCoreVersion).\r\n");
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strVersion = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"version");

		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		string strTime = Project::Tools::SystemTimeToString(stNow);

		Json::Value jsonBody;
		jsonBody["id"] = "";
		jsonBody["type"] = DTU_CMD_VERSION_CORE;
		jsonBody["cmdid"] = strCmdID;
		jsonBody["content"] = SendCmdState(strTime, 3, 10, Project::Tools::WideCharToAnsi(strVersion.c_str()));

		DTU_DATA_INFO data;
		data.strTime = strTime;
		data.strContent = jsonBody.toStyledString();
		return m_pDataHandler->SaveOneSendData(data);
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckLocalPoint(string strCmdID)
{
	PrintLog(L"RECV : Receive DTUServer Cmd(UploadPointFile).\r\n");
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strFolder;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if (Project::Tools::FindOrCreateFolder(strFolder))
		{
			wstring strS3dbPath = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath");
			if (strS3dbPath.length() <= 0)
				return false;

			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath, strPath);
			vector<DataPointEntry> vecPoint;
			ReadDataPoint(strPath, vecPoint);

			wstring strCSVPath;
			CString strZipPath;
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString strFileName;
			strFileName.Format(_T("local_point_%d%02d%02d"), st.wYear, st.wMonth, st.wDay);
			strZipPath.Format(_T("%s\\%s.zip"), strFolder.c_str(), strFileName);
			if (GeneratePointCSV(vecPoint, strFolder, strFileName, strCSVPath))
			{
				HZIP hz = CreateZip(strZipPath, NULL);
				ZipAdd(hz, strFileName, strCSVPath.c_str());
				CloseZip(hz);
				DelteFoder(strCSVPath.c_str());
				
				//存储一个待发送文件
				string strTime = Project::Tools::SystemTimeToString(st);

				Json::Value jsonBody;
				jsonBody["id"] = "";
				jsonBody["type"] = DTU_CMD_UPLOAD_POINT_CSV;
				jsonBody["cmdid"] = strCmdID;
				jsonBody["content"] = Project::Tools::WideCharToAnsi(strZipPath);

				DTU_DATA_INFO data;
				data.strTime = strTime;
				data.strFileName = Project::Tools::WideCharToAnsi(strFileName);
				data.nSubType = 1;
				data.strContent = jsonBody.toStyledString();
				return m_pDataHandler->SaveOneSendData(data);	
			}
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckErrFileList(string strCmdID)
{
	PrintLog(L"RECV : Receive DTUServer Cmd(GetErrFileList).\r\n");
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strFolder;
		GetSysPath(strFolder);
		strFolder += L"\\log";
		if (Project::Tools::FindOrCreateFolder(strFolder))
		{
			CString strFilePath;
			strFilePath.Format(_T("%s\\*.log"), strFolder.c_str());
			std::ostringstream sqlstream_file;
			CFileFind fileFinder;
			BOOL bWorking = fileFinder.FindFile(strFilePath);
			bool bHasLogFile = false;
			while (bWorking)
			{
				bWorking = fileFinder.FindNextFile();
				if (fileFinder.IsDots())
				{
					continue;
				}
				CString strFileName = fileFinder.GetFileName();
				CString strFilePath = fileFinder.GetFilePath();

				CFileStatus rStatus;
				CFile::GetStatus(strFilePath, rStatus);
				if (strFileName.GetLength() > 0)
					sqlstream_file << Project::Tools::WideCharToAnsi(strFileName) << "|" << rStatus.m_size << ",";
			}
			fileFinder.Close();

			//
			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["id"] = "";
			jsonBody["type"] = DTU_CMD_ERR_LIST;
			jsonBody["cmdid"] = strCmdID;
			jsonBody["content"] = SendCmdState(strTime, 3, 12, sqlstream_file.str());

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckCoreErrCode(string nMintue,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(GetErrorCode in " << nMintue << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		string strErrCodes = m_pDataHandler->ReadErrCodeRecentMinute(nMintue);
		//
		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		string strTime = Project::Tools::SystemTimeToString(stNow);

		Json::Value jsonBody;
		jsonBody["id"] = "";
		jsonBody["type"] = DTU_CMD_ERR_CODE;
		jsonBody["cmdid"] = strCmdID;
		jsonBody["content"] = SendCmdState(strTime, 3, 11, strErrCodes);

		DTU_DATA_INFO data;
		data.strTime = strTime;
		data.strContent = jsonBody.toStyledString();
		return m_pDataHandler->SaveOneSendData(data);
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckErrFileByName(string strErrName,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(GetErrFile:" << strErrName << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strFolder;
		GetSysPath(strFolder);
		strFolder += L"\\log";
		if (Project::Tools::FindOrCreateFolder(strFolder))
		{
			std::ostringstream sqlstream_path;
			sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str()) << "\\" << strErrName;
			string strPath = sqlstream_path.str();
			wstring strFilePath = Project::Tools::AnsiToWideChar(strPath.c_str());
			if (Project::Tools::FindFileExist(strFilePath))
			{
				CString strZipPath;
				SYSTEMTIME st;
				GetLocalTime(&st);
				CString strFileName = Project::Tools::AnsiToWideChar(strErrName.c_str()).c_str();
				strFileName = strFileName.Left(strFileName.ReverseFind('.'));
				strZipPath.Format(_T("%s\\%s.zip"), strFolder.c_str(), strFileName);

				HZIP hz = CreateZip(strZipPath, NULL);
				ZipAdd(hz, strFileName, strFilePath.c_str());
				CloseZip(hz);

				//存储一个待发送文件
				string strTime = Project::Tools::SystemTimeToString(st);

				Json::Value jsonBody;
				jsonBody["id"] = "";
				jsonBody["type"] = DTU_CMD_ERROR_SYN;
				jsonBody["cmdid"] = strCmdID;
				jsonBody["content"] = Project::Tools::WideCharToAnsi(strZipPath);

				DTU_DATA_INFO data;
				data.strTime = strTime;
				data.strFileName = Project::Tools::WideCharToAnsi(strFileName);
				data.nSubType = 1;
				data.strContent = jsonBody.toStyledString();
				return m_pDataHandler->SaveOneSendData(data);
			}
			return true;
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckReSendLostZipData(string strMinutes, string strFileNames,string strCmdID)
{
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		vector<string> vec;
		Project::Tools::SplitString(strFileNames.c_str(), ",", vec);

		std::ostringstream sqlstream;
		sqlstream << "RECV : Receive DTUServer Cmd(ReSendLostZipData: " << vec.size() << ")\r\n";
		sqlstream << DTU_SPACE << strFileNames << "\r\n";
		PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

		wstring strFolder;
		Project::Tools::GetSysPath(strFolder);
		strFolder += L"\\backup";
		std::ostringstream sqlstream_path;

		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		string strTime = Project::Tools::SystemTimeToString(stNow);

		//去除重复的
		hash_map<string, int> mapFile;
		for (int i = 0; i < vec.size(); ++i)
		{
			mapFile[vec[i]] = 1;
		}

		int nFileSize = mapFile.size();
		int nIndex = 0;
		hash_map<string, int>::iterator iterFile = mapFile.begin();
		vector<DTU_DATA_INFO>	vecSaveData;
		while (iterFile != mapFile.end())
		{
			string strFileName = (*iterFile).first;
			if (strFileName.length() < 19)
				continue;

			//判断文件是否存在的
			sqlstream_path.str("");
			sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str())
				<< "\\" << strFileName.substr(11, 4) << strFileName.substr(15, 2)
				<< strFileName.substr(17, 2);
			string strFolder_ = sqlstream_path.str();
			sqlstream_path << "\\" << strFileName;
			string strFilePath = sqlstream_path.str();

			if (Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strFilePath.c_str())))
			{
				//存储一个待发送文件

				Json::Value jsonBody;
				jsonBody["id"] = GetFileInfoPointCont(strFilePath, strFileName);
				jsonBody["type"] = DTU_CMD_HISTORY_FILE_SYN;
				jsonBody["cmdid"] = strCmdID;
				jsonBody["content"] = strFilePath;
				std::ostringstream sqlstream_remark;
				sqlstream_remark << nFileSize - nIndex << "/" << nFileSize;
				jsonBody["remark"] = sqlstream_remark.str();

				DTU_DATA_INFO data;
				data.strTime = strTime;
				data.strFileName = strFileName;
				data.nSubType = 0;
				data.strContent = jsonBody.toStyledString();
				vecSaveData.push_back(data);
			}
			iterFile++;
			nIndex++;
		}

		if (m_pDataHandler && m_pDataHandler->IsConnected())
		{
			if(vecSaveData.size() > 0)
				return m_pDataHandler->SaveMulSendData(vecSaveData);
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckReSendLostData(string strMinutes, string strPacketIndex,string strCmdID)
{
	return false;
}

bool CBEOPDTUCtrl::HandlerUpdateMutilPoint(string strPoints,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(DeleteMutilPoint:" << strPoints << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		vector<wstring> vecPoint;
		Project::Tools::SplitStringByChar(Project::Tools::AnsiToWideChar(strPoints.c_str()).c_str(), L';', vecPoint);

		vector<vector<wstring>> vecPoints;
		for (int i = 0; i < vecPoint.size(); ++i)
		{
			wstring strPoints = vecPoint[i];
			vector<wstring> vecPointInfo;
			Project::Tools::SplitStringByChar(strPoints.c_str(), L',', vecPointInfo);
			if (vecPointInfo.size() < g_Point_Length)
				continue;
			vecPoints.push_back(vecPointInfo);
		}
		if (vecPoints.size() > 0)
		{
			vector<DataPointEntry> vecEditPoint;
			if (GeneratePointFromInfo(vecPoints, vecEditPoint))
			{
				if (UpdateMutilPoint(vecEditPoint))
				{
					//回复
					std::ostringstream sqlstream_point;
					for (int i = 0; i < vecEditPoint.size(); ++i)
					{
						sqlstream_point << Project::Tools::WideCharToAnsi(vecEditPoint[i].GetShortName().c_str()) << ",";
					}
					//重启Core
					m_pDataHandler->WriteCoreDebugItemValue(L"restartcoretype", L"5");

					SYSTEMTIME stNow;
					GetLocalTime(&stNow);
					string strTime = Project::Tools::SystemTimeToString(stNow);

					Json::Value jsonBody;
					jsonBody["id"] = "";
					jsonBody["type"] = DTU_CMD_DELETE_POINT_MUL;
					jsonBody["cmdid"] = strCmdID;
					jsonBody["content"] = SendCmdState(strTime, 8, 10, sqlstream_point.str());

					DTU_DATA_INFO data;
					data.strTime = strTime;
					data.strContent = jsonBody.toStyledString();
					return m_pDataHandler->SaveOneSendData(data);
				}
			}
			return true;
		}

	}
	return false;
}

bool CBEOPDTUCtrl::HandlerUpdateMutilPoint_(string strPropertyCount, string strPoints,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(UpdateMutilPoint:" << strPoints << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		if (strPropertyCount.length() <= 0 || strPoints.length() <= 0)
			return false;

		int nPropertyCount = atoi(strPropertyCount.c_str());
		if (nPropertyCount <= 0)
			return false;

		vector<wstring> vecPointInfo;
		Project::Tools::SplitStringByChar(Project::Tools::AnsiToWideChar(strPoints.c_str()).c_str(), L',', vecPointInfo);

		int nUpdatePoint = vecPointInfo.size() / nPropertyCount;
		if (vecPointInfo.size() % nPropertyCount != 0 || nUpdatePoint <= 1)
			return false;

		//header
		std::map<std::string, int> mapColumn;
		for (int i = 0; i < nPropertyCount; ++i)
		{
			string ColumnName = Project::Tools::WideCharToAnsi(vecPointInfo[i].c_str());
			mapColumn[ColumnName] = i;
		}

		//point
		vector<DataPointEntry> vecEditPoint;
		for (int i = 1; i < nUpdatePoint; ++i)
		{
			DataPointEntry entry;
			CString strIndex = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "pointindex")].c_str();
			CString strPhysicalID = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "physicalid")].c_str();
			CString strSource = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "source")].c_str();
			CString strRemark = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "remark")].c_str();
			CString strUnit = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "Unit")].c_str();
			CString strRWProperty = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "RWProperty")].c_str();
			CString strparam1 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param1")].c_str();
			CString strparam2 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param2")].c_str();
			CString strparam3 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param3")].c_str();
			CString strparam4 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param4")].c_str();
			CString strparam5 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param5")].c_str();
			CString strparam6 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param6")].c_str();
			CString strparam7 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param7")].c_str();
			CString strparam8 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param8")].c_str();
			CString strparam9 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param9")].c_str();
			CString strparam10 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param10")].c_str();

			CString strparam15 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "customName")].c_str();
			CString strparam16 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "system")].c_str();
			CString strparam17 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "device")].c_str();
			CString strparam18 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "type")].c_str();

			CString strparam11 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param11")].c_str();
			CString strparam12 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param12")].c_str();
			CString strparam13 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param13")].c_str();
			CString strparam14 = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "param14")].c_str();
			CString strStoreCycle = vecPointInfo[nPropertyCount*i + GetColumnIndex(mapColumn, "storecycle")].c_str();

			int nIndex = _wtoi(strIndex);
			entry.SetPointIndex(nIndex);
			entry.SetShortName(strPhysicalID.GetString());
			entry.SetSourceType(strSource.GetString());
			entry.SetDescription(strRemark.GetString());
			entry.SetUnit(strUnit.GetString());

			if (strRWProperty == L"R")
			{
				entry.SetProperty(PLC_READ);
			}
			else if (strRWProperty == L"W")
			{
				entry.SetProperty(PLC_WRITE);
			}
			else if (strRWProperty == L"R/W")
			{
				entry.SetProperty(PLC_PROP_MAX);
			}
			else
			{
				entry.SetProperty(PLC_READ);
			}
			entry.SetParam(1, strparam1.GetString());
			entry.SetParam(2, strparam2.GetString());
			entry.SetParam(3, strparam3.GetString());
			entry.SetParam(4, strparam4.GetString());
			entry.SetParam(5, strparam5.GetString());
			entry.SetParam(6, strparam6.GetString());
			entry.SetParam(7, strparam7.GetString());
			entry.SetParam(8, strparam8.GetString());
			entry.SetParam(9, strparam9.GetString());
			entry.SetParam(10, strparam10.GetString());
			entry.SetParam(11, strparam11.GetString());
			entry.SetParam(12, strparam12.GetString());
			entry.SetParam(13, strparam13.GetString());
			entry.SetParam(14, strparam14.GetString());
			entry.SetStoreCycle((POINT_STORE_CYCLE)_wtoi(strStoreCycle));
			entry.SetParam(15, strparam15.GetString());
			entry.SetParam(16, strparam16.GetString());
			entry.SetParam(17, strparam17.GetString());
			entry.SetParam(18, strparam18.GetString());
			if (entry.GetShortName().length() > 0)
				vecEditPoint.push_back(entry);
		}

		if (UpdateMutilPoint(vecEditPoint))
		{
			//回复
			std::ostringstream sqlstream_point;
			for (int i = 0; i < vecEditPoint.size(); ++i)
			{
				sqlstream_point << Project::Tools::WideCharToAnsi(vecEditPoint[i].GetShortName().c_str()) << ",";
			}

			//重启Core
			m_pDataHandler->WriteCoreDebugItemValue(L"restartcoretype", L"5");

			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["id"] = "";
			jsonBody["type"] = DTU_CMD_DELETE_POINT_MUL;
			jsonBody["cmdid"] = strCmdID;
			jsonBody["content"] = SendCmdState(strTime, 8, 10, sqlstream_point.str());

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}

	}
	return false;
}

bool CBEOPDTUCtrl::HandlerMysqlFileByTableName(string strTableName,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(GetMysqlFile:" << strTableName << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		string strBaseDir = m_pDataHandler->ReadMysqlVariable("basedir");
		if (strBaseDir.length() > 0)
		{
			wstring strFolder;
			GetSysPath(strFolder);
			strFolder += L"\\Temp";
			if (Project::Tools::FindOrCreateFolder(strFolder))
			{
				//生成命令
				Database_Config  dbparam = m_pDataHandler->GetDatabaseParam();
				std::ostringstream sqlstream;
				string baseDir(strBaseDir.substr(0, 2));
				sqlstream << baseDir << "\n";
				sqlstream << "cd " << strBaseDir << "bin\n";
				sqlstream << "mysqldump -h" << dbparam.strDBIP << " -u" << dbparam.strUserName << " -p" << dbparam.strPsw << " " << dbparam.strDBName << " " << strTableName << " >"
					<< Project::Tools::WideCharToAnsi(strFolder.c_str()) << "/" << strTableName << ".sql";

				CString strConfigPath, strFilePath;
				strConfigPath.Format(_T("%s\\%s.bat"), strFolder.c_str(), Project::Tools::AnsiToWideChar(strTableName.c_str()).c_str());
				strFilePath.Format(_T("%s\\%s.sql"), strFolder.c_str(), Project::Tools::AnsiToWideChar(strTableName.c_str()).c_str());

				CStdioFile	ff;
				char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));
				setlocale(LC_ALL, "chs");
				if (ff.Open(strConfigPath, CFile::modeCreate | CFile::modeWrite))
				{
					ff.WriteString(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
					ff.Close();
					setlocale(LC_ALL, old_locale);
					free(old_locale);

					SHELLEXECUTEINFO ShExecInfo = { 0 };
					ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
					ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					ShExecInfo.hwnd = NULL;
					ShExecInfo.lpVerb = NULL;
					ShExecInfo.lpFile = strConfigPath;
					ShExecInfo.lpParameters = _T("");
					ShExecInfo.lpDirectory = _T("");
					ShExecInfo.nShow = SW_HIDE;
					ShExecInfo.hInstApp = NULL;

					int nResult = ShellExecuteEx(&ShExecInfo);
					WaitForSingleObject(ShExecInfo.hProcess, 600);

					DWORD dwExitCode;
					if (GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode))			//成功导出
					{
						CString strZipPath,strFileName;
						strZipPath.Format(_T("%s\\%s.zip"), strFolder.c_str(), Project::Tools::AnsiToWideChar(strTableName.c_str()).c_str());
						strFileName.Format(_T("%s.sql"), Project::Tools::AnsiToWideChar(strTableName.c_str()).c_str());

						HZIP hz = CreateZip(strZipPath, NULL);
						ZipAdd(hz, strFileName, strFilePath);
						CloseZip(hz);

						//存储一个待发送文件
						SYSTEMTIME st;
						GetLocalTime(&st);
						string strTime = Project::Tools::SystemTimeToString(st);

						Json::Value jsonBody;
						jsonBody["id"] = "";
						jsonBody["type"] = DTU_CMD_LOG_SYN;
						jsonBody["cmdid"] = strCmdID;
						jsonBody["content"] = Project::Tools::WideCharToAnsi(strZipPath);

						DTU_DATA_INFO data;
						data.strTime = strTime;
						data.strFileName = Project::Tools::WideCharToAnsi(strFileName);
						data.nSubType = 1;
						data.strContent = jsonBody.toStyledString();
						m_pDataHandler->SaveOneSendData(data);
					}
				}
				else
				{
					setlocale(LC_ALL, old_locale);
					free(old_locale);
				}

				//删除.sql 和 .bat
				DelteFile(strConfigPath);
				DelteFile(strFilePath);
				return true;
			}
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckSynSystemTime(string strSystemTime,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(SynSystemTime:" << strSystemTime << ")\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strTime = Project::Tools::AnsiToWideChar(strSystemTime.c_str());
		COleDateTime oleNow;
		Project::Tools::String_To_OleTime(strTime, oleNow);
		if (oleNow.GetStatus() == 0)
		{
			ImproveProcPriv();
			SYSTEMTIME st;
			Project::Tools::String_To_SysTime(strSystemTime, st);
			bool bSetSuccess = SetSystemTimeByBat(st);

			//成功后反馈
			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["id"] = "";
			jsonBody["type"] = DTU_CMD_TIME_SYN;
			jsonBody["cmdid"] = strCmdID;
			jsonBody["content"] = SendCmdState(strTime, 3, 9, "0");

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerAckReStartDTU(string strCmdID)
{
	PrintLog(L"RECV : Receive DTUServer Cmd(ReStartDTU).\r\n");
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		//重启DTU 待处理


		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		string strTime = Project::Tools::SystemTimeToString(stNow);

		Json::Value jsonBody;
		jsonBody["id"] = "";
		jsonBody["type"] = DTU_CMD_RESTART_DTU;
		jsonBody["cmdid"] = strCmdID;
		jsonBody["content"] = SendCmdState(strTime, 3, 4, "14");

		DTU_DATA_INFO data;
		data.strTime = strTime;
		data.strContent = jsonBody.toStyledString();
		return m_pDataHandler->SaveOneSendData(data);
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerRestartLogic(string strCmdID)
{
	PrintLog(L"RECV : Receive DTUServer Cmd(ReStartLogic).\r\n");
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strS3dbPath = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath");
		if (strS3dbPath.length() <= 0)
			return false;

		CString strFolderPath = strS3dbPath.c_str();
		strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
		if (FindOrCreateFolder(strFolderPath.GetString()))
		{
			//找到更新文件
			if (FindFile(strFolderPath, _T("BEOPLogicEngine.exe")))
			{
				CString strCurrentPath;
				strCurrentPath.Format(_T("%s\\BEOPLogicEngine.exe"), strFolderPath);
				//重启Watch
				if (RestartLogic(strCurrentPath.GetString()))
				{
					SYSTEMTIME stNow;
					GetLocalTime(&stNow);
					string strTime = Project::Tools::SystemTimeToString(stNow);

					Json::Value jsonBody;
					jsonBody["id"] = "";
					jsonBody["type"] = DTU_CMD_RESTART_LOGIC;
					jsonBody["cmdid"] = strCmdID;
					jsonBody["content"] = SendCmdState(strTime, 3, 11, "12");

					DTU_DATA_INFO data;
					data.strTime = strTime;
					data.strContent = jsonBody.toStyledString();
					return m_pDataHandler->SaveOneSendData(data);
				}
			}
		}
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerChangeValueByDTUServer(string strPointName, string strValue,string strCmdID)
{
	std::ostringstream sqlstream;
	sqlstream << "RECV : Receive DTUServer Cmd(ChangeValues):\r\n";
	sqlstream << DTU_SPACE << strPointName << " --> " << strValue << "\r\n";
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));

	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		if (strPointName.length() > 0)
		{
			vector<REAL_DATA_INFO> vecData;
			REAL_DATA_INFO data;
			data.strPointName = strPointName;
			data.strPointValue = strValue;
			vecData.push_back(data);

			if (m_pDataHandler->ChangeValues(vecData))
			{
				SYSTEMTIME stNow;
				GetLocalTime(&stNow);
				string strTime = Project::Tools::SystemTimeToString(stNow);

				Json::Value jsonBody;
				jsonBody["id"] = "";
				jsonBody["type"] = DTU_CMD_REAL_DATA_EDIT;
				jsonBody["cmdid"] = strCmdID;
				jsonBody["content"] = SendCmdState(strTime, 0, 2, "");

				DTU_DATA_INFO data;
				data.strTime = strTime;
				data.strContent = jsonBody.toStyledString();
				return m_pDataHandler->SaveOneSendData(data);
			}
		}
	}
	return false;
}

string CBEOPDTUCtrl::SendCmdState(const string strSendTime, int nCmdType, int nCmdState, string strPacketIndex)
{
	std::ostringstream sqlstream;
	sqlstream << DTU_ACK_TAG << "time:" << strSendTime << ";" << nCmdType << "," << nCmdState << "," << strPacketIndex << ";";
	return sqlstream.str();
}

bool CBEOPDTUCtrl::CheckDiskFreeSpace(string strDisk, string & strDiskInfo)
{
	strDiskInfo = "";
	unsigned __int64 i64FreeBytesToCaller;
	unsigned __int64 i64TotalBytes;
	unsigned __int64 i64FreeBytes;

	bool fResult = ::GetDiskFreeSpaceEx(
		Project::Tools::AnsiToWideChar(strDisk.c_str()).c_str(),
		(PULARGE_INTEGER)&i64FreeBytesToCaller,
		(PULARGE_INTEGER)&i64TotalBytes,
		(PULARGE_INTEGER)&i64FreeBytes);
	//GetDiskFreeSpaceEx函数，可以获取驱动器磁盘的空间状态,函数返回的是个BOOL类型数据  
	if (fResult)
	{
		int nCount = (float)i64TotalBytes / 1024 / 1024;
		int nSpace = (float)i64FreeBytesToCaller / 1024 / 1024;
		std::ostringstream sqlstream;
		sqlstream << nSpace << "/" << nCount;
		strDiskInfo = sqlstream.str().c_str();
		return true;
	}
	return false;
}

int CBEOPDTUCtrl::GetMemoryPervent()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	return statex.dwMemoryLoad;
}

int CBEOPDTUCtrl::GetCPUUseage()
{
	FILETIME ftIdle, ftKernel, ftUser;
	double fOldCPUIdleTime = 0;
	double fOldCPUKernelTime = 0;
	double fOldCPUUserTime = 0;
	int nCPUUseRate = 0;
	if (GetSystemTimes(&ftIdle, &ftKernel, &ftUser))
	{
		fOldCPUIdleTime = FileTimeToDouble(ftIdle);
		fOldCPUKernelTime = FileTimeToDouble(ftKernel);
		fOldCPUUserTime = FileTimeToDouble(ftUser);

		Sleep(1000);

		if (GetSystemTimes(&ftIdle, &ftKernel, &ftUser))
		{
			double fCPUIdleTime = FileTimeToDouble(ftIdle);
			double fCPUKernelTime = FileTimeToDouble(ftKernel);
			double fCPUUserTime = FileTimeToDouble(ftUser);
			nCPUUseRate = (int)(100.0 - (fCPUIdleTime - fOldCPUIdleTime)
				/ (fCPUKernelTime - fOldCPUKernelTime + fCPUUserTime - fOldCPUUserTime)
				*100.0);
		}
	}
	return nCPUUseRate;
}

double CBEOPDTUCtrl::FileTimeToDouble(FILETIME &filetime)
{
	return (double)(filetime.dwHighDateTime * 4.294967296E9) + (double)filetime.dwLowDateTime;
}

bool CBEOPDTUCtrl::ReadDataPoint(string strS3dbPath, vector<DataPointEntry>& vecPoint)
{
	vecPoint.clear();
	int tempInt = 0;
	DataPointEntry entry;
	CString cstr;
	const char* pchar = NULL;
	CString strParam3;
	CString strSourceType;
	int i = 1;
	CSqliteAcess access(strS3dbPath.c_str());
	ostringstream sqlstream;
	sqlstream << "select * from list_point order by id;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	int nReadWrite = 0;
	const char szFlag[MAX_PATH] = { '\'', '"', ' ', '"', '\'' };
	while (true)
	{
		if (SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		entry.Clear();  //reset entry
						//OPC id
		int pointindex = access.getColumn_Int(0);
		if (pointindex == 0)
		{
			entry.SetPointIndex(i++);
		}
		else
		{
			entry.SetPointIndex(pointindex);
		}
		// get shortname
		if (access.getColumn_Text(1) != NULL)
		{
			string   Physicalid_temp(const_cast<char*>(access.getColumn_Text(1)));
			wstring strBuf = Project::Tools::AnsiToWideChar(Physicalid_temp.c_str());
			entry.SetShortName(strBuf);
		}
		//get point source type
		if (access.getColumn_Text(24) != NULL)
		{
			string   st_temp(const_cast<char*>(access.getColumn_Text(24)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp.c_str());
			entry.SetSourceType(strBuf);
		}

		//plcid name
		if (access.getColumn_Text(14) != NULL)
		{
			string   st_temp1(const_cast<char*>(access.getColumn_Text(14)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp1.c_str());
			entry.SetParam(1, strBuf);
		}

		//param2
		if (access.getColumn_Text(15) != NULL)
		{
			string   st_temp2(const_cast<char*>(access.getColumn_Text(15)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp2.c_str());
			entry.SetParam(2, strBuf);
		}

		//param3
		if (access.getColumn_Text(16) != NULL)
		{
			string   st_temp3(const_cast<char*>(access.getColumn_Text(16)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp3.c_str());
			entry.SetParam(3, strBuf);
		}

		//param4
		if (access.getColumn_Text(17) != NULL)
		{
			string   st_temp4(const_cast<char*>(access.getColumn_Text(17)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp4.c_str());
			entry.SetParam(4, strBuf);
		}

		//param5
		if (access.getColumn_Text(18) != NULL)
		{
			string   st_temp5(const_cast<char*>(access.getColumn_Text(18)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp5.c_str());
			entry.SetParam(5, strBuf);
		}

		//param6
		if (access.getColumn_Text(19) != NULL)
		{
			string   st_temp6(const_cast<char*>(access.getColumn_Text(19)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp6.c_str());
			entry.SetParam(6, strBuf);
		}

		//param7
		if (access.getColumn_Text(20) != NULL)
		{
			string   st_temp7(const_cast<char*>(access.getColumn_Text(20)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp7.c_str());
			entry.SetParam(7, strBuf);
		}

		//param8
		if (access.getColumn_Text(21) != NULL)
		{
			string   st_temp8(const_cast<char*>(access.getColumn_Text(21)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp8.c_str());
			entry.SetParam(8, strBuf);
		}

		//param9
		if (access.getColumn_Text(22) != NULL)
		{
			string   st_temp9(const_cast<char*>(access.getColumn_Text(22)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp9.c_str());
			entry.SetParam(9, strBuf);
		}

		//param10
		if (access.getColumn_Text(23) != NULL)
		{
			string   st_temp10(const_cast<char*>(access.getColumn_Text(23)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp10.c_str());
			entry.SetParam(10, strBuf);
		}

		//read write
		nReadWrite = access.getColumn_Int(10);
		entry.SetProperty((PLCVALUEPROPERTY)nReadWrite);
		//remark
		if (access.getColumn_Text(8) != NULL)
		{
			string   st_remarkp(const_cast<char*>(access.getColumn_Text(8)));

			wstring strBuf = Project::Tools::AnsiToWideChar(st_remarkp.c_str());
			entry.SetDescription(strBuf);
		}

		//unit
		pchar = access.getColumn_Text(7);
		string str;
		if (pchar != NULL)
		{
			str = pchar;
		}
		else
		{
			str = "";
		}

		entry.SetUnit(Project::Tools::AnsiToWideChar(str.c_str()));
		//high
		if (access.getColumn_Text(25) != NULL)
		{
			string   st_high(const_cast<char*>(access.getColumn_Text(25)));
			str = st_high;
			if (str.size() > 0)
			{
				entry.SetHighAlarm(atof(str.c_str()));
			}
		}
		//highhigh -- maxValue
		if (access.getColumn_Text(26) != NULL)
		{
			string   strMaxValue(const_cast<char*>(access.getColumn_Text(26)));
			entry.SetParam(12, Project::Tools::AnsiToWideChar(strMaxValue.c_str()));
		}
		//low -- minValue
		if (access.getColumn_Text(27) != NULL)
		{
			string   strMinValue(const_cast<char*>(access.getColumn_Text(27)));
			entry.SetParam(13, Project::Tools::AnsiToWideChar(strMinValue.c_str()));
		}
		//lowlow -- isVisited
		if (access.getColumn_Text(28) != NULL)
		{
			string   strIsVisited(const_cast<char*>(access.getColumn_Text(28)));
			int nIsVisited = atoi(strIsVisited.c_str());
			stringstream ss;
			ss << nIsVisited;
			ss >> str;
			entry.SetParam(14, Project::Tools::AnsiToWideChar(str.c_str()));
		}
		if (access.getColumn_Text(29))
		{
			POINT_STORE_CYCLE cycle = (POINT_STORE_CYCLE)atoi(access.getColumn_Text(29));
			entry.SetStoreCycle(cycle);
		}
		//
		const char* pParam15 = access.getColumn_Text(33);
		if (pParam15 != NULL)
		{
			string strParam15;
			if (0 == strcmp(szFlag, pParam15))
			{
				strParam15 = "";
			}
			else
			{
				strParam15 = const_cast<char*>(pParam15);
			}
			entry.SetParam(15, Project::Tools::AnsiToWideChar(strParam15.c_str()));
		}
		const char* pParam16 = access.getColumn_Text(34);
		if (pParam16 != NULL)
		{
			int nNum = atoi(pParam16);
			entry.SetParam(16, g_strPointAttrSystem[nNum].GetString());
		}
		const char* pParam17 = access.getColumn_Text(35);
		if (pParam17 != NULL)
		{
			int nNum = atoi(pParam17);
			entry.SetParam(17, g_strPointAttrDevice[nNum].GetString());
		}
		const char* pParam18 = access.getColumn_Text(36);
		if (pParam18 != NULL)
		{
			int nNum = atoi(pParam18);
			entry.SetParam(18, g_strPointAttrType[nNum].GetString());
		}

		entry.SetUnit(Project::Tools::AnsiToWideChar(str.c_str()));
		vecPoint.push_back(entry);
	}
	access.SqlFinalize();
	access.CloseDataBase();
	return true;
}

bool CBEOPDTUCtrl::GeneratePointCSV(vector<DataPointEntry> &vecPoint, wstring strFolder, CString& strCSVName, wstring & strCSVPath)
{
	CString strFilePath, strCSVName_;
	strCSVName_.Format(_T("%s.csv"), strCSVName);
	strFilePath.Format(_T("%s\\%s"), strFolder.c_str(), strCSVName_);
	char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));
	setlocale(LC_CTYPE, "chs");

	CStdioFile file;
	if (file.Open(strFilePath, CFile::modeCreate | CFile::modeWrite, NULL, NULL))
	{
		CString str = _T("pointindex,physicalid,source,remark,Unit,RWProperty,param1,param2,param3,param4,param5,param6,param7,param8,param9,param10,param11,param12,param13,param14,storecycle,customName,system,device,type\n");
		file.Seek(0, CFile::begin);
		file.WriteString(str);
		for (int i = 0; i < vecPoint.size(); ++i)
		{
			DataPointEntry entry = vecPoint[i];
			string strProperty;
			if (entry.GetProperty() == PLC_READ)
			{
				strProperty = "R";
			}
			else if (entry.GetProperty() == PLC_WRITE)
			{
				strProperty = "W";
			}
			else if (entry.GetProperty() == PLC_PROP_MAX)
			{
				strProperty = "R/W";
			}
			std::ostringstream sqlstream_point;
			sqlstream_point << entry.GetPointIndex() << "," << Project::Tools::WideCharToAnsi(entry.GetShortName().c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetSourceType().c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetDescription().c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetUnit().c_str()) << "," << strProperty << "," << Project::Tools::WideCharToAnsi(entry.GetParam(1).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(2).c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(3).c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(4).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(5).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(6).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(7).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(8).c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(9).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(10).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(11).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(12).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(13).c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(14).c_str()) << "," << (int)entry.GetStoreCycle() << "," << Project::Tools::WideCharToAnsi(entry.GetParam(15).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(16).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(17).c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(18).c_str()) << "\n";
			str = Project::Tools::AnsiToWideChar(sqlstream_point.str().c_str()).c_str();
			file.WriteString(str);
		}
		file.Close();
	}
	setlocale(LC_CTYPE, old_locale);
	free(old_locale);
	strCSVPath = strFilePath;
	strCSVName = strCSVName_;
	return true;
}

bool CBEOPDTUCtrl::GeneratePointFromInfo(vector<vector<wstring>> vecPoints, vector<DataPointEntry>& vecPoint)
{
	if (m_pDataHandler && m_pDataHandler->IsConnected())
	{
		wstring strS3dbPath = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath");
		if (strS3dbPath.length() <= 0)
			return false;

		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath, strPath);
		CSqliteAcess access(strPath.c_str());
		vecPoint.clear();
		for (int i = 0; i < vecPoints.size(); ++i)
		{
			vector<wstring> vecPointInfo = vecPoints[i];
			//
			DataPointEntry entry;
			CString strIndex = vecPointInfo[0].c_str();
			CString strPhysicalID = vecPointInfo[1].c_str();
			CString strSource = vecPointInfo[2].c_str();
			CString strRemark = vecPointInfo[3].c_str();
			CString strUnit = vecPointInfo[4].c_str();
			CString strRWProperty = vecPointInfo[5].c_str();
			CString strparam1 = vecPointInfo[6].c_str();
			CString strparam2 = vecPointInfo[7].c_str();
			CString strparam3 = vecPointInfo[8].c_str();
			CString strparam4 = vecPointInfo[9].c_str();
			CString strparam5 = vecPointInfo[10].c_str();
			CString strparam6 = vecPointInfo[11].c_str();
			CString strparam7 = vecPointInfo[12].c_str();
			CString strparam8 = vecPointInfo[13].c_str();
			CString strparam9 = vecPointInfo[14].c_str();
			CString strparam10 = vecPointInfo[15].c_str();
			CString strparam11 = vecPointInfo[16].c_str();
			CString strparam12 = vecPointInfo[17].c_str();
			CString strparam13 = vecPointInfo[18].c_str();
			CString strparam14 = vecPointInfo[19].c_str();

			CString strparam15 = vecPointInfo[21].c_str();
			CString strparam16 = vecPointInfo[22].c_str();
			CString strparam17 = vecPointInfo[23].c_str();
			CString strparam18 = vecPointInfo[24].c_str();
			CString strStoreCycle = vecPointInfo[20].c_str();

			int nIndex = _wtoi(strIndex);
			entry.SetPointIndex(nIndex);
			entry.SetShortName(strPhysicalID.GetString());
			entry.SetSourceType(strSource.GetString());
			entry.SetDescription(strRemark.GetString());
			entry.SetUnit(strUnit.GetString());

			if (strRWProperty == L"R")
			{
				entry.SetProperty(PLC_READ);
			}
			else if (strRWProperty == L"W")
			{
				entry.SetProperty(PLC_WRITE);
			}
			else if (strRWProperty == L"R/W")
			{
				entry.SetProperty(PLC_PROP_MAX);
			}
			else
			{
				entry.SetProperty(PLC_READ);
			}
			entry.SetParam(1, strparam1.GetString());
			entry.SetParam(2, strparam2.GetString());
			entry.SetParam(3, strparam3.GetString());
			entry.SetParam(4, strparam4.GetString());
			entry.SetParam(5, strparam5.GetString());
			entry.SetParam(6, strparam6.GetString());
			entry.SetParam(7, strparam7.GetString());
			entry.SetParam(8, strparam8.GetString());
			entry.SetParam(9, strparam9.GetString());
			entry.SetParam(10, strparam10.GetString());
			entry.SetParam(11, strparam11.GetString());
			entry.SetParam(12, strparam12.GetString());
			entry.SetParam(13, strparam13.GetString());
			entry.SetParam(14, strparam14.GetString());
			entry.SetStoreCycle((POINT_STORE_CYCLE)_wtoi(strStoreCycle));
			entry.SetParam(15, strparam15.GetString());
			entry.SetParam(16, strparam16.GetString());
			entry.SetParam(17, strparam17.GetString());
			entry.SetParam(18, strparam18.GetString());
			if (entry.GetShortName().length() > 0)
				vecPoint.push_back(entry);
		}
		return true;
	}
	return false;
}

string CBEOPDTUCtrl::GetFileInfoPointCont(string strPath, string strFileName)
{
	if (strPath.length() <= 0 || strFileName.length() <= 0)
		return "0";

	int pos = strPath.find_last_of('\\');
	ostringstream strFilePath_;
	strFilePath_ << strPath.substr(0, pos) << "\\FilePoint.ini";
	string strFilePath = strFilePath_.str();
	CString strName = Project::Tools::AnsiToWideChar(strFileName.c_str()).c_str();
	wchar_t charPointCount[256];
	GetPrivateProfileString(L"file", strName.GetString(), L"", charPointCount, 256, Project::Tools::AnsiToWideChar(strFilePath.c_str()).c_str());
	wstring wstrPointCount = charPointCount;
	if (wstrPointCount == L"")
		wstrPointCount = L"0";
	return Project::Tools::WideCharToAnsi(wstrPointCount.c_str());
}

bool CBEOPDTUCtrl::UpdateMutilPoint(vector<DataPointEntry>& vecPoint)
{
	if (vecPoint.size() > 0)
	{
		//
		if (m_pDataHandler && m_pDataHandler->IsConnected())
		{
			wstring strS3dbPath = m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath");
			if (strS3dbPath.length() <= 0)
				return false;

			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath, strPath);
			CSqliteAcess access(strPath.c_str());
			bool bOK = false;
			int nNeedSave = 0;
			int nSaved = 0;
			access.BeginTransaction();
			int nCount = 0;
			CString	strPar16;
			CString	strPar17;
			CString	strPar18;
			for (int i = 0; i < vecPoint.size(); ++i)
			{
				nNeedSave++;
				string strName;
				string strSourceType;
				string strParam1;
				string strParam2;
				string strParam3;
				string strParam4;
				string strParam5;
				string strParam6;
				string strParam7;
				string strParam8;
				string strParam9;
				string strParam10;
				string strParam11;
				string strParam12;
				string strParam13;
				string strParam14;
				string strParam15;
				string strDescription;
				string strUnit;
				const DataPointEntry& info = vecPoint[i];

				//name
				strName = Project::Tools::WideCharToAnsi(info.GetShortName().c_str());
				//SourceType
				strSourceType = Project::Tools::WideCharToAnsi(info.GetSourceType().c_str());
				//param1 点位值
				strParam1 = Project::Tools::WideCharToAnsi(info.GetParam(1).c_str());
				//param2 点位值
				strParam2 = Project::Tools::WideCharToAnsi(info.GetParam(2).c_str());
				//param3 点位值
				strParam3 = Project::Tools::WideCharToAnsi(info.GetParam(3).c_str());
				//param4 点位值
				strParam4 = Project::Tools::WideCharToAnsi(info.GetParam(4).c_str());
				//param5 点位值
				strParam5 = Project::Tools::WideCharToAnsi(info.GetParam(5).c_str());
				//param6 点位值
				strParam6 = Project::Tools::WideCharToAnsi(info.GetParam(6).c_str());
				//param7 点位值
				strParam7 = Project::Tools::WideCharToAnsi(info.GetParam(7).c_str());
				//param8 点位值
				strParam8 = Project::Tools::WideCharToAnsi(info.GetParam(8).c_str());
				//param9 点位值
				strParam9 = Project::Tools::WideCharToAnsi(info.GetParam(9).c_str());
				//param10 点位值
				strParam10 = Project::Tools::WideCharToAnsi(info.GetParam(10).c_str());
				//param11 点位值
				strParam11 = Project::Tools::WideCharToAnsi(info.GetParam(11).c_str());
				//param12 点位值
				strParam12 = Project::Tools::WideCharToAnsi(info.GetParam(12).c_str());
				//param13 点位值
				strParam13 = Project::Tools::WideCharToAnsi(info.GetParam(13).c_str());
				//param14 点位值
				strParam14 = Project::Tools::WideCharToAnsi(info.GetParam(14).c_str());
				//param15 点位值
				strParam15 = Project::Tools::WideCharToAnsi(info.GetParam(15).c_str());
				//param16 点位值
				strPar16.Format(_T("%s"), (info.GetParam(16)).c_str());
				int nParam16 = 0;
				for (int x = 0; x < g_nLenPointAttrSystem; x++)
				{
					if (g_strPointAttrSystem[x] == strPar16)
					{
						nParam16 = x;
						break;
					}
				}
				//param17 点位值
				strPar17.Format(_T("%s"), (info.GetParam(17)).c_str());
				int nParam17 = 0;
				for (int y = 0; y < g_nLenPointAttrDevice; y++)
				{
					if (g_strPointAttrDevice[y] == strPar17)
					{
						nParam17 = y;
						break;
					}
				}
				//param18 点位值
				strPar18.Format(_T("%s"), (info.GetParam(18)).c_str());
				int nParam18 = 0;
				for (int z = 0; z < g_nLenPointAttrType; z++)
				{
					if (g_strPointAttrType[z] == strPar18)
					{
						nParam18 = z;
						break;
					}
				}

				//remark
				strDescription = Project::Tools::WideCharToAnsi(info.GetDescription().c_str());
				//单位 unit
				strUnit = Project::Tools::WideCharToAnsi(info.GetUnit().c_str());
				char szCycle[10] = { 0 };
				sprintf_s(szCycle, sizeof(szCycle), "%d", (int)info.GetStoreCycle());
				assert(strlen(szCycle) > 0);
				int nIndex = 0;
				if (0 == info.GetPointIndex())
				{
					nIndex = i;
				}
				else
				{
					nIndex = info.GetPointIndex();
				}
				access.DeletePointByName(strName.c_str());
				if (access.InsertRecordToOPCPoint(nIndex, 0, strName.c_str(), strSourceType.c_str(), info.GetProperty(), strDescription.c_str(), strUnit.c_str(),
					info.GetHighAlarm(), info.GetHighHighAlarm(), info.GetLowAlarm(), info.GetLowLowAlarm(), strParam1.c_str(),
					strParam2.c_str(), strParam3.c_str(), strParam4.c_str(), strParam5.c_str(), strParam6.c_str(), strParam7.c_str(),
					strParam8.c_str(), strParam9.c_str(), strParam10.c_str(), szCycle, strParam12.c_str(), strParam13.c_str(), strParam14.c_str(),
					strParam15.c_str(), nParam16, nParam17, nParam18) == 0)
				{
					nSaved++;
				}
				else
				{
					CString strLog;
					strLog.Format(_T("Insert xls(%d) Fails."), nIndex);
				}
			}
			if (nNeedSave == nSaved)
			{
				bOK = true;
			}

			if (bOK)
			{
				access.CommitTransaction();
			}
			else
			{
				access.RollbackTransaction();
			}
			return bOK;
		}
	}
	return false;
}

int CBEOPDTUCtrl::GetColumnIndex(std::map<std::string, int> map, string strColumnName)
{
	int nIndex = 0;
	std::map<std::string, int>::iterator iter = map.find(strColumnName);
	if (iter != map.end())
	{
		return iter->second;
	}
	return nIndex;
}

bool CBEOPDTUCtrl::DelteFoder(wstring strDir_)
{
	try
	{
		CString strDir = strDir_.c_str();
		strDir += '\0';
		SHFILEOPSTRUCT    shFileOp;
		memset(&shFileOp, 0, sizeof(shFileOp));
		shFileOp.wFunc = FO_DELETE;
		shFileOp.pFrom = strDir;
		shFileOp.pTo = NULL;
		shFileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
		int nResult = ::SHFileOperation(&shFileOp);
		return nResult == 0;
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::ImproveProcPriv()
{
	HANDLE token;
	//提升权限
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
	{
		return FALSE;
	}
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	::LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &tkp.Privileges[0].Luid);
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(token, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		return FALSE;
	}
	CloseHandle(token);
	return TRUE;
}

bool CBEOPDTUCtrl::SetSystemTimeByBat(SYSTEMTIME st)
{
	CString strConfigPath;
	wstring strPath;
	Project::Tools::GetSysPath(strPath);
	strConfigPath.Format(_T("%s\\runsettime.bat"), strPath.c_str());

	CStdioFile	ff;
	char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));
	setlocale(LC_ALL, "chs");
	if (ff.Open(strConfigPath, CFile::modeCreate | CFile::modeWrite))
	{
		ff.WriteString(_T("@echo off\n"));
		CString str;
		str.Format(_T("date %d/%02d/%02d & time %02d:%02d:%02d\n"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		ff.WriteString(str);
		ff.Close();

		setlocale(LC_ALL, old_locale);
		free(old_locale);

		SHELLEXECUTEINFO ShExecInfo = { 0 };
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = strConfigPath;
		ShExecInfo.lpParameters = _T("");
		ShExecInfo.lpDirectory = _T("");
		ShExecInfo.nShow = SW_HIDE;
		ShExecInfo.hInstApp = NULL;

		int nResult = ShellExecuteEx(&ShExecInfo);
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

		DWORD dwExitCode;
		BOOL bOK = GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
		return bOK;
	}
	setlocale(LC_ALL, old_locale);
	free(old_locale);
	return false;
}

bool CBEOPDTUCtrl::RestartLogic(wstring strPath)
{
	bool bResult = false;
	string strFilePath = Project::Tools::WideCharToAnsi(strPath.c_str());
	int pos = strFilePath.find_last_of('/');
	CString strFileName = Project::Tools::AnsiToWideChar(strFilePath.substr(pos + 1).c_str()).c_str();
	if (m_PrsView.FindProcessByName_other(strFileName))
	{
		m_PrsView.CloseApp(strFileName);
	}

	if (m_PrsView.OpenApp(strPath.c_str()))
	{
		bResult = true;
	}
	else
	{
		bResult = false;
	}
	return bResult;
}

bool CBEOPDTUCtrl::ReConnect()
{
	if (m_wstrDTUType == L"0")													//禁用
	{
	}
	else if (m_wstrDTUType == L"1")												//2gDTU(UDP+DDP)
	{
	}
	else if (m_wstrDTUType == L"2")												//2gDTU(TCP)
	{
		return m_pTCPDataSender->ReConnect();
	}
	else if (m_wstrDTUType == L"4")												//TCP
	{
		if(m_pTCPDataSender)
			return m_pTCPDataSender->ReConnect();
	}
	else if (m_wstrDTUType == L"5")												//Iot
	{
		//if(m_pIotDataSender)
		//	return m_pIotDataSender->ReConnect();
	}
	else if (m_wstrDTUType == L"6")												//RabbitMQ
	{
		if (m_pRabbitMQSender)
			return m_pRabbitMQSender->ReConnect();
	}
	return false;
}

bool CBEOPDTUCtrl::HandlerUpdateCmd( DTU_UPDATE_INFO& data )
{
	if(data.strFileName.length() > 0)
	{
		CString strExeName = Project::Tools::AnsiToWideChar(data.strFileName.c_str()).c_str();
		CString strLog;

		if(strExeName.Find(_T(".exe"))>=0)
		{
			if(strExeName.Find(_T("domcore.exe"))>=0)
			{
				strLog.Format(_T("DTU:Receive Core Update File(%s)..."),strExeName);
				OutPutUpdateLog(strLog);

				string strResultString = "ACK:Update Core Successs";
				UpdateCore(data,strResultString);

				SYSTEMTIME stNow;
				GetLocalTime(&stNow);
				string strTime = Project::Tools::SystemTimeToString(stNow);

				Json::Value jsonBody;
				jsonBody["type"] = DTU_CMD_UPDATE_EXE;
				jsonBody["cmdid"] = data.strCmdID;
				jsonBody["content"] = strResultString;

				DTU_DATA_INFO data;
				data.strTime = strTime;
				data.strContent = jsonBody.toStyledString();
				return m_pDataHandler->SaveOneSendData(data);
			}
			else if(strExeName.Find(_T("BEOPLogicEngine.exe"))>=0)
			{
				strLog.Format(_T("DTU:Receive LogicEngine Update File(%s)..."),strExeName);
				OutPutUpdateLog(strLog);

				string strResultString = "ACK:Update Logic Successs";
				UpdateLogic(data,strResultString);

				SYSTEMTIME stNow;
				GetLocalTime(&stNow);
				string strTime = Project::Tools::SystemTimeToString(stNow);

				Json::Value jsonBody;
				jsonBody["type"] = DTU_CMD_UPDATE_EXE;
				jsonBody["cmdid"] = data.strCmdID;
				jsonBody["content"] = strResultString;

				DTU_DATA_INFO data;
				data.strTime = strTime;
				data.strContent = jsonBody.toStyledString();
				return m_pDataHandler->SaveOneSendData(data);
			}
		}
		else if(strExeName.Find(_T(".dcg"))>=0)
		{
			strLog.Format(_T("DTU:Receive Dll Update File(%s)..."),strExeName);
			OutPutUpdateLog(strLog);

			string strResultString = "ACK:Update DLL Successs";
			UpdateDll(data,strResultString);

			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["type"] = DTU_CMD_UPDATE_DLL;
			jsonBody["cmdid"] = data.strCmdID;
			jsonBody["content"] = strResultString;

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}
		else if(strExeName.Find(_T(".xls"))>=0)
		{
			strLog.Format(_T("DTU:Receive Point Update File(%s)..."),strExeName);
			OutPutUpdateLog(strLog);

			string strResultString = "ACK:Update Point Excel Successs";
			UpdatePointExcel(data,strResultString);

			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["type"] = DTU_CMD_UPDATE_POINT_EXCEL;
			jsonBody["cmdid"] = data.strCmdID;
			jsonBody["content"] = strResultString;

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}
		else if(strExeName.Find(_T(".csv"))>=0)
		{
			strLog.Format(_T("DTU:Receive Point Update File(%s)..."),strExeName);
			OutPutUpdateLog(strLog);

			string strResultString = "ACK:Update Point CSV Successs";
			UpdatePointCSV(data,strResultString);

			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["type"] = DTU_CMD_UPDATE_POINT_CSV;
			jsonBody["cmdid"] = data.strCmdID;
			jsonBody["content"] = strResultString;

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}
		else if(strExeName.Find(_T(".bat"))>=0)
		{
			strLog.Format(_T("DTU:Receive Bat File(%s)..."),strExeName);
			OutPutUpdateLog(strLog);

			string strResultString = "ACK:Update Bat Successs";
			UpdateBat(data,strResultString);

			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["type"] = DTU_CMD_UPDATE_BAT;
			jsonBody["cmdid"] = data.strCmdID;
			jsonBody["content"] = strResultString;

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}
		else if(strExeName.Find(_T(".s3db"))>=0)
		{
			strLog.Format(_T("DTU:Receive S3db Update File(%s)..."),strExeName);
			OutPutUpdateLog(strLog);

			string strResultString = "ACK:Update S3db Successs";
			UpdateS3db(data,strResultString);

			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["type"] = DTU_CMD_UPDATE_S3DB;
			jsonBody["cmdid"] = data.strCmdID;
			jsonBody["content"] = strResultString;

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();
			return m_pDataHandler->SaveOneSendData(data);
		}
	}
	return true;
}

bool CBEOPDTUCtrl::OutPutUpdateLog( CString strLog )
{
	wstring wstrFileFolder;
	Project::Tools::GetSysPath(wstrFileFolder);
	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wstrFileFolder += L"\\fileupdate";
	if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
	{
		char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
		setlocale( LC_ALL, "chs" );

		CString strOut;
		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		strOut.Format(_T("%s ::%s\n"),strTime.c_str(),strLog);

		CString strLogPath;
		strLogPath.Format(_T("%s\\fileupdate.log"),wstrFileFolder.c_str());

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strOut);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
		setlocale( LC_CTYPE, old_locale ); 
		free( old_locale ); 
	}
	return false;
}

bool CBEOPDTUCtrl::CloseExe( CString strExeName )
{
	bool bResult = false;
	if (m_PrsView.FindProcessByName_other(strExeName))
	{
		if(m_PrsView.CloseApp(strExeName))
		{
			bResult = true;
			CString str;
			str.Format(_T("DEBUG: Close Exe(%s) Success.\r\n"),strExeName);
			PrintLog(str.GetString());
		}
		else
		{
			bResult = false;
			CString str;
			str.Format(_T("ERROR: Close Exe(%s) Fail.\r\n"),strExeName);
			PrintLog(str.GetString());
		}
	}
	return bResult;
}

bool CBEOPDTUCtrl::OpenExe( CString strExePath )
{
	if(strExePath.GetLength() > 0)
	{
		CString strExeName = strExePath.Right(strExePath.GetLength()-strExePath.ReverseFind('\\')-1);
		if(m_PrsView.FindProcessByName_other(strExeName))
			return true;
		return m_PrsView.OpenApp(strExePath);
	}
	return false;
}

bool CBEOPDTUCtrl::UpdateCore(DTU_UPDATE_INFO& data,string& strResultString)
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandler && m_pDataHandler->IsConnected())
			{
				strS3dbPath =  m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath",L"");
				if(strS3dbPath.length() <= 0)
				{
					strResultString = "ACK:Update Core Fail(corefilepath is empty)";
					return false;
				}

				CString strFolderPath = strS3dbPath.c_str();
				strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//找到更新文件
					if(FindFile(strUpdateFolder.c_str(),_T("domcore.exe")))
					{
						//拷贝更新文件到运行目录  以.bak结尾 并删除更新文件
						CString strUpdatePath;
						strUpdatePath.Format(_T("%s\\domcore.exe"),strUpdateFolder.c_str());

						CString strCurrentBacPath;
						strCurrentBacPath.Format(_T("%s\\domcore.exe.bak"),strFolderPath);
						if(!DelteFile(strUpdatePath,strCurrentBacPath))
						{
							strResultString = "ACK:Update Core Fail(DelteFile fail)";
							return false;
						}

						//备份当前运行的Core文件到back文件夹
						CString strCurrentPath;
						strCurrentPath.Format(_T("%s\\domcore.exe"),strFolderPath);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);
						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						if(FindOrCreateFolder(strBacFolder))
						{
							CString strBackPath;
							strBackPath.Format(_T("%s\\domcore_%s.exe"),strBacFolder.c_str(),strTime);

							//当前exe存在拷贝一份
							if(FindFile(strFolderPath,_T("domcore.exe")))
							{
								if(!CopyFile(strCurrentPath,strBackPath,FALSE))
								{
									strResultString = "ACK:Update Core Fail(CopyFile fail)";
									return false;
								}
							}
						}

						//关闭当前运行Core
						CloseExe(_T("domcore.exe"));

						//删除当前运行Core
						DelteFoder(strCurrentPath.GetString());

						//重命名.bak为
						int nReslut = rename(Project::Tools::WideCharToAnsi(strCurrentBacPath).c_str(),Project::Tools::WideCharToAnsi(strCurrentPath).c_str());
						if(nReslut != 0)
						{
							strResultString = "ACK:Update Core Fail(rename fail)";
							return false;
						}

						//重启Core
						if(!OpenExe(strCurrentPath))
						{
							strResultString = "ACK:Update Core Fail(Open Core fail)";
							return false;
						}
					}
				}
			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return true;
}

bool CBEOPDTUCtrl::UpdateDll( DTU_UPDATE_INFO& data,string& strResultString )
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandler && m_pDataHandler->IsConnected())
			{
				strS3dbPath =  m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath",L"");
				if(strS3dbPath.length() <= 0)
				{
					strResultString = "ACK:Update Dll Fail(corefilepath is empty)";
					return false;
				}
				CString strFilePath = strS3dbPath.c_str();
				strFilePath = strFilePath.Right(strFilePath.GetLength()-strFilePath.ReverseFind('\\')-1);
				strS3dbName = strFilePath.Left(strFilePath.ReverseFind('.')).GetString();
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//
					CString strNewPath;
					strNewPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strFilePath);
					//先备份一个原s3db到更新文件夹
					if(!CopyFile(strS3dbPath.c_str(),strNewPath,FALSE))
					{
						strResultString = "ACK:Update Dll Fail(CopyFile fail)";
						return false;
					}

					//再更新该备份s3db
					vector<CString> vecDLL,vecDCG;
					FindFile(strUpdateFolder.c_str(),_T("dll"),vecDLL);
					FindFile(strUpdateFolder.c_str(),_T("dcg"),vecDCG);

					SYSTEMTIME sNow;
					GetLocalTime(&sNow);
					CString strTime;
					strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

					if(vecDCG.size()>0)
					{
						//使用.dcg更新备份s3db
						CString strDCGPath,strDCGBackPath,strDCGName;
						strDCGName = vecDCG[0];
						strDCGPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strDCGName);

						int nDllSize,nDeleteParamSize,nInsertParamSize,nThreadSize;
						wstring strDllList,strParamList,strThreadList;

						wchar_t charSize[1024];
						GetPrivateProfileString(L"dll", L"dllSize", L"", charSize, 1024, strDCGPath);
						wstring strSize = charSize;
						if(strSize == L"")
							strSize = L"0";
						nDllSize = _wtoi(strSize.c_str());

						//
						GetPrivateProfileString(L"dll", L"paramterDeleteSize", L"", charSize, 1024, strDCGPath);
						strSize = charSize;
						if(strSize == L"")
							strSize = L"0";
						nDeleteParamSize = _wtoi(strSize.c_str());

						GetPrivateProfileString(L"dll", L"paramterInsertSize", L"", charSize, 1024, strDCGPath);
						strSize = charSize;
						if(strSize == L"")
							strSize = L"0";
						nInsertParamSize = _wtoi(strSize.c_str());

						GetPrivateProfileString(L"dll", L"threadSize", L"", charSize, 1024, strDCGPath);
						strSize = charSize;
						if(strSize == L"")
							strSize = L"0";
						nThreadSize = _wtoi(strSize.c_str());

						bool bUpdateS3db = false;
						if(nDllSize>0)
						{
							wchar_t* charDllList = new wchar_t[nDllSize+1];
							GetPrivateProfileString(L"dll", L"dllList", L"", charDllList, nDllSize, strDCGPath);
							charDllList[nDllSize] = L'\0';
							strDllList = charDllList;
							delete[] charDllList;

							//插入，删除，更新dll
							vector<wstring> vecDll;
							Project::Tools::SplitStringByChar(strDllList.c_str(),L';',vecDll);
							for(int i=0; i<vecDll.size(); ++i)
							{
								wstring strDllInfo = vecDll[i];
								vector<wstring> vecDllInfo;
								Project::Tools::SplitStringByChar(strDllInfo.c_str(),L',',vecDllInfo);
								if(vecDllInfo.size() == 3)
								{
									CString strDllName = vecDllInfo[1].c_str();
									if(!UpdateDll(_wtoi(vecDllInfo[0].c_str()),strDllName,vecDllInfo[2].c_str(),strNewPath))
									{
										strResultString = "ACK:Update Dll Fail(UpdateDll fail)";
									}

									if(_wtoi(vecDllInfo[0].c_str()) == 0)
									{
										CString strDllPath,strDllBackPath;
										strDllPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strDllName);
										//删除改.dll
										CString strName = strDllName.Left(strDllName.ReverseFind('.'));
										CString strPriName = strDllName.Right(strDllName.GetLength() - strDllName.ReverseFind('.') - 1);
										strDllBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
										DelteFile(strDllPath,strDllBackPath);
									}
									bUpdateS3db = true;
								}
							}

							//
						}

						if(nDeleteParamSize>0)
						{
							wchar_t* charParamList = new wchar_t[nDeleteParamSize+1];
							GetPrivateProfileString(L"dll", L"paramterDelete", L"", charParamList, nDeleteParamSize, strDCGPath);
							charParamList[nDeleteParamSize] = L'\0';
							strParamList = charParamList;
							delete[] charParamList;

							//删除策略关系
							if(!DeleteDllParamter(strNewPath,strParamList))
							{
								strResultString = "ACK:Update Dll Fail(Delete Dll Params fail)";
							}
							bUpdateS3db = true;
						}

						if(nInsertParamSize>0)
						{
							wchar_t* charParamList = new wchar_t[nInsertParamSize+1];
							GetPrivateProfileString(L"dll", L"paramterInsert", L"", charParamList, nInsertParamSize, strDCGPath);
							charParamList[nInsertParamSize] = L'\0';
							strParamList = charParamList;
							delete[] charParamList;

							//插入策略关系
							if(!UpdateDllParamter(strNewPath,strParamList))
							{
								strResultString = "ACK:Update Dll Fail(Insert Dll Params fail)";
							}
							bUpdateS3db = true;
						}

						if(nThreadSize>0)
						{
							wchar_t* charThreadList = new wchar_t[nThreadSize+1];
							GetPrivateProfileString(L"dll", L"threadList", L"", charThreadList, nThreadSize, strDCGPath);
							charThreadList[nThreadSize] = L'\0';
							strThreadList = charThreadList;
							delete[] charThreadList;

							//更新线程关系
							vector<wstring> vecThread;
							Project::Tools::SplitStringByChar(strThreadList.c_str(),L';',vecThread);
							for(int i=0; i<vecThread.size(); ++i)
							{
								wstring strThreadInfo = vecThread[i];
								vector<wstring> vecThreadInfo;
								Project::Tools::SplitStringByChar(strThreadInfo.c_str(),L',',vecThreadInfo);
								if(vecThreadInfo.size() == 3)		//只需修改数据库
								{
									if(!UpdateThreadState(strNewPath.GetString(),vecThreadInfo[0],vecThreadInfo[1],vecThreadInfo[2]))
									{
										strResultString = "ACK:Update Dll Fail(DUpdate Thread fail)";
									}

									if(_wtoi(vecThreadInfo[0].c_str()) == 1)
									{
										bUpdateS3db = true;
									}	
								}
							}
						}

						//删除配置文件
						CString strName = strDCGName.Left(strDCGName.ReverseFind('.'));
						CString strPriName = strDCGName.Right(strDCGName.GetLength() - strDCGName.ReverseFind('.') - 1);
						strDCGBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
						DelteFile(strDCGPath,strDCGBackPath);

						if(bUpdateS3db)
						{
							SYSTEMTIME sNow;
							GetLocalTime(&sNow);

							CString strTime;
							strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

							//备份当前使用s3db到备份文件夹
							if(FindOrCreateFolder(strBacFolder))
							{
								CString strNewBacPath;
								strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
								//先备份一个原s3db到更新文件夹
								if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
								{
									strResultString = "ACK:Update Dll Fail(CopyFile fail)";
									return false;
								}
							}

							//覆盖
							if(!DelteFile(strNewPath,strS3dbPath.c_str()))
							{
								strResultString = "ACK:Update Dll Fail(DelteFile fail)";
								return false;
							}

							//重启Core
							CString strFileFolder = strFilePath.Left(strFilePath.ReverseFind('\\'));
							bool bRestartCore = RestartExe(_T("domcore.exe"),strFileFolder);
							bool bRestartLogic = RestartExe(_T("BEOPLogicEngine.exe"),strFileFolder);

							if(!bRestartCore)
							{
								strResultString = "ACK:Update Dll Fail(Restart Core fail)";
								return false;
							}
						}
					}
				}
			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return true;
}

bool CBEOPDTUCtrl::UpdateDll( int nUpdateSate,CString strDllName,CString strThreadName,CString strS3dbPath )
{
	try
	{
		if(nUpdateSate == 0)		//增加或修改
		{
			if(strDllName.GetLength()<=0)
			{
				return false;
			}

			wstring strFolder;
			GetSysPath(strFolder);
			strFolder += L"\\fileupdate\\update";
			if(FindOrCreateFolder(strFolder))
			{
				if(FindFile(strFolder.c_str(),strDllName))
				{
					CString strdllPath;
					strdllPath.Format(_T("%s\\%s"),strFolder.c_str(),strDllName);
					HINSTANCE hDLL = NULL;
					hDLL = LoadLibrary(strdllPath);//加载动态链接库MyDll.dll文件；
					int nErr = GetLastError();
					if(hDLL==NULL)
					{
						return false;
					}
					pfnLogicBase pf_Dll = (pfnLogicBase)GetProcAddress(hDLL,"fnInitLogic");
					if(pf_Dll==NULL)
					{
						FreeLibrary(hDLL);
						return false;
					}

					CLogicBase *pLBDll = pf_Dll();
					CString strVersion,strDescription,strAuthor;
					if(pLBDll != NULL)
					{
						strVersion = pLBDll->GetDllLogicVersion();
						strDescription = pLBDll->GetDllDescription();
						strAuthor = pLBDll->GetDllAuthor();
					}

					if (pLBDll != NULL)
					{
						delete pLBDll;
						pLBDll = NULL;
					}
					if (hDLL != NULL)
					{
						FreeLibrary(hDLL);
						hDLL = NULL;
					}

					//读取内容
					return StoreDllToDB(strdllPath,strS3dbPath,strDllName,strThreadName,strVersion,strDescription,strAuthor);
				}
			}
		}
		else if(nUpdateSate == 1)						//删除
		{
			if(strDllName.GetLength()<=0)
			{
				return false;
			}

			return DeleteDll(strS3dbPath,strDllName,strThreadName);
		}
		else if(nUpdateSate == 2)						//更改策略线程
		{
			if(strDllName.GetLength()<=0)
			{
				return false;
			}

			return UpdateDllThread(strS3dbPath,strDllName,strThreadName);
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::UpdateS3db( DTU_UPDATE_INFO& data,string& strResultString )
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandler && m_pDataHandler->IsConnected())
			{
				strS3dbPath =  m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath",L"");
				if(strS3dbPath.length() <= 0)
				{
					strResultString = "ACK:Update S3db Fail(corefilepath is empty)";
					return false;
				}

				CString strFilePath = strS3dbPath.c_str();
				strFilePath = strFilePath.Right(strFilePath.GetLength()-strFilePath.ReverseFind('\\')-1);
				strS3dbName = strFilePath.Left(strFilePath.ReverseFind('.')).GetString();
				if(FindOrCreateFolder(strUpdateFolder))
				{
					vector<CString> vecFile;
					FindFile(strUpdateFolder.c_str(),_T("s3db"),vecFile);
					if(vecFile.size()>0)
					{
						//使用.csv更新备份s3db
						CString strUpdateS3dbPath,strUpdateS3dbBackPath;
						strUpdateS3dbPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);

						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						//备份当前使用s3db到备份文件夹
						if(FindOrCreateFolder(strBacFolder))
						{
							CString strNewBacPath;
							strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
							//先备份一个原s3db到备份文件夹
							if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
							{
								strResultString = "ACK:Update S3db Fail(CopyFile fail)";
								return false;
							}

							CString strLog;
							strLog.Format(_T("Backup before update s3db(%s)."),strNewBacPath);
							OutPutUpdateLog(strLog);
						}

						//覆盖
						if(DelteFile(strUpdateS3dbPath,strS3dbPath.c_str()))
						{
							CString strFileFolder = strFilePath.Left(strFilePath.ReverseFind('\\'));
							RestartExe(_T("domcore.exe"),strFileFolder);
							RestartExe(_T("BEOPLogicEngine.exe"),strFileFolder);							
						}
						else
						{
							strResultString = "ACK:Update S3db Fail(DelteFile fail)";
							return false;
						}
					}
					else
					{
						strResultString = "ACK:Update S3db Fail(File unexist)";
						return false;
					}
				}
			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return true;
}

bool CBEOPDTUCtrl::UpdateLogic( DTU_UPDATE_INFO& data ,string& strResultString)
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandler && m_pDataHandler->IsConnected())
			{
				strS3dbPath =  m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath",L"");
				if(strS3dbPath.length() <= 0)
				{
					strResultString = "ACK:Update Logic Fail(corefilepath is empty)";
					return false;
				}

				CString strFolderPath = strS3dbPath.c_str();
				strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//找到更新文件
					if(FindFile(strUpdateFolder.c_str(),_T("BEOPLogicEngine.exe")))
					{
						//拷贝更新文件到运行目录  以.bak结尾 并删除更新文件
						CString strUpdatePath;
						strUpdatePath.Format(_T("%s\\BEOPLogicEngine.exe"),strUpdateFolder.c_str());

						CString strCurrentBacPath;
						strCurrentBacPath.Format(_T("%s\\BEOPLogicEngine.exe.bak"),strFolderPath);
						if(!DelteFile(strUpdatePath,strCurrentBacPath))
						{
							strResultString = "ACK:Update Logic Fail(DelteFile fail)";
							return false;
						}

						//备份当前运行的Core文件到back文件夹
						CString strCurrentPath;
						strCurrentPath.Format(_T("%s\\BEOPLogicEngine.exe"),strFolderPath);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);
						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						if(FindOrCreateFolder(strBacFolder))
						{
							CString strBackPath;
							strBackPath.Format(_T("%s\\BEOPLogicEngine%s.exe"),strBacFolder.c_str(),strTime);

							//当前exe存在拷贝一份
							if(FindFile(strFolderPath,_T("BEOPLogicEngine.exe")))
							{
								if(!CopyFile(strCurrentPath,strBackPath,FALSE))
								{
									strResultString = "ACK:Update Logic Fail(CopyFile fail)";
									return false;
								}
							}
						}

						//关闭当前运行Logic
						CloseExe(_T("BEOPLogicEngine.exe"));

						//删除当前运行Core
						DelteFoder(strCurrentPath.GetString());

						//重命名.bak为
						int nReslut = rename(Project::Tools::WideCharToAnsi(strCurrentBacPath).c_str(),Project::Tools::WideCharToAnsi(strCurrentPath).c_str());
						if(nReslut != 0)
						{
							strResultString = "ACK:Update Logic Fail(rename fail)";
							return false;
						}

						//重启Core
						if(!OpenExe(strCurrentPath))
						{
							strResultString = "ACK:Update Logic Fail(Open Logic fail)";
							return false;
						}
					}
				}
			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return true;
}

bool CBEOPDTUCtrl::UpdateBat( DTU_UPDATE_INFO& data,string& strResultString )
{
	bool bResult = false;
	wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
	GetSysPath(strFolder);
	strFolder += L"\\fileupdate";
	if(FindOrCreateFolder(strFolder))
	{
		strUpdateFolder = strFolder + L"\\update";
		strBacFolder = strFolder + L"\\back";

		if(m_pDataHandler && m_pDataHandler->IsConnected())
		{
			strS3dbPath =  m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath",L"");
			if(strS3dbPath.length() <= 0)
			{
				strResultString = "ACK:Update Bat Fail(corefilepath is empty)";
				return false;
			}

			CString strFolderPath = strS3dbPath.c_str();
			strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
			if(FindOrCreateFolder(strUpdateFolder))
			{
				//再更新该备份s3db
				vector<CString> vecFile;
				FindFile(strUpdateFolder.c_str(),_T("bat"),vecFile);
				if(vecFile.size()>0)
				{
					//使用.csv更新备份s3db
					CString strBatPath,strBatBackPath;
					strBatPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);
					SHELLEXECUTEINFO ShExecInfo = {0};
					ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
					ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					ShExecInfo.hwnd = NULL;
					ShExecInfo.lpVerb = NULL;
					ShExecInfo.lpFile = strBatPath;	
					ShExecInfo.lpParameters = _T("");	
					ShExecInfo.lpDirectory = _T("");
					ShExecInfo.nShow = SW_HIDE;
					ShExecInfo.hInstApp = NULL;

					int nResult = ShellExecuteEx(&ShExecInfo);
					WaitForSingleObject(ShExecInfo.hProcess, INFINITE); 
					DWORD dwExitCode;
					bResult = GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);

					SYSTEMTIME sNow;
					GetLocalTime(&sNow);
					CString strTime;
					strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

					CString strBatName = vecFile[0];
					CString strName = strBatName.Left(strBatName.ReverseFind('.'));
					CString strPriName = strBatName.Right(strBatName.GetLength() - strBatName.ReverseFind('.') - 1);
					//删除改.csv
					strBatBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
					DelteFile(strBatPath,strBatBackPath);

					if(!bResult)
					{
						strResultString = "ACK:Update Bat Fail(Excute fail)";
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool CBEOPDTUCtrl::UpdatePointExcel( DTU_UPDATE_INFO& data,string& strResultString )
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandler && m_pDataHandler->IsConnected())
			{
				strS3dbPath =  m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath",L"");
				if(strS3dbPath.length() <= 0)
				{
					strResultString = "ACK:Update Point Excel Fail(corefilepath is empty)";
					return false;
				}

				CString strFilePath = strS3dbPath.c_str();
				strFilePath = strFilePath.Right(strFilePath.GetLength()-strFilePath.ReverseFind('\\')-1);
				strS3dbName = strFilePath.Left(strFilePath.ReverseFind('.')).GetString();
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//
					CString strNewPath;
					strNewPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strFilePath);
					//先备份一个原s3db到更新文件夹
					if(!CopyFile(strS3dbPath.c_str(),strNewPath,FALSE))
					{
						strResultString = "ACK:Update Point Excel Fail(CopyFile fail)";
						return false;
					}

					//再更新该备份s3db
					vector<CString> vecFile;
					FindFile(strUpdateFolder.c_str(),_T("xls"),vecFile);
					if(vecFile.size()>0)
					{
						//使用.xls更新备份s3db
						CString strExcelPath,strExcelBackPath;
						strExcelPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);

						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						int nErrCode = 0;
						bool bRestartCore = false;
						bool bRestartLogic = false;
						if(UpdatePointXls(strExcelPath,strNewPath,nErrCode))		//更新成功
						{
							//备份当前使用s3db到备份文件夹
							if(FindOrCreateFolder(strBacFolder))
							{
								CString strNewBacPath;
								strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
								//先备份一个原s3db到更新文件夹
								if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
								{
									strResultString = "ACK:Update Point Excel Fail(CopyFile fail)";
									return false;
								}
							}

							//覆盖
							if(!DelteFile(strNewPath,strS3dbPath.c_str()))
							{
								strResultString = "ACK:Update Point Excel Fail(DelteFile fail)";
								return false;
							}

							//重启Core
							CString strFileFolder = strFilePath.Left(strFilePath.ReverseFind('\\'));
							bRestartCore = RestartExe(_T("domcore.exe"),strFileFolder);
							bRestartLogic = RestartExe(_T("BEOPLogicEngine.exe"),strFileFolder);
						}
						else
						{
							strResultString = "ACK:Update Point Excel Fail(UpdatePointXls fail)";
							return false;
						}

						CString strExcelName = vecFile[0];
						CString strName = strExcelName.Left(strExcelName.ReverseFind('.'));
						CString strPriName = strExcelName.Right(strExcelName.GetLength() - strExcelName.ReverseFind('.') - 1);
						//删除改.xls
						strExcelBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
						DelteFile(strExcelPath,strExcelBackPath);

						if(!bRestartCore)
						{
							strResultString = "ACK:Update Point CSV Fail(Restart Core Fail)";
							return false;
						}
					}
					else
					{
						//查找.xlsx
						FindFile(strUpdateFolder.c_str(),_T("xlsx"),vecFile);
						if(vecFile.size()>0)
						{
							//使用.xlsx更新备份s3db
							CString strExcelPath,strExcelBackPath;
							strExcelPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);

							SYSTEMTIME sNow;
							GetLocalTime(&sNow);

							CString strTime;
							strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

							int nErrCode = 0;
							if(UpdatePointXls(strExcelPath,strNewPath,nErrCode))		//更新成功
							{
								//备份当前使用s3db到备份文件夹
								if(FindOrCreateFolder(strBacFolder))
								{
									CString strNewBacPath;
									strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
									//先备份一个原s3db到更新文件夹
									if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
									{
										strResultString = "ACK:Update Point Excel Fail(CopyFile fail)";
										return false;
									}
								}

								//覆盖
								if(!DelteFile(strNewPath,strS3dbPath.c_str()))
								{
									strResultString = "ACK:Update Point Excel Fail(DelteFile fail)";
									return false;
								}
							}
							else
							{
								strResultString = "ACK:Update Point Excel Fail(UpdatePointXls fail)";
								return false;
							}

							CString strExcelName = vecFile[0];
							CString strName = strExcelName.Left(strExcelName.ReverseFind('.'));
							CString strPriName = strExcelName.Right(strExcelName.GetLength() - strExcelName.ReverseFind('.') - 1);
							//删除改.xls
							strExcelBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
							DelteFile(strExcelPath,strExcelBackPath);
						}
						else
						{
							strResultString = "ACK:Update Point Excel Fail(File unexist)";
							return false;
						}
					}
				}
			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return true;
}

bool CBEOPDTUCtrl::UpdatePointCSV( DTU_UPDATE_INFO& data,string& strResultString )
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandler && m_pDataHandler->IsConnected())
			{
				strS3dbPath =  m_pDataHandler->ReadOrCreateCoreDebugItemValue_Defalut(L"corefilepath",L"");
				if(strS3dbPath.length() <= 0)
				{
					strResultString = "ACK:Update Point CSV Fail(corefilepath is empty)";
					return false;
				}

				CString strFilePath = strS3dbPath.c_str();
				strFilePath = strFilePath.Right(strFilePath.GetLength()-strFilePath.ReverseFind('\\')-1);
				strS3dbName = strFilePath.Left(strFilePath.ReverseFind('.')).GetString();
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//
					CString strNewPath;
					strNewPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strFilePath);
					//先备份一个原s3db到更新文件夹
					if(!CopyFile(strS3dbPath.c_str(),strNewPath,FALSE))
					{
						strResultString = "ACK:Update Point CSV Fail(CopyFile fail)";
						return false;
					}

					//再更新该备份s3db
					vector<CString> vecFile;
					FindFile(strUpdateFolder.c_str(),_T("csv"),vecFile);
					if(vecFile.size()>0)
					{
						//使用.csv更新备份s3db
						CString strExcelPath,strExcelBackPath;
						strExcelPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);

						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						int nErrCode = 0;
						bool bRestartCore = false;
						bool bRestartLogic = false;
						if(UpdatePointCSV(strExcelPath,strNewPath,nErrCode))		//更新成功
						{
							//备份当前使用s3db到备份文件夹
							if(FindOrCreateFolder(strBacFolder))
							{
								CString strNewBacPath;
								strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
								//先备份一个原s3db到更新文件夹
								if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
								{
									strResultString = "ACK:Update Point CSV Fail(CopyFile fail)";
									return false;
								}
							}

							//覆盖
							if(!DelteFile(strNewPath,strS3dbPath.c_str()))
							{
								strResultString = "ACK:Update Point CSV Fail(DelteFile fail)";
								return false;
							}

							//重启Core
							CString strFileFolder = strFilePath.Left(strFilePath.ReverseFind('\\'));
							bRestartCore = RestartExe(_T("domcore.exe"),strFileFolder);
							bRestartLogic = RestartExe(_T("BEOPLogicEngine.exe"),strFileFolder);
						}
						else
						{
							strResultString = "ACK:Update Point CSV Fail(UpdatePointCSV fail)";
							return false;
						}

						CString strExcelName = vecFile[0];
						CString strName = strExcelName.Left(strExcelName.ReverseFind('.'));
						CString strPriName = strExcelName.Right(strExcelName.GetLength() - strExcelName.ReverseFind('.') - 1);
						//删除改.csv
						strExcelBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
						DelteFile(strExcelPath,strExcelBackPath);

						if(!bRestartCore)
						{
							strResultString = "ACK:Update Point CSV Fail(Restart Core Fail)";
							return false;
						}
					}
					else
					{
						strResultString = "ACK:Update Point CSV Fail(File unexist)";
						return false;;
					}
				}
			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return true;
}

bool CBEOPDTUCtrl::UpdatePointCSV( CString strCSVPath,CString strS3dbPath,int& nErrCode )
{
	try
	{
		vector<DataPointEntry>	vecPoint;
		CExcelOperator excepOperator;
		if(excepOperator.ReadFromCSV_Comma(strCSVPath, vecPoint,nErrCode))
		{
			if (vecPoint.size()>0)
			{
				string strPath;
				Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
				CSqliteAcess access(strPath.c_str());
				bool bOK = false;
				int nNeedSave = 0;
				int nSaved = 0;
				access.BeginTransaction();
				access.DeleteAllPoint();
				int nCount = 0;
				CString	strPar16;
				CString	strPar17;
				CString	strPar18;
				for ( int i=0; i<vecPoint.size(); ++i )
				{	
					nNeedSave++;
					string strName;
					string strSourceType;
					string strParam1;
					string strParam2;
					string strParam3;
					string strParam4;
					string strParam5;
					string strParam6;
					string strParam7;
					string strParam8;
					string strParam9;
					string strParam10;
					string strParam11;
					string strParam12;
					string strParam13;
					string strParam14;
					string strParam15;
					string strDescription;
					string strUnit;
					const DataPointEntry& info = vecPoint[i];

					//name
					strName = Project::Tools::WideCharToAnsi(info.GetShortName().c_str());
					//SourceType
					strSourceType =  Project::Tools::WideCharToAnsi(info.GetSourceType().c_str());
					//param1 点位值
					strParam1 =  Project::Tools::WideCharToAnsi(info.GetParam(1).c_str());
					//param2 点位值
					strParam2 =  Project::Tools::WideCharToAnsi(info.GetParam(2).c_str());
					//param3 点位值
					strParam3 =  Project::Tools::WideCharToAnsi(info.GetParam(3).c_str());
					//param4 点位值
					strParam4 =  Project::Tools::WideCharToAnsi(info.GetParam(4).c_str());
					//param5 点位值
					strParam5 =  Project::Tools::WideCharToAnsi(info.GetParam(5).c_str());
					//param6 点位值
					strParam6 =  Project::Tools::WideCharToAnsi(info.GetParam(6).c_str());
					//param7 点位值
					strParam7 =  Project::Tools::WideCharToAnsi(info.GetParam(7).c_str());
					//param8 点位值
					strParam8 =  Project::Tools::WideCharToAnsi(info.GetParam(8).c_str());
					//param9 点位值
					strParam9 =  Project::Tools::WideCharToAnsi(info.GetParam(9).c_str());
					//param10 点位值
					strParam10 =  Project::Tools::WideCharToAnsi(info.GetParam(10).c_str());
					//param11 点位值
					strParam11 =  Project::Tools::WideCharToAnsi(info.GetParam(11).c_str());
					//param12 点位值
					strParam12 =  Project::Tools::WideCharToAnsi(info.GetParam(12).c_str());
					//param13 点位值
					strParam13 =  Project::Tools::WideCharToAnsi(info.GetParam(13).c_str());
					//param14 点位值
					strParam14 =  Project::Tools::WideCharToAnsi(info.GetParam(14).c_str());
					//param15 点位值
					strParam15 =  Project::Tools::WideCharToAnsi(info.GetParam(15).c_str());
					//param16 点位值
					strPar16.Format(_T("%s"), (info.GetParam(16)).c_str());
					int nParam16 = 0;
					for (int x=0; x<g_nLenPointAttrSystem; x++)
					{
						if (g_strPointAttrSystem[x] == strPar16)
						{
							nParam16 = x;
							break;
						}
					}
					//param17 点位值
					strPar17.Format(_T("%s"), (info.GetParam(17)).c_str());
					int nParam17 = 0;
					for (int y=0; y<g_nLenPointAttrDevice; y++)
					{
						if (g_strPointAttrDevice[y] == strPar17)
						{
							nParam17 = y;
							break;
						}
					}
					//param18 点位值
					strPar18.Format(_T("%s"), (info.GetParam(18)).c_str());
					int nParam18 = 0;
					for (int z=0; z<g_nLenPointAttrType; z++)
					{
						if (g_strPointAttrType[z] == strPar18)
						{
							nParam18 = z;
							break;
						}
					}

					//remark
					strDescription =  Project::Tools::WideCharToAnsi(info.GetDescription().c_str());
					//单位 unit
					strUnit =  Project::Tools::WideCharToAnsi(info.GetUnit().c_str());
					char szCycle[10] = {0};
					sprintf_s(szCycle,sizeof(szCycle),"%d",(int)info.GetStoreCycle());
					assert(strlen(szCycle)>0);
					int nIndex = 0;
					if (0 == info.GetPointIndex())
					{
						nIndex = i;
					}
					else
					{
						nIndex = info.GetPointIndex();
					}
					if(access.InsertRecordToOPCPoint(nIndex,0,strName.c_str(),strSourceType.c_str(),info.GetProperty(),strDescription.c_str(),strUnit.c_str(),
						info.GetHighAlarm(),info.GetHighHighAlarm(),info.GetLowAlarm(),info.GetLowLowAlarm(),strParam1.c_str(),
						strParam2.c_str(),strParam3.c_str(),strParam4.c_str(),strParam5.c_str(),strParam6.c_str(),strParam7.c_str(),
						strParam8.c_str(),strParam9.c_str(),strParam10.c_str(),szCycle,strParam12.c_str(),strParam13.c_str(),strParam14.c_str(),
						strParam15.c_str(),nParam16,nParam17,nParam18) == 0)
					{
						nSaved++;
					}
					else
					{
						CString strLog;
						strLog.Format(_T("Insert s3db(%d) fails."),nIndex);
						OutPutUpdateLog(strLog);
						nErrCode = 31003;
					}
				}

				if (nNeedSave == nSaved)
				{
					bOK = true;
				}

				if(bOK)
				{
					access.CommitTransaction();
				}
				else
				{
					access.RollbackTransaction();
				}
				return bOK;
			}
		}
		else
		{
			CString strLog;
			strLog.Format(_T("Read csv(%s) fails."),strCSVPath);
			OutPutUpdateLog(strLog);
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::UpdatePointXls( CString strExcelPath,CString strS3dbPath,int& nErrCode )
{
	vector<DataPointEntry>	vecPoint;
	CExcelOperator excepOperator;
	if(excepOperator.ReadFromExcel(strExcelPath, vecPoint,nErrCode))
	{
		if (vecPoint.size()>0)
		{
			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
			CSqliteAcess access(strPath.c_str());
			bool bOK = false;
			int nNeedSave = 0;
			int nSaved = 0;
			access.BeginTransaction();
			access.DeleteAllPoint();
			int nCount = 0;
			CString	strPar16;
			CString	strPar17;
			CString	strPar18;
			for ( int i=0; i<vecPoint.size(); ++i )
			{	
				nNeedSave++;
				string strName;
				string strSourceType;
				string strParam1;
				string strParam2;
				string strParam3;
				string strParam4;
				string strParam5;
				string strParam6;
				string strParam7;
				string strParam8;
				string strParam9;
				string strParam10;
				string strParam11;
				string strParam12;
				string strParam13;
				string strParam14;
				string strParam15;
				string strDescription;
				string strUnit;
				const DataPointEntry& info = vecPoint[i];

				//name
				strName = Project::Tools::WideCharToAnsi(info.GetShortName().c_str());
				//SourceType
				strSourceType =  Project::Tools::WideCharToAnsi(info.GetSourceType().c_str());
				//param1 点位值
				strParam1 =  Project::Tools::WideCharToAnsi(info.GetParam(1).c_str());
				//param2 点位值
				strParam2 =  Project::Tools::WideCharToAnsi(info.GetParam(2).c_str());
				//param3 点位值
				strParam3 =  Project::Tools::WideCharToAnsi(info.GetParam(3).c_str());
				//param4 点位值
				strParam4 =  Project::Tools::WideCharToAnsi(info.GetParam(4).c_str());
				//param5 点位值
				strParam5 =  Project::Tools::WideCharToAnsi(info.GetParam(5).c_str());
				//param6 点位值
				strParam6 =  Project::Tools::WideCharToAnsi(info.GetParam(6).c_str());
				//param7 点位值
				strParam7 =  Project::Tools::WideCharToAnsi(info.GetParam(7).c_str());
				//param8 点位值
				strParam8 =  Project::Tools::WideCharToAnsi(info.GetParam(8).c_str());
				//param9 点位值
				strParam9 =  Project::Tools::WideCharToAnsi(info.GetParam(9).c_str());
				//param10 点位值
				strParam10 =  Project::Tools::WideCharToAnsi(info.GetParam(10).c_str());
				//param11 点位值
				strParam11 =  Project::Tools::WideCharToAnsi(info.GetParam(11).c_str());
				//param12 点位值
				strParam12 =  Project::Tools::WideCharToAnsi(info.GetParam(12).c_str());
				//param13 点位值
				strParam13 =  Project::Tools::WideCharToAnsi(info.GetParam(13).c_str());
				//param14 点位值
				strParam14 =  Project::Tools::WideCharToAnsi(info.GetParam(14).c_str());
				//param15 点位值
				strParam15 =  Project::Tools::WideCharToAnsi(info.GetParam(15).c_str());
				//param16 点位值
				strPar16.Format(_T("%s"), (info.GetParam(16)).c_str());
				int nParam16 = 0;
				for (int x=0; x<g_nLenPointAttrSystem; x++)
				{
					if (g_strPointAttrSystem[x] == strPar16)
					{
						nParam16 = x;
						break;
					}
				}
				//param17 点位值
				strPar17.Format(_T("%s"), (info.GetParam(17)).c_str());
				int nParam17 = 0;
				for (int y=0; y<g_nLenPointAttrDevice; y++)
				{
					if (g_strPointAttrDevice[y] == strPar17)
					{
						nParam17 = y;
						break;
					}
				}
				//param18 点位值
				strPar18.Format(_T("%s"), (info.GetParam(18)).c_str());
				int nParam18 = 0;
				for (int z=0; z<g_nLenPointAttrType; z++)
				{
					if (g_strPointAttrType[z] == strPar18)
					{
						nParam18 = z;
						break;
					}
				}

				//remark
				strDescription =  Project::Tools::WideCharToAnsi(info.GetDescription().c_str());
				//单位 unit
				strUnit =  Project::Tools::WideCharToAnsi(info.GetUnit().c_str());
				char szCycle[10] = {0};
				sprintf_s(szCycle,sizeof(szCycle),"%d",(int)info.GetStoreCycle());
				assert(strlen(szCycle)>0);
				int nIndex = 0;
				if (0 == info.GetPointIndex())
				{
					nIndex = i;
				}
				else
				{
					nIndex = info.GetPointIndex();
				}
				if(access.InsertRecordToOPCPoint(nIndex,0,strName.c_str(),strSourceType.c_str(),info.GetProperty(),strDescription.c_str(),strUnit.c_str(),
					info.GetHighAlarm(),info.GetHighHighAlarm(),info.GetLowAlarm(),info.GetLowLowAlarm(),strParam1.c_str(),
					strParam2.c_str(),strParam3.c_str(),strParam4.c_str(),strParam5.c_str(),strParam6.c_str(),strParam7.c_str(),
					strParam8.c_str(),strParam9.c_str(),strParam10.c_str(),szCycle,strParam12.c_str(),strParam13.c_str(),strParam14.c_str(),
					strParam15.c_str(),nParam16,nParam17,nParam18) == 0)
				{
					nSaved++;
				}
				else
				{
					CString strLog;
					strLog.Format(_T("Insert xls(%d) fails."),nIndex);
					OutPutUpdateLog(strLog);
					nErrCode = 31003;
				}
			}

			if (nNeedSave == nSaved)
			{
				bOK = true;
			}

			if(bOK)
			{
				access.CommitTransaction();
			}
			else
			{
				access.RollbackTransaction();
			}
			return bOK;
		}
	}
	else
	{
		CString strLog;
		strLog.Format(_T("Read xls(%s) fails."),strExcelPath);
		OutPutUpdateLog(strLog);
	}
	return false;
}

bool CBEOPDTUCtrl::FindFile( CString strFolder,CString strExten,vector<CString>& vecFileName )
{
	vecFileName.clear();
	CFileFind tempFind;
	CString strFind;
	strFind.Format(_T("%s\\*.%s"),strFolder,strExten);
	BOOL IsFinded = tempFind.FindFile(strFind);
	CString strBackupPath;
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();
		if (!tempFind.IsDots()) 
		{
			if (!tempFind.IsDirectory())
			{
				CString strFileName = tempFind.GetFileName();
				vecFileName.push_back(strFileName);
			}
		}
	}
	tempFind.Close();
	return true;
}

bool CBEOPDTUCtrl::FindFile( CString strFolder,CString strName )
{
	CFileFind tempFind;
	CString strFind;
	strFind.Format(_T("%s\\*.*"),strFolder);
	BOOL IsFinded = tempFind.FindFile(strFind);
	CString strBackupPath;
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();
		if (!tempFind.IsDots()) 
		{
			if (!tempFind.IsDirectory())
			{
				CString strFileName = tempFind.GetFileName();
				if(strFileName.Find(strName)>=0)
				{
					tempFind.Close();
					return true;
				}					
			}
		}
	}
	tempFind.Close();
	return false;
}

bool CBEOPDTUCtrl::StoreDllToDB( CString strDllPath,CString strS3dbPath,CString strDllName,CString strThreadName,CString strVersion,CString strDescription,CString strAuthor )
{
	try
	{
		FILE *fp;
		long filesize = 0;
		char * ffile;
		_bstr_t tem_des = strDllPath;

		char* des = (char*)tem_des;
		fopen_s(&fp,des, "rb");
		if(fp != NULL)
		{
			//计算文件的大小
			fseek(fp, 0, SEEK_END);
			filesize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			//读取文件
			ffile = new char[filesize+1];
			size_t sz = fread(ffile, sizeof(char), filesize, fp);
			fclose(fp);
		}

		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			char szSQL[1024] = {0};
			memset(szSQL, 0, sizeof(szSQL));
			int rc = 0;
			int nID = 1;

			SYSTEMTIME sysTime;
			CString importtime;
			::GetLocalTime(&sysTime);
			importtime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), sysTime.wYear, sysTime.wMonth,sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

			_bstr_t tem_des = importtime;
			char* des = (char*)tem_des;
			std::string importtime__(des);

			tem_des = strDllName;
			des = (char*)tem_des;
			std::string dllname__(des);

			tem_des = strAuthor;
			des = (char*)tem_des;
			std::string author__(des);

			//dll版本
			tem_des = strVersion;
			des = (char*)tem_des;
			std::string version__(des);

			//dll描述
			tem_des = strDescription;
			des = (char*)tem_des;
			std::string description__(des);

			//////////////////////////////////////////////////////////////////////////
			//查找是否存在
			ostringstream sqlstream;
			sqlstream << "select * from list_dllstore;";
			string sql_ = sqlstream.str();
			char *ex_sql = const_cast<char*>(sql_.c_str());
			int re = access.SqlQuery(ex_sql);

			//
			int timespan = 20;
			while(true)
			{
				if(SQLITE_ROW != access.SqlNext())
				{
					break;
				}
				timespan = access.getColumn_Int(4);
			}
			access.SqlFinalize();

			//
			tem_des = strThreadName;
			des = (char*)tem_des;
			std::string threadname__(des);

			//删除已有的同名dll
			string sql_del;
			ostringstream sqlstream_del;
			sqlstream_del << "delete from list_dllstore where DllName = "<< "'" << dllname__ << "';"; 
			sql_del = sqlstream_del.str();
			if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
			{
				;
			}
			//////////////////////////////////////////////////////////////////////////
			int runstatus = 1;
			sqlstream.str("");
			sqlstream << "insert into list_dllstore(id,DllName,importtime,author,periodicity,dllcontent,runstatus,unitproperty01,unitproperty02,unitproperty03,unitproperty04,unitproperty05)  values ('" << nID<< "','" << dllname__ << "','" \
				<< importtime__ << "','" << author__ << "','" << timespan << "',?,'1','"<<version__ <<"','"<<description__<<"','"<<dllname__<<"','"<<threadname__<<"','');";
			sql_ = sqlstream.str();

			bool bInsertOK = false;
			if (SQLITE_OK == access.SqlQuery(sql_.c_str()))
			{
				if (access.m_stmt)
				{
					rc = access.SqlBindBlob(ffile,1,filesize);
					rc = access.SqlNext();
					rc = access.SqlFinalize();
				}
			}
			access.CloseDataBase();
		}

		if (ffile != NULL)
		{
			delete [] ffile;
			ffile = NULL;
		}
		return true;
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::DeleteDll( CString strS3dbPath,CString strDllName,CString strThreadName )
{
	try
	{
		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			_bstr_t tem_des = strDllName;
			char* des = (char*)tem_des;
			std::string dllname__(des);

			tem_des = strThreadName;
			des = (char*)tem_des;
			std::string threadname__(des);

			//删除已有的同名dll
			string sql_del;
			ostringstream sqlstream_del;
			sqlstream_del << "delete from list_dllstore where DllName = "<< "'" << dllname__ << "' and unitproperty04='" << threadname__ << "';"; 
			sql_del = sqlstream_del.str();
			if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
			{
				;
			}

			//删除dll策略关系
			sqlstream_del.str("");
			sqlstream_del << "delete from list_paramterConfig where DllName = "<< "'" << dllname__ << "';"; 
			sql_del = sqlstream_del.str();
			if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
			{
				;
			}

			access.CloseDataBase();
		}
		return true;
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::UpdateDllThread( CString strS3dbPath,CString strDllName,CString strThreadName )
{
	try
	{
		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			_bstr_t tem_des = strDllName;
			char* des = (char*)tem_des;
			std::string dllname__(des);

			tem_des = strThreadName;
			des = (char*)tem_des;
			std::string threadname__(des);

			//删除已有的同名dll
			string sql_del;
			ostringstream sqlstream_del;
			sqlstream_del << "update list_dllstore set unitproperty04 ='" << threadname__ << "' where DllName = '" << dllname__ << "';"; 
			sql_del = sqlstream_del.str();
			if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
			{
				;
			}
			access.CloseDataBase();
			return true;
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::UpdateDllParamter( CString strS3dbPath,wstring strParamSql )
{
	try
	{
		if(strParamSql.length() <= 0)
			return false;

		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			string sql = "";
			sql = Project::Tools::WideCharToAnsi(strParamSql.c_str());
			vector<string> vecParamSql;
			SplitStringSpecial(sql.c_str(),sql.length(),vecParamSql);
			int nResult = 0;
			if(vecParamSql.size()>0)
			{
				access.BeginTransaction();
				for(int i=0; i<vecParamSql.size(); ++i)
				{
					string sql_insert;
					ostringstream sqlstream_insert;
					sqlstream_insert << "insert into list_paramterConfig values " << vecParamSql[i] << ");";
					sql_insert = sqlstream_insert.str();
					nResult = access.SqlExe(sql_insert.c_str());
				}
				access.CommitTransaction();
			}
			access.CloseDataBase();
			if(nResult == 0)
				return true;
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::DeleteDllParamter( CString strS3dbPath,wstring strParamSql )
{
	try
	{
		if(strParamSql.length() <= 0)
			return false;

		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			string sql = "";
			sql = Project::Tools::WideCharToAnsi(strParamSql.c_str());
			int nResult = access.SqlExe(sql.c_str());
			access.CloseDataBase();
			if(nResult == 0)
				return true;
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::UpdateThreadState( wstring strS3dbPath,wstring strThreadName,wstring strState,int nPeriod/*=-1*/ )
{
	try
	{
		if(strThreadName.length() <= 0)
			return false;
		//修改数据库
		m_pDataHandler->WriteCoreDebugItemValue(strThreadName,strState);

		if(nPeriod != -1)		//只需修改数据库
		{
			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath,strPath);
			CSqliteAcess access(strPath.c_str());
			if (access.GetOpenState())
			{
				_bstr_t tem_des = strThreadName.c_str();
				char* des = (char*)tem_des;
				std::string threadname(des);

				ostringstream sqlstream_del;
				sqlstream_del << "update list_dllstore set periodicity ="<< nPeriod <<" where unitproperty04 = "<< "'" << threadname << "';"; 
				string sql_del = sqlstream_del.str();
				access.SqlExe(sql_del.c_str());
				access.CloseDataBase();
			}
		}
		return true;
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::UpdateThreadState( wstring strS3dbPath,wstring strState,wstring strThreadName,wstring strValue )
{
	try
	{
		if(strThreadName.length() <= 0)
			return false;

		if(_wtoi(strState.c_str()) == 0)
		{
			//修改数据库
			m_pDataHandler->WriteCoreDebugItemValue(strThreadName,strValue);
		}
		else
		{
			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath,strPath);
			CSqliteAcess access(strPath.c_str());
			if (access.GetOpenState())
			{
				_bstr_t tem_des = strThreadName.c_str();
				char* des = (char*)tem_des;
				std::string threadname(des);

				ostringstream sqlstream_del;
				sqlstream_del << "update list_dllstore set periodicity ="<< _wtoi(strValue.c_str()) <<" where unitproperty04 = "<< "'" << threadname << "';"; 
				string sql_del = sqlstream_del.str();
				access.SqlExe(sql_del.c_str());
				access.CloseDataBase();
			}
		}
		return true;
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CBEOPDTUCtrl::RestartExe( CString strExeName,CString strFolder )
{
	CString strFilePath;
	strFilePath.Format(_T("%s/%s"),strFolder,strExeName);
	if (Project::Tools::FindFileExist(strFilePath.GetString()))
	{
		//关闭现有的
		if (m_PrsView.FindProcessByName_other(strExeName))
		{
			m_PrsView.CloseApp(strExeName);
			Sleep(1000);
		}

		if (!m_PrsView.FindProcessByName_other(strExeName))
		{
			CString strFilePath;
			strFilePath.Format(_T("%s/%s"),strFolder,strExeName);
			if(m_PrsView.OpenApp(strFilePath))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}

void CBEOPDTUCtrl::SplitStringSpecial( const char* buffer,int nSize, std::vector<string>& resultlist )
{
	try
	{
		if(nSize <= 0)
			return;

		resultlist.clear();
		char* cSpiltBufferSpilt = new char[nSize+1];
		memcpy(cSpiltBufferSpilt,buffer,nSize);

		char cBefore = 0x00;
		int nBefore = 0;
		int nAfter = 0;
		for(int i=0; i<nSize; ++i)
		{
			if(buffer[i] == ',' && cBefore == ')')
			{
				char* cBuffer = new char[1024];
				memset(cBuffer,0,1024);
				int length = i-1-nBefore;
				length = (length>1024)?1024:length;
				memcpy(cBuffer,cSpiltBufferSpilt+nBefore,length);		
				resultlist.push_back(cBuffer);
				nBefore = i+1;
			}
			cBefore = buffer[i];
		}

		int nLength = nSize - nBefore;
		if(nLength>0)
		{
			char* cBuffer = new char[1024];
			memset(cBuffer,0,1024);
			int length = (nLength>1024)?1024:nLength;
			memcpy(cBuffer,cSpiltBufferSpilt+nBefore,length);
			resultlist.push_back(cBuffer);
		}

		delete []cSpiltBufferSpilt;
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
}
