#ifndef __ERROR_CODE_LOG_H__
#define __ERROR_CODE_LOG_H__
#include "../Tools/CustomTools/CustomTools.h"

class CErrCodeLog
{
public:
	virtual ~CErrCodeLog();                         // default destructor

	static CErrCodeLog* GetInstance();              // get the singleton instance of assign object
	static void DestroyInstance();                   // destroy the singleton instance of assign objec

protected:
	void InitParams();                    // initialize parameters and object
	
private:
	CErrCodeLog();                              // default constructor

private:
	static CErrCodeLog*    m_csDebugCommObj;     // singleton object
	static Mutex            m_mutexLock;                    // mutex object
};

#endif  
