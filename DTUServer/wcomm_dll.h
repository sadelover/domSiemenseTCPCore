#ifndef __COMM_NET_H
#define __COMM_NET_H

#ifndef DSCDLLAPI
#define DSCDLLAPI  extern "C"
#endif

typedef long           int32;	/* 32-bit signed integer */
typedef unsigned int   uint;	/* 16 or 32-bit unsigned integer */
typedef unsigned long  uint32;	/* 32-bit unsigned integer */
typedef unsigned short uint16;	/* 16-bit unsigned integer */
typedef unsigned char  byte_t;	/*  8-bit unsigned integer */
typedef unsigned char  uint8;	/* 8-bit unsigned integer */

#define MAX_RECEIVE_BUF 1024  //the max data packet length that would be received
#define MAX_SEND_BUF MAX_RECEIVE_BUF  //the max data packet length that would be sent

#define DEFAULT_CLIENT_PORT 5001 //the default monitoring port on terminal

#ifndef  HDCALL
#define HDCALL WINAPI
#endif

/*
 * user info interface
 */
typedef struct _USER_INFO{
	char       m_userid[12];   	//DTU Identify number
	uint32     m_sin_addr;     	//the ip address of DTU in Internet,maybe a gateway ip addr
	uint16     m_sin_port;     	//the ip port of DTU in Internet
	uint32     m_local_addr;   	//the ip address of DTU in local mobile net
	uint16     m_local_port;   	//the local port of DTU in local mobile net
	char       m_logon_date[20]; 	//the date that the DTU logon 
	char	   m_update_time[20]; //the last date that receive IP packet
	uint8	   m_status;		//DTU status, on line 1 : offline 0
}user_info;
/*
 * user data record interface
 * as param when call function do_read_proc()
 * m_data_type: if 0 unknown type, 0x01 DTU logon;0x04 invalid command;
 * 0x05 DTU receive data successfully;
 * 0x0d setup parameters successfully;0x0b query parameters successfully;
 * 0x06 disconnect ppp link successfully;0x07 stop send data to DSC successfully;
 * 0x08 start to send data to DSC successfully;0x0A flush DTU data in cache successfully;
 * 0x09 user data type
 */
typedef struct _USER_DATA_RECORD{
	char       m_userid[12];		//DTU Identify number
	char       m_recv_date[20];		//the date that receive data packet
	char       m_data_buf[MAX_RECEIVE_BUF]; //store data packet
	uint16     m_data_len;			//the data length
	uint8      m_data_type;	//data type
}data_record;
/*
 * Get the user amount that could be supported 
 * return uint
 */
DSCDLLAPI  uint HDCALL get_max_user_amount();
/*
 * Get the online user amount in user list
 */
DSCDLLAPI  uint HDCALL get_online_user_amount();
/*
 * Get the user info in user list, if exist 0,else -1
 * @param userid: 	[in] The user's identity number. 
 * @param infoPtr:	[out] A pointer to a buffer that receives the user info
 */
DSCDLLAPI  int HDCALL get_user_info(uint8 *userid,user_info *infoPtr);

/*
 * Get a user's info at one position 
 * @param index: 	[in] 0 ~ get_max_user_amount().
 * @param infoPtr:	[out] A pointer to a buffer that receives the user info
 */
DSCDLLAPI  int HDCALL get_user_at(uint index,user_info *infoPtr);
/*
 * Start net service
 * @param hWnd  A handle identifying the window that will receive a message when a network event occurs. 
 * @param wMsg  The message to be received when a network event occurs. 
 *	    default value: const int WM_CLIENT_READCLOSE=WM_USER+105;
 * @param nServerPort network monitor port,default 5002
 * @param mess:		[out] A pointer to a buffer that store error message
 * A deprecated function,it was replaced by function start_net_service()
 */
DSCDLLAPI int HDCALL start_gprs_server(HWND hWnd,unsigned int wMsg,int nServerPort,char *mess);
/*
 * Stop net service
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 * A deprecated function,it was replaced by function stop_net_service()
 */
