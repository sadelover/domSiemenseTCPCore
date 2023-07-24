
#include "stdafx.h"
#include "ErrCodeLog.h"

// Static object.
CErrCodeLog* CErrCodeLog::m_csDebugCommObj = NULL;
Mutex CErrCodeLog::m_mutexLock;

//======================================================================
// Default constructor.
CErrCodeLog::CErrCodeLog()
	//======================================================================
{
	// Initialize parameters and objects.
	InitParams();

}

//======================================================================
// Default destructor.
CErrCodeLog::~CErrCodeLog()
	//======================================================================
{
}

//======================================================================
// Get the singleton instance of assign object.
CErrCodeLog* CErrCodeLog::GetInstance()
	//======================================================================
{
	if (m_csDebugCommObj == NULL)
	{
		Scoped_Lock<Mutex> mut(m_mutexLock);
		m_csDebugCommObj = new CErrCodeLog();
		atexit(DestroyInstance);
	}

	return m_csDebugCommObj;
}

//======================================================================
// Destroy the singleton instance of assign object.
void CErrCodeLog::DestroyInstance()
	//======================================================================
{
	if (m_csDebugCommObj != NULL)
	{
		delete m_csDebugCommObj;
		m_csDebugCommObj = NULL;
	}
}

//======================================================================
// Initialize parameters and object.
void CErrCodeLog::InitParams()
	//======================================================================
{
	
}
