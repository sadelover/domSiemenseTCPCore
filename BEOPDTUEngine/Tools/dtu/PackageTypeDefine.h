

/*
 *
 *
 * this class will defines package type of the data transferred between 
 * dtu client and dtu server.
 *
 *
 */


#pragma once

namespace rnbtech
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
			Type_DTUServerCmd,				//DTUServerCmd  普通的文本命令
			Type_DTUServerFrl,				//DTUServerCmd文件接收Log
			Type_DTUServerFup,				//DTUServerCmd文件更新
			Type_DTUWriteTime,				//DTU发送时间
			Type_DTUServerPoint,			//DTUServerCmd点表更新
			Type_DTUServerDll,				//DTUServerCmd Dll更新
			Type_DTUServerS3db,				//DTUServerCmd S3db更新
			Type_WebServerFup,				//WebServerCmd文件更新
		};

		static DTU_DATATYPE  GetPackageType(const char* buffer);
		static int	RemovePrefix(char* buffer,int length=-1);
	};

}
