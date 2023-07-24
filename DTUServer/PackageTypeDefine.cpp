

#include "stdafx.h"

#include "PackageTypeDefine.h"

namespace weldtech
{
	const int PACKAGE_PREFIX_LEN = 4;
	const int PACKAGE_PREFIX_LEN_TIME = 5;
	const char* POINTDATAPACKAGE_PREFIX = "pdp:";
	const char* OPERATIONRECORDPACKAGE_PREFIX = "orp:";
	const char* CALCRESULTPACKAGE_PREFIX = "crp:";
	const char* WARNINGDATAPACKAGE_PREFIX = "wdp:";				//报警数据包头
	const char* WARNINGOPERATIONPACKAGE_PREFIX = "wop:";			//报警操作包头
	const char* POINTDATAPACKAGE_PREFIX_TIME = "time:";
}

weldtech::CPackageType::DTU_DATATYPE weldtech::CPackageType::GetPackageType( const char* buffer )
{
	ASSERT(buffer != NULL);
	int bufferlen = strlen(buffer);
	if (bufferlen <= PACKAGE_PREFIX_LEN){
		return Type_InValid;
	}

	char buffer_prefix[PACKAGE_PREFIX_LEN+1];
	memcpy(buffer_prefix, buffer, PACKAGE_PREFIX_LEN);
	buffer_prefix[PACKAGE_PREFIX_LEN] = '\0';

	if (strcmp(buffer_prefix, POINTDATAPACKAGE_PREFIX) == 0)
	{
		char buffer_timefield[PACKAGE_PREFIX_LEN_TIME+1] = {0};
		memcpy(buffer_timefield, buffer + PACKAGE_PREFIX_LEN, PACKAGE_PREFIX_LEN_TIME);
		buffer_timefield[PACKAGE_PREFIX_LEN_TIME] = '\0';

		if (strcmp(buffer_timefield, POINTDATAPACKAGE_PREFIX_TIME) == 0){
			return Type_PointData_WithTime;
		}
		return Type_PointData;
	}else if (strcmp(buffer_prefix, OPERATIONRECORDPACKAGE_PREFIX) == 0)
	{
		return Type_OperationRecord;
	}
	else if (strcmp(buffer_prefix, CALCRESULTPACKAGE_PREFIX) == 0)
	{
		return Type_OptimizeResult;
	}
	else if (strcmp(buffer_prefix, WARNINGDATAPACKAGE_PREFIX) == 0)
	{
		return Type_WarningData;
	}
	else if (strcmp(buffer_prefix, WARNINGOPERATIONPACKAGE_PREFIX) == 0)
	{
		return Type_WarningOperation;
	}
	return Type_InValid;
}

char* weldtech::CPackageType::RemovePrefix( char* buffer )
{
	DTU_DATATYPE datatype = GetPackageType(buffer);
	if (datatype != Type_InValid)
	{
		if (datatype == Type_PointData_WithTime || datatype == Type_WarningData || datatype == Type_WarningOperation )
		{
			int len = strlen(buffer);
			memmove(buffer,buffer+9,len-9);
			buffer[len-9]='\0';
		}
		else{
			int len = strlen(buffer);
			memmove(buffer,buffer+4,len-4);
			buffer[len-4]='\0';
		}
		
	}

	return buffer;
}
