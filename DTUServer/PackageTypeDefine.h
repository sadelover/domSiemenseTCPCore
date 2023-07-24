

/*
 *
 *
 * this class will defines package type of the data transferred between 
 * dtu client and dtu server.
 *
 *
 */


#pragma once

namespace weldtech
{
	class  CPackageType
	{
	public:
		enum DTU_DATATYPE
		{
			Type_InValid,
			Type_PointData,
			Type_OperationRecord,
			Type_OptimizeResult,
			Type_PointData_WithTime,		//带时间戳的数据
			Type_WarningData,				//报警数据
			Type_WarningOperation,			//报警操作
		};

		static DTU_DATATYPE  GetPackageType(const char* buffer);
		static char*	RemovePrefix(char* buffer);
	};

}
