//gsoap ns service name: beopfunctions
//gsoap ns service namespace: http://localhost/beopfunctions.wsdl
//gsoap ns service location: http://localhost
//gsoap ns service executable: beopfunctions.cgi
//gsoap ns service encoding: encoded
//gsoap ns schema namespace: urn:beopfunctions


typedef std::wstring beop__string;


int beop_getvalue(beop__string strPointName, beop__string *strPointValue);
int beop_getvalue_multiple(beop__string strPointNameList, beop__string *strPointValueList);
int beop_userlogin( beop__string  userName, beop__string  userPwd, beop__string  *userInfo );
