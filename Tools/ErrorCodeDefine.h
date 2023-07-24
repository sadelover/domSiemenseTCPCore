#pragma once


#define ERROR_CODE_SUCCESS						0			//无错误

#define ERROR_MAX_WINDOW						16000		//windows 错误

#define ERROR_CUSTOM_BEGIN						30000		//自定义错误

//30001-30999		Core的一般错误
#define ERROR_CUSTOM_MFC_INIT_FAIL				30001		//MFC Init Failed
#define ERROR_CUSTOM_GET_MODULE_FAIL			30002		//GetModuleHandle 失败
#define ERROR_CUSTOM_ANOTHER_RUN				30003		//Another BEOPGatewayCore is running, Start failed.
#define ERROR_CUSTOM_INIT_FAIL					30004		//Init Failed, Try again after 30 seconds automatic

//30100
#define ERROR_CUSTOM_UPDATE_INPUT				30100		//UpdateInputTable Failed
#define ERROR_CUSTOM_UPDATE_OUTPUT				30101		//Update Output Values Failed
#define ERROR_CUSTOM_INIT_INPUT					30102		//Init Realtime DB Input Datas Failed, Please Check Buffer Size of MySQL
#define ERROR_CUSTOM_READ_INPUT					30103		//Read Point Buffer Data Failed
#define ERROR_CUSTOM_INSERT_INPUT				30104		//Insert Realtime Datas Failed
#define ERROR_CUSTOM_READ_OUTPUT				30106		//ReadRealTimeData_Output Failed!
#define ERROR_CUSTOM_INSERT_HISTORY				30107		//update history data(%s) row: %d , %s

//31000	点表功能错误代码（31000-31999）
#define ERROR_CUSTOM_INIT_POINT					31000		//Reading Points Failed
#define ERROR_CUSTOM_POINT_INDEX				31001		//序号%s，\r\n点%s，\r\n参数中包含单引号，导入中止！
#define ERROR_CUSTOM_OPEN_POINT					31002		//打开点表失败
#define ERROR_CUSTOM_INSERT_EXCEL				31003		//插入xls失败
#define ERROR_CUSTOM_DELETE_S3DB				30004		//Delete Current DB Files Failed!
#define ERROR_CUSTOM_DELETE_UNNEED_S3DB			30005		//Delete UnNeccesary DB Files Failed
#define ERROR_CUSTOM_UNEXIST_S3DB				30006		//None DB File Found
#define ERROR_CUSTOM_UPDATE_POINT				30007		//Updating Online Points Failed
#define ERROR_CUSTOM_OPEN_S3DB					30008		//打开S3DB数据库失败
#define ERROR_CUSTOM_READ_S3DB_CONFIG			30009		//读取数据库输出参数失败
#define ERROR_CUSTOM_MODBUS_POINT				30010		//Modbus点 : %s不合法

//32000 策略
#define ERROR_CUSTOM_UNLOAD_DLL					32000		//Unloading Threads: Failed
#define ERROR_CUSTOM_UNLOAD_LOGIC_MANAGER		32001		//Unloading Threads: Failed
#define ERROR_CUSTOM_EXIT_LOGIC_MANAGER			32002		//Exit
#define ERROR_CUSTOM_EXIT_DATAENGINECORE		32003		//Exit
#define ERROR_CUSTOM_EXIT_DATABASE				32004		//Exit Database
#define ERROR_CUSTOM_UPDATE_DLL					32005		//Logic File Download and Update Failed
#define ERROR_CUSTOM_SAVE_MEM_CREATE			32006		//SaveMemToFile CreateFolder failed
#define ERROR_CUSTOM_SAVE_MEM_DELETE			32007		//SaveMemToFile DeleteFile failed
#define ERROR_CUSTOM_SAVE_MEM_OPEN				32008		//SaveMemToFile _wfopen_s failed
#define ERROR_CUSTOM_EDIT_DLL					32009		//数据库策略修改失败
#define ERROR_CUSTOM_LOAD_DLL					32010		//Logic Loadlibrary return NULL
#define ERROR_CUSTOM_INIT_DLL					32011		//Cant find InitLogic entrance
#define ERROR_CUSTOM_DLL_VERSION				32012		//Logic Version Overdue
#define ERROR_CUSTOM_GET_DLL_OUT				32013		// Logic %s Get Output Parameters Value Failed
#define ERROR_CUSTOM_GET_DLL_INPUT				32014		// Logic %s Get Input Parameters Value Failed
#define ERROR_CUSTOM_CHRAD_THREAD				32015		//_beginthreadex failed( Threadname:%s,errCode:%d)
#define ERROR_CUSTOM_PARAM_UNEXIST				32016		//Logic Output Permit Point Not Exist

