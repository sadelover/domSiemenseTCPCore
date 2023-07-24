#pragma once

#ifndef _OPCTESTCLIENT_STDAFX_H
#define _OPCTESTCLIENT_STDAFX_H

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxcview.h>
#include <afxmt.h>			// MFC support for multithreaded synchronization objects
#include <afxadv.h>
#include <process.h>
#include "ocidl.h"
#include "comcat.h"

#include "./BottCode/globals.h"
#include "./BottCode/safelock.h"
#include "./BottCode/safearray.h"
#include "./BottCode/opcda.h"
#include "./BottCode/opccomn.h"
#include "./BottCode/opcerrors.h"
#include "./BottCode/opcprops.h"
#include "./BottCode/opcquality.h"
#include "./BottCode/server.h"
#include "./BottCode/group.h"
#include "./BottCode/item.h"

 #ifndef tstring
 #include <string>
 #if defined(_UNICODE) || defined(UNICODE)
 	#define tstring	std::wstring
 #else
 	#define tstring	std::string
 #endif
 #endif

#endif // _OPCTESTCLIENT_STDAFX_H
