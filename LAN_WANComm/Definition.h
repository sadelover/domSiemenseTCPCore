#ifndef NETWORK_DEFINITION_H
#define NETWORK_DEFINITION_H



#include <string>
#include <vector>
using namespace std;

#ifdef _UNICODE
  typedef wstring _tstring;
#else
  typedef string _tstring;
#endif  // _UNICODE


typedef struct tagMESSAGEPARAM
{
	HWND windowsHandle;          // the handle of windows needed to message
	UINT uEqmStatusMsg;          // the message type -- Equipment status
	UINT uDatabaseDatasMsg;      // the message type -- Database datas

	tagMESSAGEPARAM()
	{
		memset(this, NULL, sizeof(*this));
	}
}MessageParam, *pMessageParam;

typedef struct tagUpdateElement
{
	_tstring strUpdateFileName;
	_tstring strUpdateFilePath;
	_tstring strUpdateFileVersion;

	tagUpdateElement()
	{
		strUpdateFileName = _T("");
		strUpdateFilePath = _T("");
		strUpdateFileVersion = _T("");
	}
}UpdateElement, *pUpdateElement;

typedef struct tagUpdateContents
{
	_tstring strXmlFileVersion;
	vector<UpdateElement> vUpdateElements;

	tagUpdateContents()
	{
		strXmlFileVersion = _T("");
		vUpdateElements.clear();
	}
}UpdateContents, *pUpdateContents;

#endif  // NETWORK_DEFINITION_H