//33000 数据库
#define ERROR_CUSTOM_CONNECT_DATABASE			33000		//database connect failed.
#define ERROR_CUSTOM_READ_DB_CONFIG				33001		//DB Read Failed, Please Check the DB File.
#define ERROR_CUSTOM_CHECK_DB					33002		//Check MySQL DB Failed, Please Check the MySQL installation.
#define ERROR_CUSTOM_INIT_DB					33003		//InitDBConnection Failed
#define ERROR_CUSTOM_CONNECT_DB					33004		//Database connect failed
#define ERROR_CUSTOM_CONNECT_REDUNDENCY_DB		33005		//Redundency Database connect failed

#define ERROR_CUSTOM_INSERT_WARNING				33006		//insert into warning_record values
#define ERROR_CUSTOM_INSERT_OPERATION			33007		//insert into operation_record values
#define ERROR_CUSTOM_INSERT_RINPUT				33008		//insert realtimedata_input err
#define ERROR_CUSTOM_UPDATE_RINPUT				33009		//update realtimedata_output err
#define ERROR_CUSTOM_UPDATE_ROUTPUT				33010		//update realtimedata_output err
#define ERROR_CUSTOM_INSERT_UNIT01				33011		//insert unit01 err
#define ERROR_CUSTOM_UPDATE_UNIT01				33012		//update unit01 err
#define ERROR_CUSTOM_INSERT_UNIT02				33013		//insert unit02 err
#define ERROR_CUSTOM_INSERT_POINT_BUFFER		33014		//insert point_value_buffer err
#define ERROR_CUSTOM_UPDATE_POINT_BUFFER		33015		//update point_value_buffer err
#define ERROR_CUSTOM_INSERT_LOG					33016		//insert log err
#define ERROR_CUSTOM_INSERT_HISTORY1			33017		//insert history err
#define ERROR_CUSTOM_INSERT_WARNING_RECORD		33018		//insert warningrecord err
#define ERROR_CUSTOM_UPDATE_WARNING_RECORD		33019		//insert warningrecord err
#define ERROR_CUSTOM_INSERT_OPERATION_RECORD	33020		//insert operationrecord
#define ERROR_CUSTOM_INSERT_UNIT04				33021		//insert unit04 err

#define ERROR_CUSTOM_CREATE_TB					33030		//create table err

//34000	DTU
#define ERROR_CUSTOM_TCP_NAME					34000		//TCPSender Name not valid.
#define ERROR_CUSTOM_ACK_CHANGE_VALUE			34001		//Ack DTUServer Change Values Cmd
#define ERROR_CUSTOM_ACK_CHANGE_DATA			34004		//Ack DTUServer Change Data Cmd
#define ERROR_CUSTOM_ACK_LOST_DATA				34005		//Ack DTUServer ReSend Lost Data Cmd
#define ERROR_CUSTOM_ACK_RESTART				34006		// Ack DTUServer Reatart Cmd
#define ERROR_CUSTOM_ACK_RESTART_WATCH			34007		//Ack DTUServer Reatart Watch Cmd
#define ERROR_CUSTOM_ACK_SQL					34008		//Ack DTUServer Execute Sql Cmd
#define ERROR_CUSTOM_ACK_HEARTBEAT				34009		//Ack DTUServer HeartBeat Cmd
#define ERROR_CUSTOM_ACK_RESTART_DTU			34010		//Ack DTUServer ReStart DTU Cmd Success
#define ERROR_CUSTOM_ACK_VERSION				34011		//Ack DTUServer Query Version Cmd
#define ERROR_CUSTOM_INIT_DTU					34012		//It does not support running simultaneously DTU and TCPSender
#define ERROR_CUSTOM_TCP_NAME_NULL				34013		//Init TCPSender Engine Fail(TCPSenderName is null)
#define ERROR_CUSTOM_ACK_RESTART_LOGIC			34014		//Ack DTUServer Reatart Logic Cmd

