#include <string>
using namespace std;
#define E_BEOPDB_VERSION	2.3

#define E_DROPDB_BEOP	"DROP DATABASE if exists beopdata"
#define E_CREATEDB_BEOP "CREATE DATABASE if not exists beopdata;"

#define E_DROPTB_BEOPINFO "DROP TABLE IF EXISTS beopdata.`beopinfo`;"
#define E_CREATETB_BEOPINFO "CREATE TABLE beopdata.`beopinfo` (\
							`infoname` varchar(45) NOT NULL DEFAULT '',\
							`incocontent` varchar(512) NOT NULL DEFAULT ''\
							) ENGINE=InnoDB DEFAULT CHARSET=utf8;"
#define E_INSERTTB_BEOPINFO	"INSERT INTO beopdata.`beopinfo` (`infoname`,`incocontent`) VALUES \
							('version','2.3');"


#define	E_DROPTB_LOG "DROP TABLE IF EXISTS beopdata.`log`;"
#define E_CREATETB_LOG "CREATE TABLE beopdata.`log` (\
							`time` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',\
							`loginfo` varchar(20000) NOT NULL DEFAULT ''\
							) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_DROPTB_OPERATION_RECORD "DROP TABLE IF EXISTS beopdata.`operation_record`;"
#define E_CREATETB_OPERATION_RECORD "CREATE TABLE beopdata.`operation_record` (\
									`RecordTime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
									`user` varchar(128) NOT NULL DEFAULT '',\
									`OptRemark` varchar(256) NOT NULL DEFAULT ''\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_POINT_VALUE_BUFFER		"DROP TABLE IF EXISTS beopdata.`point_value_buffer`;"
#define E_CREATETB_POINT_VALUE_BUFFER   "CREATE TABLE beopdata.`point_value_buffer` (\
											`time` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',\
											`pointname` varchar(255) NOT NULL DEFAULT '0',\
											`pointvalue` varchar(256) NOT NULL DEFAULT '0'\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_REALTIMEDATA_INPUT	"DROP TABLE IF EXISTS beopdata.`realtimedata_input`;"
#define E_CREATETB_REALTIMEDATA_INPUT	"CREATE TABLE beopdata.`realtimedata_input` (\
											`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
											`pointname` varchar(64) NOT NULL DEFAULT '',\
											`pointvalue` varchar(256) NOT NULL DEFAULT ''\
											) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_DROPTB_REALTIMEDATA_OUTPUT "DROP TABLE IF EXISTS beopdata.`realtimedata_output`;"
#define E_CREATETB_REALTIMEDATA_OUTPUT "CREATE TABLE beopdata.`realtimedata_output` (\
											`pointname` varchar(64) NOT NULL DEFAULT '',\
											`pointvalue` varchar(256) NOT NULL DEFAULT '',\
											PRIMARY KEY (`pointname`)\
											) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_DROPTB_UNIT01 "DROP TABLE IF EXISTS beopdata.`unit01`;"
#define E_CREATETB_UNIT01 "CREATE TABLE beopdata.`unit01` (\
								`unitproperty01` text,\
								`unitproperty02` text,\
								`unitproperty03` text,\
								`unitproperty04` text,\
								`unitproperty05` text,\
								`unitproperty06` text,\
								`unitproperty07` text,\
								`unitproperty08` text,\
								`unitproperty09` text,\
								`unitproperty10` text,\
								`unitproperty11` text,\
								`unitproperty12` text,\
								`unitproperty13` text,\
								`unitproperty14` text,\
								`unitproperty15` text\
								) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_UNIT02 "DROP TABLE IF EXISTS beopdata.`unit02`;"
#define E_CREATETB_UNIT02	"CREATE TABLE beopdata.`unit02` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_UNIT03 "DROP TABLE IF EXISTS beopdata.`unit03`;"
#define E_CREATETB_UNIT03	"CREATE TABLE beopdata.`unit03` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_DROPTB_UNIT04 "DROP TABLE IF EXISTS beopdata.`unit04`;"
#define E_CREATETB_UNIT04	"CREATE TABLE beopdata.`unit04` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_UNIT05 "DROP TABLE IF EXISTS beopdata.`unit05`;"
#define E_CREATETB_UNIT05	"CREATE TABLE beopdata.`unit05` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"
							
