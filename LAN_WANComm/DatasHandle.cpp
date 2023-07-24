// DatasHandle.h implement

#include "stdafx.h"
#include "DatasHandle.h"

#pragma warning(disable:4996 4267)
//#include "tools/vld.h"

// Default contructor.
CDatasHandle::CDatasHandle()
{
	m_strPackageType = "";
}

// Default destructor.
CDatasHandle::~CDatasHandle()
{
}

// Create package with src datas(Data_Sensor).
//void CDatasHandle::CreatePackage(string& pchDest, const Data_Sensor& dataSensor)
//{
//	unsigned int nIndex = 0;
//	char chFormat[MAX_FORMAT_SIZE] = {0};
//	string strDest;
//	strDest.clear();
//
//	// Get the package type string.
//	strDest = GetPackageType();
//
//	vector<double> vecEquipmentsValue;/* = GetEquipmentsValueArray(dataSensor);*/
//
//	for (unsigned int i = 0; i < vecEquipmentsValue.size(); i++)
//	{
//		// Get equipment data point and format.
//		sprintf(chFormat, "%.15f", vecEquipmentsValue[i]);
//		strDest += chFormat;
//		strDest += ";";	
//	}
//
//	pchDest = strDest;
//}

// Create package with src datas(hash_map<int, wstring>).
void CDatasHandle::CreatePackage(string& pchDest, const hash_map<int, wstring>& hashmapValue) 
{
	char chFormat[MAX_FORMAT_SIZE] = {0};
	string strDest;
	strDest.clear();

	// Get the package type string.
	strDest = GetPackageType();

	for (hash_map<int, wstring>::const_iterator constIt = hashmapValue.begin(); constIt != hashmapValue.end(); constIt++)
	{
		// Get equipment data point and format.
		char chValue[MAX_PATH] = {0};
		wstring result = (*constIt).second;
		wsprintfA(chValue, "%S", result.c_str());
		sprintf(chFormat, "%d,%s", (*constIt).first, chValue);
		strDest += chFormat;
		strDest += ";";	
	}

	pchDest = strDest;
}

// Create package with src datas(hash_map<int, bool>).
void CDatasHandle::CreatePackage(string& pchDest, const hash_map<int, int>& hashmapEqmStatus)
{
	char chFormat[MAX_FORMAT_SIZE] = {0};
	string strDest;
	strDest.clear();

	// Get the package type string.
	strDest = GetPackageType();

	for (hash_map<int, int>::const_iterator constIt = hashmapEqmStatus.begin(); constIt != hashmapEqmStatus.end(); constIt++)
	{
		// Get equipment data point and format.
		sprintf(chFormat, "%d,%d", (*constIt).first, (*constIt).second);
		strDest += chFormat;
		strDest += ";";	
	}

	pchDest = strDest;
}