#define ERROR_CUSTOM_SEND_WARNING_DATA			34020		//DTU Send WarningData Buffer(%d) Failed
#define ERROR_CUSTOM_SEND_WARNIN_GOPERATION		34021		//DTU Send WarningOperation Buffer(%d) Failed.
#define ERROR_CUSTOM_SEND_USER_OPERATION		34022		//DTU Send UserOperation Buffer(%d) Failed.
#define ERROR_CUSTOM_SEND_REALDATA				34023		//DTU Send RealData Buffer(%d) Failed.
#define ERROR_CUSTOM_SEND_UNIT					34024		// DTU Send Unit01 Buffer(%d) Failed
#define ERROR_CUSTOM_SEND_EXECUTE_LOG			34025		// DTU Send ExecuteLog Buffer(%d) Failed.
#define ERROR_CUSTOM_RESEND_REALDATA			34026		//DTU ReSend RealData(%s) Failed.
#define ERROR_CUSTOM_SEND_HISTORY				34027		// Send History File
#define ERROR_CUSTOM_SEND_REALFILE				34028		//Send RealData File(%s)

#define ERROR_CUSTOM_ACK_SYN_DATA				34040		//Ack DTUServer Syn Data Cmd
#define ERROR_CUSTOM_ACK_SYN_UNIT				34041		//Ack DTUServer Syn Unit01 Cmd
#define ERROR_CUSTOM_ACK_SYN_TIME				34042		//Ack DTUServer Syn SystemTime Cmd

#define ERROR_CUSTOM_ACK_EDIT_UNIT				34060		//Ack DTUServer Edit Unit01 Cmd

#define ERROR_CUSTOM_CONNECT_TCPSENDER			34080		//TCPDataSender connecting failed
#define ERROR_CUSTOM_CLOSE_TCPSENDER			34081		//Connection closed
#define ERROR_CUSTOM_REC_TCPSENDER				34082		//recv failed with error: %d
#define ERROR_CUSTOM__INIT_WINSOCK_TCPSENDER	34083		//Winsock 2 initialize failed with error: %d
#define ERROR_CUSTOM_CREATE_WINSOCK_TCPSENDER	34084		//Create the listener socket failed with error: %d
#define ERROR_CUSTOM_SEND_TCPSENDER				34085		//TcpIpComm Send datas failed
#define ERROR_CUSTOM_SET_WINSOCK_TCPSENDER		34086		//set the listener socket failed with error: %d.
#define ERROR_CUSTOM_TIMEOUT_TCPSENDER			34087		//connect time out: %d
#define ERROR_CUSTOM_RECONNECT_TCPSENDER		34088		//TCPDataSender reconnecting failed

//引擎
#define ERROR_CUSTOM_INIT_ENGINE				35000		//InitEngine Failed
#define ERROR_CUSTOM_INIT_OPC					35001		//OPC Engine Start Failed
#define ERROR_CUSTOM_INIT_BACNET				35002		//Bacnet Engine Start Failed
#define ERROR_CUSTOM_INIT_MODBUS				35003		//Init Modbus Engine(%s) Failed
#define ERROR_CUSTOM_INIT_PROTOCOL104			35004		//Init Protocol-104 Engine(%s) Failed
#define ERROR_CUSTOM_INIT_MYSQL					35005		//Init Mysql Engine(%s) Failed
#define ERROR_CUSTOM_INIT_FCBUS					35006		//Init FCbus Engine(%s:%s) Failed