DSCDLLAPI int HDCALL stop_gprs_server(char *mess);
 /*
 * Process all data from DTU
 * @param recdPtr: 	[out] A pointer to a struct that store dtu data info
 * @param mess:		[out] A pointer to a buffer that store message
 * @param reply: 	[in] If true,the DSC will auto-reply the DTU when it receive data from DTU.
 *					when release your program,use false，when debug your program,use true
 * return -1 failed,0 successful
 */
DSCDLLAPI int HDCALL do_read_proc(data_record *recdPtr,char *mess,BOOL reply);
/*
 * Send data to DTU
 * @param userid: 	[in] The user's identity number. 
 * @param data:	[in] The data that will be sent
 * @param len: 	[in] The length of the data. 
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed,0 successful
 */
DSCDLLAPI int HDCALL do_send_user_data(uint8* userid,uint8*data,uint len,char *mess);
/*
 * Close one DTU in current user list and send close command to DTU
 * @param userid: 	[in] The user's identity number. 
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 */
DSCDLLAPI int HDCALL do_close_one_user(uint8* userid,char *mess);
/*
 * Close all DTU in current user list and send close command to DTU
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 */
DSCDLLAPI int HDCALL do_close_all_user(char *mess);
/*
 * Close one DTU in current user list and send close command to DTU
 * @param userid: 	[in] The user's identity number. 
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 * A deprecated function
 */
DSCDLLAPI int HDCALL do_close_one_user2(uint8* userid,char *mess);
/*
 * Close all DTU in current user list and send close command to dtu
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 * A deprecated function
 */
DSCDLLAPI int HDCALL do_close_all_user2(char *mess);

DSCDLLAPI unsigned long HDCALL GetCurrentIP(void);

/*
 * Set up IP that will be binded
 * @param ulIPAddr: [in] IP address. network byte order
 */
DSCDLLAPI void  HDCALL SetCustomIP(unsigned long ulIPAddr);	

/*
 * A deprecated function,it was replaced by function delete_all_user()
 */
DSCDLLAPI void HDCALL DeleteAllUser(void);
/*
 * when start server,u can call this function to add a user.
 * A deprecated function,it was replaced by function add_one_user()
 */
DSCDLLAPI int HDCALL AddOneUser(user_info *pUserInfo);
/*
 *Add by SEA on Oct 28,2003
 *Modified by SEA Oct 29,2003
 *Setup work mode
 *@param nWorkMode: 		[in] The Mode of dll to work. 
 *0-use blocking mode and uses thread to receive data
 *1-use nonblocking mode,not need windows's handle and message
 *2-use nonblocking mode,must transfer valid window's handle and message
 *default value is 2 ,it is compatible with previous version
 */

DSCDLLAPI int  HDCALL SetWorkMode(int nWorkMode);

/*
0	UDP
1	TCP
*/
DSCDLLAPI int  HDCALL SelectProtocol(int nProtocol);

////////////////////////////////////////////////////////////////////////////////	
/*
 * Parameters setting API
 * 1.ClearParam()
 * 2.SetParam(...)
 * 3.do_update_param(...)
 */
/*
 *Clear paramters' buffer
 *Call this function before setting parameters
 */
DSCDLLAPI void  HDCALL ClearParam();
/*
 * Add parameter that would be updated.
 * @param nParamType: 	[in] parameter's type,see in param_types.h. 
 * @param cpValue: 		[in] parameter's value that would be updated. 
 * @param nParamLenth: 	[in] parameter's length. 
 * @param errorCode:	[out] error code that would be returned.
 * return 1 successful, 0 failed
 */
DSCDLLAPI int  HDCALL SetParam(int nParamType, char *cpValue, int nParamLenth, int *errorCode);
/*
 * Update DTU parameters
 * destIP: DTU's IP(host byte order)
 * destPort: DTU's IP port(host byte order)
 * return 1 successful, 0 failed
 * A deprecated function,it was replaced by function do_update_param()
 */
DSCDLLAPI int  HDCALL DoUpdateParam(const unsigned long destIP,const unsigned short destPort, char *m_userid);
/*
 * Read DTU parameters
 * return 1 successful, 0 failed
 * A deprecated function,it was replaced by function do_read_param()
 */