#define E_DROPTB_UNIT06 "DROP TABLE IF EXISTS beopdata.`unit06`;"
#define E_CREATETB_UNIT06	"CREATE TABLE beopdata.`unit06` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_UNIT07 "DROP TABLE IF EXISTS beopdata.`unit07`;"
#define E_CREATETB_UNIT07	"CREATE TABLE beopdata.`unit07` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_UNIT08 "DROP TABLE IF EXISTS beopdata.`unit08`;"
#define E_CREATETB_UNIT08	"CREATE TABLE beopdata.`unit08` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_UNIT09 "DROP TABLE IF EXISTS beopdata.`unit09`;"
#define E_CREATETB_UNIT09	"CREATE TABLE beopdata.`unit09` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_UNIT10 "DROP TABLE IF EXISTS beopdata.`unit10`;"
#define E_CREATETB_UNIT10		"CREATE TABLE beopdata.`unit10` (\
										`unitproperty01` text,\
										`unitproperty02` text,\
										`unitproperty03` text,\
										`unitproperty04` text,\
										`unitproperty05` text,\
										`unitproperty06` text,\
										`unitproperty07` text,\
										`unitproperty08` text,\
										`unitproperty09` text,\
										`unitproperty10` text,\
										`unitproperty11` text,\
										`unitproperty12` text,\
										`unitproperty13` text,\
										`unitproperty14` text,\
										`unitproperty15` text\
										) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_USERLIST_ONLINE	"DROP TABLE IF EXISTS beopdata.`userlist_online`;"