//35100 Bacnet
#define ERROR_CUSTOM_INIT_BACNET_ADDRESS		35101		//ip address init failed
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_WRITE	35102		//address_bind_request failed(write)
#define	ERROR_CUSTOM_BACNET_TYPE_INVALID		35103		//bacnet type not in AI/AO/BI/BO/AV/BV/MI/MO/MV 
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_AI		35104		//"address_bind_request error in reading AI
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_AO		35105		//"address_bind_request error in reading AO
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_BI		35106		//"address_bind_request error in reading BI
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_BO		35107		//"address_bind_request error in reading BO
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_AV		35108		//"address_bind_request error in reading AV
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_BV		35109		//"address_bind_request error in reading BV
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_MI		35110		//"address_bind_request error in reading MI
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_MO		35111		//"address_bind_request error in reading MO
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_MV		35112		//"address_bind_request error in reading MV
#define	ERROR_CUSTOM_BACNET_ADDRESS_BIND_READ	35113		//address_bind_request:%d failed.
#define	ERROR_CUSTOM_BACNET_READ_MUTIL			35114		//Send_Read_Property_Multiple_Request:reqInvokeId=0

//35200 opc
#define ERROR_CUSTOM_GET_OPC_CACHE				35201		//Get Point() Value Failed.Please Check!
#define ERROR_CUSTOM_GET_OPC_DEVICE				35202		//Get Point() Value From Device Failed.Please Check!
#define ERROR_CUSTOM_GET_MUTIL_OPC_DEVICE		35203		//OPC Engine(%d):Read OPCValue From Device num:%d Time:%fs)
#define ERROR_CUSTOM_GET_OPC					35204		//Get Point() Value Failed.Please Check!
#define ERROR_CUSTOM_GET_MUTIL_OPC_VALUES		35205		//OPC Engine(%d):Read MutilValue num %d(%s) Failed.
#define ERROR_CUSTOM_GET_MUTIL_OPC_VELUE		35206		//OPC Engine(%d):Read MutilValue(%s) Failed.
#define ERROR_CUSTOM_UNEXIST_OPC_ITEM			35207		//OPCServer Can't find %s
#define ERROR_CUSTOM_OPCSERVICE_DISCONNECT		35208		//被调用的对象已与其客户端断开连接
#define ERROR_CUSTOM_OPCSERVICE_FAIL			35209		//远程过程调用失败
#define ERROR_CUSTOM_OPCSERVICE_UN_FIND			35210		//Can not find OPC Service
#define ERROR_CUSTOM_PRC_FAIL					35211		//RPC 服务器不可用
#define ERROR_CUSTOM_OPCITEM_UNEXIST			35212		//OPC does not exist
#define ERROR_CUSTOM_OPCITEM_PROPERTY_NULL		35213		//IOPCItemProperties OPC return NULL
#define ERROR_CUSTOM_READ_OPC					35214		//OPCEngine Read MutilValue num (%d/%d) Failed.
#define ERROR_CUSTOM_CONNECT_OPC				35215		//OPC Engine Connected Failed
#define ERROR_CUSTOM_OPC_VALUE_NOT_GOOD			35216		//OPC OPC_QUALITY_GOOD_NON_SPECIFIC
#define ERROR_CUSTOM_OPC_GET_VALUE_FAIL			35217		//OPC GetValueSet UnConnect
#define ERROR_CUSTOM_RECONNECT_OPC				35218		//OPC Engine ReConnected Failed

//35300 fcbus
#define ERROR_CUSTOM_WRITE_CFBUS				35301		// FCBus write point send package failed
#define ERROR_CUSTOM_RECONNECT_CFBUS			35302		// FCBbus reconnecting failed

