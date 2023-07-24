#pragma once

using namespace std;
class Equipment
{
public:
	Equipment(void);
	~Equipment(void);


	wstring m_wstrEquipmentName;

	wstring m_wstrEnableType;//0:const, 1:point
	wstring m_wstrEnableBindName;


};