#define E_CREATETB_USERLIST_ONLINE	"CREATE TABLE beopdata.`userlist_online` (\
										`username` varchar(64) NOT NULL DEFAULT '',\
										`userhost` varchar(64) NOT NULL,\
										`priority` int(10) unsigned NOT NULL,\
										`usertype` varchar(64) NOT NULL,\
										`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
										PRIMARY KEY (`username`,`usertype`)\
										) ENGINE=MEMORY DEFAULT CHARSET=utf8;"
#define E_INSERTTB_USERLIST_ONLINE	"INSERT INTO beopdata.`userlist_online` (`username`,`userhost`,`priority`,`usertype`,`time`) VALUES\
										('operator','',0,'client','2014-01-06 11:21:46');"

#define E_DROPTB_WARNING_CONFIG	"DROP TABLE IF EXISTS beopdata.`warning_config`;"
#define E_CREATETB_WARNING_CONFIG	"CREATE TABLE beopdata.`warning_config` (\
										`HHEnable` int(10) unsigned DEFAULT NULL,\
										`HEnable` int(10) unsigned DEFAULT NULL,\
										`LEnable` int(10) unsigned DEFAULT NULL,\
										`LLEnable` int(10) unsigned DEFAULT NULL,\
										`HHLimit` double DEFAULT NULL,\
										`HLimit` double DEFAULT NULL,\
										`LLimit` double DEFAULT NULL,\
										`LLLimit` double DEFAULT NULL,\
										`pointname` varchar(255) NOT NULL DEFAULT '',\
										`HHwarninginfo` varchar(255) DEFAULT NULL,\
										`Hwarninginfo` varchar(255) DEFAULT NULL,\
										`Lwarninginfo` varchar(255) DEFAULT NULL,\
										`LLwarninginfo` varchar(255) DEFAULT NULL,\
										`boolwarning` int(11) DEFAULT NULL,\
										`boolwarninginfo` varchar(255) DEFAULT NULL,\
										`boolwarninglevel` int(11) DEFAULT NULL,\
										`unitproperty01` varchar(255) DEFAULT NULL,\
										`unitproperty02` varchar(255) DEFAULT NULL,\
										`unitproperty03` varchar(255) DEFAULT NULL,\
										`unitproperty04` varchar(255) DEFAULT NULL,\
										`unitproperty05` varchar(255) DEFAULT NULL\
										) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_INSERTTB_WARNING_CONFIG	"INSERT INTO beopdata.`warning_config` (`HHEnable`,`HEnable`,`LEnable`,`LLEnable`,`HHLimit`,`HLimit`,`LLimit`,`LLLimit`,`pointname`,`HHwarninginfo`,`Hwarninginfo`,`Lwarninginfo`,`LLwarninginfo`,`boolwarning`,`boolwarninginfo`,`boolwarninglevel`,`unitproperty01`,`unitproperty02`,`unitproperty03`,`unitproperty04`,`unitproperty05`) VALUES \
										(0,0,0,0,0,0,0,0,'ChErr01','','','','',1,'Chiller01 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr02','','','','',1,'Chiller02 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr03','','','','',1,'Chiller03 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr04','','','','',1,'Chiller04 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr05','','','','',1,'Chiller05 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr06','','','','',1,'Chiller06 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr07','','','','',1,'Chiller07 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr08','','','','',1,'Chiller08 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr09','','','','',1,'Chiller09 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr10','','','','',1,'Chiller10 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr11','','','','',1,'Chiller11 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr12','','','','',1,'Chiller12 ERROR1111',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr13','','','','',1,'Chiller13 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr14','','','','',1,'Chiller14 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr15','','','','',1,'Chiller15 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'ChErr16','','','','',1,'Chiller16 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr01','','','','',1,'Pri Chilled Pump01 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr02','','','','',1,'Pri Chilled Pump02 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr03','','','','',1,'Pri Chilled Pump03 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr04','','','','',1,'Pri Chilled Pump04 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr05','','','','',1,'Pri Chilled Pump05 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr06','','','','',1,'Pri Chilled Pump06 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr07','','','','',1,'Pri Chilled Pump07 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr08','','','','',1,'Pri Chilled Pump08 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr09','','','','',1,'Pri Chilled Pump09 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr10','','','','',1,'Pri Chilled Pump10 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr11','','','','',1,'Pri Chilled Pump11 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr12','','','','',1,'Pri Chilled Pump12 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr13','','','','',1,'Pri Chilled Pump13 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr14','','','','',1,'Pri Chilled Pump14 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr15','','','','',1,'Pri Chilled Pump15 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr16','','','','',1,'Pri Chilled Pump16 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr17','','','','',1,'Pri Chilled Pump17 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr18','','','','',1,'Pri Chilled Pump18 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr19','','','','',1,'Pri Chilled Pump19 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'PriChWPErr20','','','','',1,'Pri Chilled Pump20 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr01','','','','',1,'Sec Chilled Pump01 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr02','','','','',1,'Sec Chilled Pump02 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr03','','','','',1,'Sec Chilled Pump03 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr04','','','','',1,'Sec Chilled Pump04 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr05','','','','',1,'Sec Chilled Pump05 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr06','','','','',1,'Sec Chilled Pump06 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr07','','','','',1,'Sec Chilled Pump07 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr08','','','','',1,'Sec Chilled Pump08 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr09','','','','',1,'Sec Chilled Pump09 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr10','','','','',1,'Sec Chilled Pump10 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr11','','','','',1,'Sec Chilled Pump11 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr12','','','','',1,'Sec Chilled Pump12 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr13','','','','',1,'Sec Chilled Pump13 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr14','','','','',1,'Sec Chilled Pump14 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr15','','','','',1,'Sec Chilled Pump15 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr16','','','','',1,'Sec Chilled Pump16 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr17','','','','',1,'Sec Chilled Pump17 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr18','','','','',1,'Sec Chilled Pump18 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr19','','','','',1,'Sec Chilled Pump19 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'SecChWPErr20','','','','',1,'Sec Chilled Pump20 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr01','','','','',1,'Cool Water Pump01 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr02','','','','',1,'Cool Water Pump02 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr03','','','','',1,'Cool Water Pump03 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr04','','','','',1,'Cool Water Pump04 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr05','','','','',1,'Cool Water Pump05 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr06','','','','',1,'Cool Water Pump06 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr07','','','','',1,'Cool Water Pump07 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr08','','','','',1,'Cool Water Pump08 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr09','','','','',1,'Cool Water Pump09 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr10','','','','',1,'Cool Water Pump10 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr11','','','','',1,'Cool Water Pump11 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr12','','','','',1,'Cool Water Pump12 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr13','','','','',1,'Cool Water Pump13 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr14','','','','',1,'Cool Water Pump14 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr15','','','','',1,'Cool Water Pump15 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr16','','','','',1,'Cool Water Pump16 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr17','','','','',1,'Cool Water Pump17 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr18','','','','',1,'Cool Water Pump18 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr19','','','','',1,'Cool Water Pump19 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CWPErr20','','','','',1,'Cool Water Pump20 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr01','','','','',1,'Cooling Tower01 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr02','','','','',1,'Cooling Tower02 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr03','','','','',1,'Cooling Tower03 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr04','','','','',1,'Cooling Tower04 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr05','','','','',1,'Cooling Tower05 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr06','','','','',1,'Cooling Tower06 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr07','','','','',1,'Cooling Tower07 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr08','','','','',1,'Cooling Tower08 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr09','','','','',1,'Cooling Tower09 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr10','','','','',1,'Cooling Tower10 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr11','','','','',1,'Cooling Tower11 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr12','','','','',1,'Cooling Tower12 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr13','','','','',1,'Cooling Tower13 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr14','','','','',1,'Cooling Tower14 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr15','','','','',1,'Cooling Tower15 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr16','','','','',1,'Cooling Tower16 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr17','','','','',1,'Cooling Tower17 ERROR',3,'0','0','0','0','0'),\
										(0,0,0,0,0,0,0,0,'CTErr18','','','','',1,'Cooling Tower18 ERROR',3,'0','0','0','0','0');"

#define E_DROPTB_WARNING_RECORD	"DROP TABLE IF EXISTS beopdata.`warningrecord`;"
#define E_CREATETB_WARNING_RECORD	"CREATE TABLE beopdata.`warningrecord` (\
									`time` timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
									`code` int(10) unsigned NOT NULL DEFAULT '0',\
									`info` varchar(1024) NOT NULL DEFAULT '',\
									`level` int(10) unsigned NOT NULL DEFAULT '0',\
									`endtime` timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
									`confirmed` int(10) unsigned NOT NULL DEFAULT '0',\
									`confirmeduser` varchar(2000) NOT NULL DEFAULT '',\
									`bindpointname` varchar(255) DEFAULT NULL\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_FILESTORAGE	"DROP TABLE IF EXISTS `beopdata`.`filestorage`;"
#define E_CREATETB_FILESTORAGE		"CREATE TABLE  `beopdata`.`filestorage` (\
									`fileid` varchar(255) NOT NULL,\
									`filename` varchar(255) NOT NULL DEFAULT '',\
									`filedescription` varchar(255) DEFAULT NULL,\
									`fileupdatetime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
									`reserve01` varchar(255) DEFAULT NULL,\
									`reserve02` varchar(255) DEFAULT NULL,\
									`reserve03` varchar(255) DEFAULT NULL,\
									`reserve04` varchar(255) DEFAULT NULL,\
									`reserve05` varchar(255) DEFAULT NULL,\
									`fileblob` LONGBLOB DEFAULT NULL\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_BEOPUSER	"DROP TABLE IF EXISTS `beopdata`.`beopuser`;"
