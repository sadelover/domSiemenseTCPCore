#ifndef UPPC_DBBASICIO_POINTRECORD_DEFINE_H_
#define UPPC_DBBASICIO_POINTRECORD_DEFINE_H_


/********************************************************************************
*CRunningRecordHelper.h - declare the class CRunningRecordHelper
*
*       Copyright (c) WeldTeh Corporation. All rights reserved.
*
*Purpose:
*		define the struct which used to save the record.
*		of data sensor meters, such as flow, pressure difference,
*		power, temperature and so on.
*     
*
*Note:
*       This class will not be exported.
*       
*
*Author:
*      ¹ð±Ø³É
*******************************************************************************/

// this is consistent with the length of the column of the table 
// in the  database.
#define POINT_NAME_LENGTH 64

// it contains only POD data. 
typedef struct tag_DataSensor_Record_Entry
{
	SYSTEMTIME current_time;					// current time
	char       point_name[POINT_NAME_LENGTH];	// the point name. shortname
	double	   sensor_value;					// the real value.
} DataSensor_Record_Entry, *pDataSensor_Record_Entry;




#endif