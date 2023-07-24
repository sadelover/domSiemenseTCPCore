


/*
 *this class helps to make the application auto run while the computer restart.
 *
 *it will write the value to the regestry table, as well as delete the value.
 *
 *
 */


#pragma once



class CRestartAutoRun
{
public:
	static bool  SetAutoRun(const CString& exepath);
	static bool  RemoveAutoRun(const CString& exename);
};