DSCDLLAPI int  HDCALL DoReadParam(const unsigned long destIP,const unsigned short destPort, char *m_userid);
/*
 * Read a parameter
 * @param nParamType: 	[in] parameter's type,see in param_types.h. 
 * @param cpValue: 		[in] A pointer to a buffer that used to store parameter's value. 
 * @param nParamLenth: 	[in] cpValue's length [out] parameter's length. 
 * return 1 successful, 0 failed
 */
DSCDLLAPI int  HDCALL GetParam(int nParamType, char *cpValue, int *nParamLenth);

////////////used to maintian user list//////////////////////////////////////////////////////////
/*
 *  服务启动后，调用add_one_user函数可向系统的用户表中
 *  添加一个用户，这样可以省略掉DTU向中心登录的过程；
 *  该情况适用于中心重启后，而DTU并没有立即登录，中心
 *  为了保持和DTU之间的联系，用户可调用函数add_one_user
 *  向用户列表中手工添加用户；
 */
/*
 * When start server,u can call this function to add a user. 
 * return -1 failed,0 successful
 * This function used to replace function AddOneUser();
 */
DSCDLLAPI int HDCALL add_one_user(user_info * pUserInfo);
/*
 * It will delete all users in user list
 * return -1 failed or nonexistent,0 successful
 * This function used to replace function DeleteAllUser();
 */
DSCDLLAPI int HDCALL delete_all_user(void);
/*
 * Delete one user in user list
 * Close one DTU in current user list but no really send close command to DTU
 * return -1 failed or nonexistent,0 successful
 */
DSCDLLAPI int HDCALL delete_one_user(uint8* userid,char *mess);
////////////////////////////////////////////////////////////////////////////////
/*
 * Start net service
 * @param hWnd  A handle identifying the window that will receive a message when a network event occurs. 
 * @param wMsg  The message to be received when a network event occurs. 
 *	    default value: const int WM_CLIENT_READCLOSE=WM_USER+105;
 * @param nServerPort network monitor port,default 5002
 * @param mess:		[out] A pointer to a buffer that store error message
 *  This function used to replace function start_gprs_server();
 */
DSCDLLAPI int HDCALL start_net_service(HWND hWnd,unsigned int wMsg,int nServerPort,char *mess);
/*
 * Stop net service
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 *  This function used to replace function stop_gprs_server();
 */
DSCDLLAPI int HDCALL stop_net_service(char *mess);
/*
 * Update DTU parameters,the DTU always response it
 * @param userid: 	[in] The user's identity number. 
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 */
DSCDLLAPI int HDCALL do_update_param(uint8* userid,char *mess);
/*
 * Read DTU parameters
 * @param userid: 	[in] The user's identity number. 
 * @param qtype: 	[in] The option that will be queried. 0x00 query all parameters
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 */
DSCDLLAPI int HDCALL do_read_param(uint8* userid,uint8 qtype,char *mess);
/*
 * Added on Feb 28,2005
 * Run on version 2.6.4 or later version
 */
/*
 * Send command to remote DTU,the DTU always response it
 * The DTU will disconnect ppp link and wait to redial(when setup reconnect interval)
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 */
DSCDLLAPI int HDCALL do_disconnect_ppp_link(uint8* userid,char *mess);
/*
 * Send command to remote DTU,the DTU always response it
 * The DTU will stop to send data to DSC
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 */
DSCDLLAPI int HDCALL do_stop_send_data(uint8* userid,char *mess);
/*
 * Send command to remote DTU,the DTU always response it
 * The DTU will start to send data to DSC
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 */
DSCDLLAPI int HDCALL do_start_send_data(uint8* userid,char *mess);
/*
 * Send command to remote DTU,the DTU always response it
 * The data in DTU cache will be discarded
 * @param mess:		[out] A pointer to a buffer that store error message
 * return -1 failed or nonexistent,0 successful
 */
DSCDLLAPI int HDCALL do_flush_cache_data(uint8* userid,char *mess);

/*
 * Cancel block when call function do_read_proc.
 * It can only be used after call function SetWorkMode(0)
 */
DSCDLLAPI void HDCALL cancel_read_block();
////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
