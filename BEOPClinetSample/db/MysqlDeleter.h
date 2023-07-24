
#ifndef MYSQLDELETER_H__
#define MYSQLDELETER_H__



struct Mysql_Result_Deleter
{
	void operator()(MYSQL_RES* result)
	{
		if (result != NULL)
			mysql_free_result(result);
	}
};

#endif