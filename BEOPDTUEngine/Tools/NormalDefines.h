#pragma once 
#ifndef  _NORMALDEFINES_H
#define  _NORMALDEFINES_H

#pragma warning(disable:4251)
#pragma warning(disable:4275)
#pragma warning(disable:4995)

#include "windows.h"
#include "StringDef.h"

#define Beop_Try			try							\
							{

#define Beop_Catch(x)		}							\
	catch(...)					\
							{							\
							/*AfxMessageBox((x));*/		\
							}

#define chSTR2(x)			#x
#define chSTR(x)			chSTR2(x)
#define chMSG( desc )		message( __FILE__ "(" chSTR(__LINE__) "):" #desc )

#define LOG_MAXLEVEL_ERROR	5
#define LOG_MINLEVEL_ALL	1

#define RATIO_W_PUMP		0.95
#define KW_TO_TON_MULTI		3.516
#define NUM_CT_COEFF		50

#define INIT_CriticalSection			InitializeCriticalSection(&m_cs);
#define	DELT_CriticalSection			DeleteCriticalSection(&m_cs);
#define LOCK							EnterCriticalSection(&m_cs);
#define UNLOCK							LeaveCriticalSection(&m_cs);

#define _MINUTES		*(60*1000)
#define _SECONDS		*1000

//语言类型
enum eLanguageType
{
	Language_Chinese,
	Language_English,
};

typedef struct tag__LogValve 
{
	bool	bWrite;
	int		iLevel;
}_LogValve, *p_LogValve;

/*
分段函数两点
*/
typedef struct tag_PwFunc_2P
{
    tag_PwFunc_2P()
        :dbX1(0)
    ,dbY1(0)
    ,dbX2(0)
    ,dbY2(0)
    {}
	double dbX1;
	double dbY1;
	double dbX2;
	double dbY2;
}PwFunc_2P, *pPwFunc_2P;

// 对于数据库的NULL值，转换为0
#define ATOI(s) ((s) != NULL ? atoi(s) : 0)

#define ATOF(s) ((s) != NULL ? atof(s) : 0.f)


#endif //_NORMALDEFINES_H