// Create package with src datas(string).
void CDatasHandle::CreatePackage(string& pchDest, const string& pchSrc)
{
	string strDest;
	strDest.clear();

	strDest += pchSrc;
	strDest += "|";	
	pchDest = strDest;
}
//
//// Package split.
//Data_Sensor CDatasHandle::PackageSplitDataSensor(const string& src, string delimit /*= ";"*/)
//{
//	Data_Sensor dataSensorElements;
//	vector<string> vStringElements;
//	vStringElements.clear();
//	memset(&dataSensorElements, 0, sizeof(Data_Sensor));
//
//	Split(vStringElements, src, delimit);
//	if (vStringElements.size() >= 32)
//	{
//		dataSensorElements.dbT_chws0 = atof(vStringElements[0].c_str());
//		dataSensorElements.dbT_chwr0 = atof(vStringElements[1].c_str());
//		dataSensorElements.dbQ_chw0 = atof(vStringElements[2].c_str());
//		dataSensorElements.dbT_dbin = atof(vStringElements[3].c_str());
//		dataSensorElements.dbRH = atof(vStringElements[4].c_str());
//		dataSensorElements.dbT_wbin = atof(vStringElements[5].c_str());
//		dataSensorElements.dbT_win = atof(vStringElements[6].c_str());
//		dataSensorElements.dbT_wout = atof(vStringElements[7].c_str());
//		dataSensorElements.dbFlow_Chw1 = atof(vStringElements[8].c_str());
//		dataSensorElements.dbFlow_Chw2 = atof(vStringElements[9].c_str());
//		dataSensorElements.dbFlow_CW = atof(vStringElements[10].c_str());
//		dataSensorElements.dbChw2DP = atof(vStringElements[11].c_str());
//		dataSensorElements.Fs.dbF_cwp = atof(vStringElements[12].c_str());
//		dataSensorElements.Fs.dbF_chwp1 = atof(vStringElements[13].c_str());
//		dataSensorElements.Fs.dbF_chwp2 = atof(vStringElements[14].c_str());
//		dataSensorElements.Ws.dbWTotal = atof(vStringElements[15].c_str());
//		dataSensorElements.Ws.dbWc = atof(vStringElements[16].c_str());
//		dataSensorElements.Ws.dbWt = atof(vStringElements[17].c_str());
//		dataSensorElements.Ws.dbWcwe = atof(vStringElements[18].c_str());
//		dataSensorElements.Ws.dbWchwp1 = atof(vStringElements[19].c_str());
//		dataSensorElements.Ws.dbWchwp2 = atof(vStringElements[20].c_str());
//		dataSensorElements.Ws.dbWTransducerCwp = atof(vStringElements[21].c_str());
//		dataSensorElements.Ws.dbWTransducerChwp1 = atof(vStringElements[22].c_str());
//		dataSensorElements.Ws.dbWTransducerChwp2 = atof(vStringElements[23].c_str());
//		dataSensorElements.dbPc = atof(vStringElements[24].c_str());
//		dataSensorElements.dbEffic = atof(vStringElements[25].c_str());
//		dataSensorElements.wPC_HistoryLenth = atoi(vStringElements[26].c_str());
//
//		double dPC_History = atof(vStringElements[27].c_str());
//		dataSensorElements.pPC_History.push_back(dPC_History);
//        ASSERT(false); //bird, don't know what meaning.
//
//		dataSensorElements.dPC_Keep_Average = atof(vStringElements[28].c_str());
//		dataSensorElements.dPC_Keep_Inertia = atof(vStringElements[29].c_str());
//		dataSensorElements.dPC_toCalc = atof(vStringElements[30].c_str());
//	}
//	else
//	{
//		TRACE("传入值少于预期,请检查传入值!  \r\n");
//		vStringElements.clear();
//	}
//
//	return dataSensorElements;
//}

// Package split.
//SplittedPackElement CDatasHandle::PackageSplitDataSensor_HashMap(const string& src, string main_delimit /*= "|"*/, string delimit /*= ";"*/, string subDelimit /*= ","*/)   
//{  
//	SplittedPackElement splittedPackElements;
//	vector<string> vStringElements;
//	vector<string> vStringSubElement;
//	vStringElements.clear();
//
//	Split(vStringElements, src, main_delimit);
//
//	if (vStringElements.size() >= 2)
//	{
//		splittedPackElements.dataSensor;/* = PackageSplitDataSensor(vStringElements[0], delimit);*/
//		splittedPackElements.hashmapValue = PackageSplitHashMap_Wstring(vStringElements[1], delimit, subDelimit);
//	}
//	else
//	{
//		TRACE("传入值少于预期,请检查传入值!  \r\n");
//	}
//
//	return splittedPackElements;
//}

// Package split.
hash_map<int, wstring> CDatasHandle::PackageSplitHashMap_Wstring(const string& src, string delimit /*= ";"*/, string subDelimit /*= ","*/)
{
	hash_map<int, wstring> hashmapElements;
	vector<string> vStringElements;
	vector<string> vStringSubElement;
	pair<int, wstring> pairElement;
	hashmapElements.clear();
	vStringElements.clear();

	Split(vStringElements, src, delimit);
	for (unsigned int i = 0; i < vStringElements.size() - 1; i++)
	{
		Split(vStringSubElement, vStringElements[i], subDelimit);
		if (vStringElements.size() >= 2)
		{
			string strFormat = vStringSubElement[1];
			wstring wstrFormat = m_toolsFunction.Ansi2WChar(strFormat.c_str(), strFormat.size());
			hashmapElements.insert(make_pair(atoi(vStringSubElement[0].c_str()), wstrFormat));
		}
		else
		{
			TRACE("Incoming values less than expected, check the incoming values!  \r\n");
			hashmapElements.clear();
		}

		vStringSubElement.clear();
	}

	return hashmapElements;
}

