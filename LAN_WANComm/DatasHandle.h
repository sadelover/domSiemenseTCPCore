#ifndef DATAS_HANDLE_H
#define DATAS_HANDLE_H


#include <string>                         // std::string
#include <vector>                         // std::vector
#include <hash_map>
using namespace std;
using namespace stdext;

#include "Tools/ToolsFunction/ToolsFunction.h"
//#include "Equipments/IData_SensorIO.h" // Data_Sensor structure  

#define MAX_FORMAT_SIZE 512
//
//typedef struct tagSPLITTED_PACKAGE_ELEMENT
//{
//	Data_Sensor dataSensor;
//	hash_map<int, wstring> hashmapValue;
//
//	tagSPLITTED_PACKAGE_ELEMENT()
//	{
//		//memset(this, 0, sizeof(*this));
//	}
//
//}SplittedPackElement, *pSplittedPackElement;

class CDatasHandle
{
public:
	CDatasHandle();              // default constructor
	virtual ~CDatasHandle();     // default destructor

public:
	//virtual void CreatePackage(__out string& pchDest, 
	//	__in const Data_Sensor& dataSensor);               // create package with src datas(Data_Sensor)
	virtual void CreatePackage(__out string& pchDest, 
		__in const hash_map<int, wstring>& hashmapValue);  // create package with src datas(hash_map<int, wstring>)
	virtual void CreatePackage(__out string& pchDest, 
		__in const hash_map<int, int>& hashmapEqmStatus);  // create package with src datas(hash_map<int, bool>)                                    
	virtual void CreatePackage(__out string& pchDest, 
		__in const string& pchSrc);                  // create package with src datas(string)

	/*virtual Data_Sensor PackageSplitDataSensor(__in const string& src, 
		__in string delimit = ";");                  // package split
			*/
	virtual hash_map<int, wstring> PackageSplitHashMap_Wstring(__in const string& src,
		__in string delimit = ";", 
		__in string subDelimit = ",");              // package split
	//virtual SplittedPackElement PackageSplitDataSensor_HashMap(__in const string& src, 
	//	__in string main_delimit = "|", 
	//	__in string delimit = ";", 
	//	__in string subDelimit = ",");              // package split
	//
	virtual hash_map<int, bool> PackageSplitHashMap_bool(__in const string& src, 
		__in string delimit = ";", 
		__in string subDelimit = ",");              // package split

	string GetPackageType() const;                  // get the created package type
	void SetPackageType(__in const string& strType);// set the created package type

	void Split(__out vector<string>& des, 
		__in const string& src, 
		__in string& delim);                        // split function

private:
	//vector<double> GetEquipmentsValueArray(__in const Data_Sensor& dataSensor);  // get the equipments value array

private:
	CToolsFunction m_toolsFunction;

	string m_strPackageType;
};

#endif  // DATAS_HANDLE_H