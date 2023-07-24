#include <string>
#include <afxinet.h>
#include <hash_map>
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
using namespace std;

/*
点表：
    点地址，订阅号，IP/端口，倍率，用户名，密码

订阅结构：
    点地址，订阅号，IP/端口，用户名，密码，实际订阅号，实际消息号，失效时间

*/

/*		待修改
	2：变化点更新模式 需读取一次全部数据
	5：返回参数类型可选吗？（太多）
*/

struct SubscriptionInfo
{
	SubscriptionInfo()
	{
		strSubscriptionID = "";
		strSubscriptionID_Active = "";
		strNotificationID_Active = "";
		strNotificationIDAll_Active = "";
		strExpireTime = "";
		bActive = false;
		bUpdateAll = false;
		mapSubPoint.clear();
		mapValue.clear();
	}

	string strSubscriptionID;				//虚拟订阅ID
	string strSubscriptionID_Active;		//实际订阅ID
	string strNotificationID_Active;		//实际消息ID
	string strNotificationIDAll_Active;		//实际消息全部ID
	string strExpireTime;					//失效时间
	bool   bActive;							//活动状态
	bool   bUpdateAll;						//至少获取一次全部数据
	hash_map<string,int>	mapSubPointAddress;	//订阅点(点地址，点值)
	hash_map<string,string>	mapValue;		//订阅点(点地址，点值)
	hash_map<wstring,DataPointEntry*> mapSubPoint;	//订阅点(点名，点名)
};

class __declspec(dllexport) CWebHttpCtrl
{
public:
	CWebHttpCtrl(Beopdatalink::CCommonDBAccess*	dbsession_history,int nDurationInMinutes=30,int nUpdateInterval = 60,int nActiveMinutes=30,string strSubscriptionType="ValueItemChanged",bool bChangesOnly = false,bool bDeleteSubcription=false,wstring strSubcriptions= L"");
	virtual ~CWebHttpCtrl(void);

	bool	CloseConnection();												//关闭连接
	bool	CreateDefaultConnection();										//创建Http Post连接
	bool	CreateConnection(CString strHost, INTERNET_PORT nPort);			//创建Http Post连接

	bool	Init();															//初始化
	bool	Exit();															//退出
	bool	SetConnection(CString strHost, INTERNET_PORT nPort);			//设置Http Post连接
	void	SetPointList(const vector<DataPointEntry>& pointlist);			//设置点表
	void	SetIndex(int nIndex);											//设置引擎号
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );		//获取点值
	
	void	SumReadAndResponse();											//每次发送前统计上一次的收发情况
	//////////////////////////////////////////////////////////////////////////
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);				//查询
	bool	GenerateSubcription();											//生成订阅结构体
	bool	SendSuccriptions();												//查询所有订阅
	bool	SaveSuccriptions();												//存储所有订阅
	bool	SendOneSuccription(SubscriptionInfo& subcription);				//查询单个订阅
	bool	CreateOneSuccription(SubscriptionInfo& subcription);			//创建一个订阅
	bool	DeleteOneSuccription(SubscriptionInfo& subcription);			//删除一个订阅
	bool	DeleteOneSuccription(string strSubcription);					//删除一个订阅
	bool	GetAllSuccription(vector<string>& vecSuccription);				//获取所有订阅
	bool	DeletAllSuccription();											//删除所有订阅
	bool	ActiveOneSuccription(SubscriptionInfo& subcription);			//激活一个订阅
	bool	CreateOneNotification(SubscriptionInfo& subcription);			//创建一个消息
	bool	CreateOneNotificationAll(SubscriptionInfo& subcription);		//创建一个消息(全部数据)
	bool	DeleteOneNotificationAll(SubscriptionInfo& subcription);		//删除一个消息(全部数据)
	bool	DeleteOneNotification(SubscriptionInfo& subcription);			//删除一个消息
	bool	PollOneSuccription(SubscriptionInfo& subcription);				//轮询一个订阅
	bool	GetToken();														//获取授权
	//////////////////////////////////////////////////////////////////////////
	bool	JsonParseToken(string strJson);									//解析Token
	bool	JsonParseCreateSuccription(string strJson,SubscriptionInfo& subcription);		//解析创建订阅
	bool	JsonParseActiveSuccription(string strJson,SubscriptionInfo& subcription);		//解析激活订阅
	bool	JsonParseCreateNotification(string strJson,SubscriptionInfo& subcription);		//解析创建消息
	bool	JsonParseCreateNotificationAll(string strJson,SubscriptionInfo& subcription);	//解析创建消息
	bool	JsonParsePollSuccription(string strJson,SubscriptionInfo& subcription,bool bNotificationAll =false);			//解析轮询消息
	bool	JsonParseAllSuccription(string strJson,vector<string>& vecSuccription);			//解析获取全部订阅
	//////////////////////////////////////////////////////////////////////////

	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
	bool	OutPutWebServerInfo(SubscriptionInfo& subcription);				//输出订阅信息
	bool	OutPutLogString(wstring strOut);
public:
	CInternetSession					m_Isession;
	CHttpConnection*					m_pConnection;
	CString								m_strHost;							//Http IP
	INTERNET_PORT						m_nPort;							//Http Port
	CString								m_strUser;							//Http User
	CString								m_strPsw;							//Http Psw
	vector<DataPointEntry>				m_pointlist;						//点表.
	hash_map<string,SubscriptionInfo>	m_mapSubcription;					//订阅缓存  订阅号+订阅
	bool								m_bExitThread;						//退出线程
	int									m_nUpdateInterval;					//查询间隔
	Project::Tools::Mutex				m_mtuex_http;						//HTTP接口互斥锁
	int									m_nTimeOutCount;					//连续超时次数
	int									m_nInvalidResponseCount;			//连续无效回应次数
	string								m_strToken;							//授权
	int									m_nDurationInMinutes;				//订阅参数时间
	string								m_strSubscriptionType;				//订阅参数类型
	int									m_nActiveMinutes;					//激活订阅时间
	bool								m_bChangesOnly;						//返回变化值  If true, all items will be returned. If false, only changes 
	COleDateTime						m_oleUpdateTime;					//更新时间
	int									m_nEngineIndex;						//引擎编号
	HANDLE								m_hSendCmd;							//查询引擎
	bool								m_bDeleteSubcription;				//删除指定订阅号
	wstring								m_strSubcriptions;					//订阅号
	bool								m_bCreateSubcription;				//创建订阅成功后 将订阅号存到数据库中
	Beopdatalink::CCommonDBAccess*		m_dbsession_history;				//数据库连接
	int									m_nCmdCount;						//命令个数
	int									m_nResponseCount;					//请求回复个数
	int									m_nUpdatePointCount;				//更新点
	string								m_strUpdateLog;						//更新log
};