// Package split.
hash_map<int, bool> CDatasHandle::PackageSplitHashMap_bool(const string& src, string delimit /*= ";"*/, string subDelimit /*= ","*/)
{
	hash_map<int, bool> hashmapEqmStatus;
	vector<string> vStringElements;
	vector<string> vStringSubElement;
	pair<int, bool> pairElement;
	hashmapEqmStatus.clear();
	vStringElements.clear();

	Split(vStringElements, src, delimit);
	for (unsigned int i = 0; i < vStringElements.size() - 1; i++)
	{
		Split(vStringSubElement, vStringElements[i], subDelimit);
		if (vStringElements.size() >= 2)
		{
			string strFormat = vStringSubElement[1];
			if (strcmp(strFormat.c_str(), "1") == 0)
			{
				hashmapEqmStatus.insert(make_pair(atoi(vStringSubElement[0].c_str()), true));
			}
			else if (strcmp(strFormat.c_str(), "0") == 0) 
			{
				hashmapEqmStatus.insert(make_pair(atoi(vStringSubElement[0].c_str()), false));
			}
		}
		else
		{
			TRACE("Incoming values less than expected, check the incoming values!  \r\n");
			hashmapEqmStatus.clear();
		}

		vStringSubElement.clear();
	}

	return hashmapEqmStatus;
}
//
//// Get the equipments value array.
//vector<double> CDatasHandle::GetEquipmentsValueArray(const Data_Sensor& dataSensor)
//{
//	// Realtime update equipments value.
//	vector<double> vecEquipmentsValue;
//
//	// Struct Data_Sensor value.
// 	vecEquipmentsValue.push_back(dataSensor.dbT_chws0);
// 	vecEquipmentsValue.push_back(dataSensor.dbT_chwr0);
// 	vecEquipmentsValue.push_back(dataSensor.dbQ_chw0);
// 	vecEquipmentsValue.push_back(dataSensor.dbT_dbin);
// 	vecEquipmentsValue.push_back(dataSensor.dbRH);
// 	vecEquipmentsValue.push_back(dataSensor.dbT_wbin);
// 	vecEquipmentsValue.push_back(dataSensor.dbT_win);
// 	vecEquipmentsValue.push_back(dataSensor.dbT_wout);
// 	vecEquipmentsValue.push_back(dataSensor.dbFlow_Chw1);
// 	vecEquipmentsValue.push_back(dataSensor.dbFlow_Chw2);
// 	vecEquipmentsValue.push_back(dataSensor.dbFlow_CW);
// 	vecEquipmentsValue.push_back(dataSensor.dbChw2DP);
// 	vecEquipmentsValue.push_back(dataSensor.Fs.dbF_cwp);
//	vecEquipmentsValue.push_back(dataSensor.Fs.dbF_chwp1);
//	vecEquipmentsValue.push_back(dataSensor.Fs.dbF_chwp2);
// 	vecEquipmentsValue.push_back(dataSensor.Ws.dbWTotal);
// 	vecEquipmentsValue.push_back(dataSensor.Ws.dbWc);
// 	vecEquipmentsValue.push_back(dataSensor.Ws.dbWt);
// 	vecEquipmentsValue.push_back(dataSensor.Ws.dbWcwe);
// 	vecEquipmentsValue.push_back(dataSensor.Ws.dbWchwp1);
//	vecEquipmentsValue.push_back(dataSensor.Ws.dbWchwp2);
//	vecEquipmentsValue.push_back(dataSensor.Ws.dbWTransducerCwp);
//	vecEquipmentsValue.push_back(dataSensor.Ws.dbWTransducerChwp1);
//	vecEquipmentsValue.push_back(dataSensor.Ws.dbWTransducerChwp2);
//	vecEquipmentsValue.push_back(dataSensor.dbPc);
//	vecEquipmentsValue.push_back(dataSensor.dbEffic);
//	vecEquipmentsValue.push_back(dataSensor.wPC_HistoryLenth);
//    double dPC_History = 0.0;
//    if(dataSensor.pPC_History.size() )
//    {
//	     dPC_History = dataSensor.pPC_History[0];
//    }
//	vecEquipmentsValue.push_back(dPC_History);
//	vecEquipmentsValue.push_back(dataSensor.dPC_Keep_Average);
//	vecEquipmentsValue.push_back(dataSensor.dPC_Keep_Inertia);
//	vecEquipmentsValue.push_back(dataSensor.dPC_toCalc);
//
//	return vecEquipmentsValue;
//}

// Split function.
void CDatasHandle::Split(vector<string>& des, const string& src, string& delim)
{
	size_t nLast = 0;
	size_t nIndex = src.find_first_of(delim, nLast);     // get the first element

	while (nIndex != string::npos)
	{
		string subStr = src.substr(nLast, nIndex - nLast);
		des.push_back(subStr);
		nLast = nIndex + 1;
		nIndex = src.find_first_of(delim, nLast);
	}

	if (nIndex - nLast > 0)
	{
		des.push_back(src.substr(nLast, nIndex - nLast));
	}
}

// get the created package type
string CDatasHandle::GetPackageType() const 
{
	return m_strPackageType;
}

// set the created package type
void CDatasHandle::SetPackageType(const string& strType)   
{
	m_strPackageType = strType;
}