#define E_CREATETB_BEOPUSER		"CREATE TABLE  `beopdata`.`beopuser` (\
									`userid` varchar(255) DEFAULT NULL,\
										`username` varchar(255) DEFAULT NULL,\
										`userpwd` varchar(255) DEFAULT NULL,\
										`userfullname` varchar(255) DEFAULT NULL,\
										`usersex` varchar(255) DEFAULT NULL,\
										`usermobile` varchar(255) DEFAULT NULL,\
										`useremail` varchar(255) DEFAULT NULL,\
										`usercreatedt` varchar(255) DEFAULT NULL,\
										`userstatus` varchar(255) DEFAULT NULL,\
										`userpic` LONGBLOB DEFAULT NULL,\
										`userofrole` varchar(255) DEFAULT NULL,\
										`unitproperty01` varchar(255) DEFAULT NULL,\
										`unitproperty02` varchar(255) DEFAULT NULL,\
										`unitproperty03` varchar(255) DEFAULT NULL,\
										`unitproperty04` varchar(255) DEFAULT NULL,\
										`unitproperty05` varchar(255) DEFAULT NULL\
										) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_BEOPROLE	"DROP TABLE IF EXISTS `beopdata`.`beoprole`;"
#define E_CREATETB_BEOPROLE		"CREATE TABLE  `beopdata`.`beoprole` (\
								`roleid` varchar(255) DEFAULT NULL,\
									`rolename` varchar(255) DEFAULT NULL,\
									`roledesc` varchar(255) DEFAULT NULL,\
									`rolecreatedt` varchar(255) DEFAULT NULL,\
									`rolestatus` varchar(255) DEFAULT NULL,\
									`rolerightbasic` TEXT DEFAULT NULL,\
									`rolerightpage` TEXT DEFAULT NULL,\
									`rolerightwritepoint` TEXT DEFAULT NULL,\
									`roleright1` TEXT DEFAULT NULL,\
									`roleright2` TEXT DEFAULT NULL,\
									`roleright3` TEXT DEFAULT NULL,\
									`roleright4` TEXT DEFAULT NULL,\
									`roleright5` TEXT DEFAULT NULL,\
									`unitproperty01` varchar(255) DEFAULT NULL,\
									`unitproperty02` varchar(255) DEFAULT NULL,\
									`unitproperty03` varchar(255) DEFAULT NULL,\
									`unitproperty04` varchar(255) DEFAULT NULL,\
									`unitproperty05` varchar(255) DEFAULT NULL\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"