//35400 modbus
#define ERROR_CUSTOM_MODBUS_01					35401		//modbus fun 01 Error:id-%d,addr-%d, errcode-%d
#define ERROR_CUSTOM_MODBUS_02					35402		//modbus fun 02 Error:id-%d,addr-%d, errcode-%d
#define ERROR_CUSTOM_MODBUS_03					35403		//modbus fun 03 Error:id-%d,addr-%d, errcode-%d
#define ERROR_CUSTOM_MODBUS_04					35404		//modbus fun 04 Error:id-%d,addr-%d, errcode-%d
#define ERROR_CUSTOM_MODBUS_05					35405		//modbus fun 05 Error:id-%d,addr-%d, errcode-%d
#define ERROR_CUSTOM_MODBUS_06					35406		//modbus fun 06 Error:id-%d,addr-%d, errcode-%d
#define ERROR_CUSTOM_MODBUS_16					35407		//modbus fun 16 Error:id-%d,addr-%d, errcode-%d

#define ERROR_CUSTOM_CONNECT_MODBUS				35410		//connect fail
#define ERROR_CUSTOM_MODBUS_INVALID				35411		//modbus entry %s params not valid, skip
#define ERROR_CUSTOM_SEND_MODBUS				35412		//send fail
#define ERROR_CUSTOM_MODBUS_NO_RESPONSE			35413		//modbus slave(ADDR: %d) no response!
#define ERROR_CUSTOM_MODBUS_UN_REC				35414		//Read modbus data Un-Recved, error %d times.
#define ERROR_CUSTOM_WRITE_MODBUS				35415		//modbus write point send package failed
#define ERROR_CUSTOM_RECONNECT_MODBUS			35416		//Modbus reconnecting failed
#define ERROR_CUSTOM_SEND_ONE_MODBUS			35417		//send onebyone fail
#define ERROR_CUSTOM_WRITE_MODBUS_VALID			35418		//modbus write point params invalid
#define ERROR_CUSTOM_RECONNECT_CKECO3P			35419		//Modbus reconnecting failed
#define ERROR_CUSTOM_CO3P_UN_REC				35420		//Read co3p data Un-Recved, error %d times.
#define ERROR_CUSTOM_CO3P_INVALID				35421		//co3p entry %s params not valid, skip
#define ERROR_CUSTOM_SEND_CO3P					35422		//send fail
#define ERROR_CUSTOM_CO3P_NO_RESPONSE			35423		//modbus slave(ADDR: %d) no response!

//35500 tcp
#define ERROR_CUSTOM_CONNECT_TCP				35500		//Cannot connect to IP by Socket
#define ERROR_CUSTOM_CLOSE_TCP					35501		//Connection closed
#define ERROR_CUSTOM_REC_TCP					35502		//recv failed with error: %d
#define ERROR_CUSTOM__INIT_WINSOCK_TCP			35503		//Winsock 2 initialize failed with error: %d
#define ERROR_CUSTOM_CREATE_WINSOCK_TCP			35504		//Create the listener socket failed with error: %d
#define ERROR_CUSTOM_SEND_TCP					35505		//TcpIpComm Send datas failed

//35600 mysql
#define ERROR_CUSTOM_CONNECT_MYSQL				35600		//connect fail

//35700 104
#define ERROR_CUSTOM_CONNECT_PROTOCOL104		35700		//connect fail
#define ERROR_CUSTOM_RECONNECT_PROTOCOL104		35701		//Protocol104 reconnecting failed

//35800 s7UDP
#define ERROR_CUSTOM_CONNECT_S7UDP				35800		//Couldn't connect to PLC
#define ERROR_CUSTOM_RECONNECT_S7UDP			35801		//Protocol104 reconnecting failed
#define ERROR_CUSTOM_READ_S7UDP					35802		//ERROR SiemensTCP reading From %s To %s
#define ERROR_CUSTOM_QUERY_S7UDP				35803		//ERROR SiemensTCP query %s
#define ERROR_CUSTOM_READ_SIX					35804		//ERROR in SiemensTCP reading From %s To %s(daveUseResult faile six time in a row)
#define ERROR_CUSTOM_CONNECT_S7UDP_ADD			35805		//Couldn't connect to PLC addressing
#define ERROR_CUSTOM_READ_ONE_S7UDP				35806		//ERROR SiemensTCP reading %sERROR SiemensTCP reading From %s To %s
#define ERROR_CUSTOM_QUERY_ONE_S7UDP			35807		//ERROR SiemensTCP query %s