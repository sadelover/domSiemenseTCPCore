
#pragma once

#ifndef RUNNINGRECORDOPERATION_H__
#define RUNNINGRECORDOPERATION_H__

//#include <vector>
#include <string>

#include "DBInterfaceImp.h"

//#include <WTypes.h>
//template class _ std::allocator<char>;
//template class _ std::basic_string<char>;
//template class _ std::allocator<WCHAR>;
//template class _ std::basic_string<WCHAR>;

class COperationRecord
{
public:
    COperationRecord(){};

    std::wstring    m_strTime;
    std::wstring    m_strOperation;
    std::wstring    m_strUser;

};
typedef std::vector<COperationRecord>   VEC_OP_RECORD;

class CWarningRecord
{
public:
    CWarningRecord(){};

	std::wstring m_strTime;

	std::wstring m_warninginfo;
};
typedef std::vector<CWarningRecord> WARNING_LIST;
#endif
