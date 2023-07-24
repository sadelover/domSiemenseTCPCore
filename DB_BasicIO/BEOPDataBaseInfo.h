#include <string>
using namespace std;

/*
	2.3->2.4
	´´½¨
		dtu_receive_buffer
		dtu_send_buffer
*/

#define E_BEOPDB_VERSION	2.3

#define E_DROPDB_BEOP	"DROP DATABASE if exists beopdata"
#define E_CREATEDB_BEOP "CREATE DATABASE if not exists beopdata;"


#define E_CREATEDB_DOM_LOG "CREATE DATABASE if not exists domlog;"

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
											`pointvalue` varchar(20000) NOT NULL DEFAULT '0'\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_REALTIMEDATA_INPUT	"DROP TABLE IF EXISTS beopdata.`realtimedata_input`;"
#define E_CREATETB_REALTIMEDATA_INPUT	"CREATE TABLE beopdata.`realtimedata_input` (\
											`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
											`pointname` varchar(64) NOT NULL DEFAULT '',\
											`pointvalue` varchar(20000) NOT NULL DEFAULT '',\
											PRIMARY KEY (`pointname`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_DROPTB_REALTIMEDATA_OUTPUT "DROP TABLE IF EXISTS beopdata.`realtimedata_output`;"
#define E_CREATETB_REALTIMEDATA_OUTPUT "CREATE TABLE beopdata.`realtimedata_output` (\
											`pointname` varchar(64) NOT NULL DEFAULT '',\
											`pointvalue` varchar(20000) NOT NULL DEFAULT '',\
											PRIMARY KEY (`pointname`)\
											) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_UPGRADE_REALTIMEDATA_OUTPUT "alter table beopdata.`realtimedata_output`  modify column pointvalue varchar(20000)"
#define E_UPGRADE_REALTIMEDATA_INPUT "alter table beopdata.`realtimedata_input`  modify column pointvalue varchar(20000)"

#define E_CREATETB_REALTIMEDATA_INPUT_MODBUS_EQUIPMENT	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_modbus_equipment` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_MODBUS_EQUIPMENT "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_modbus_equipment` (\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"


#define E_CREATETB_REALTIMEDATA_INPUT_PERSAGY_CONTROLLER	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_persagy_controller` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_PERSAGY_CONTROLLER "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_persagy_controller` (\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_INPUT_THIRDPARTY	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_thirdparty` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_THIRDPARTY "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_thirdparty` (\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
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

#define E_UPGRADE_WARNING_CONFIG "alter table beopdata.warning_config add column script varchar(2000), add column OfPosition varchar(255), add column  ofSystem varchar(255), \
										 add column ofDepartment varchar(255),  add column ofGroup varchar(255),  add column tag varchar(1024),  add column id INT;"
#define E_UPGRADE_WARNING_RECORD "alter table beopdata.warningrecord add column id INT, add column ruleId INT, \
                             add column OfPosition varchar(255), add column  ofSystem varchar(255), \
										 add column ofDepartment varchar(255),  add column ofGroup varchar(255),  add column tag varchar(1024);"
#define E_UPGRADE_WARNING_RECORD2 "alter table beopdata.warningrecord add column infodetail varchar(2000),  add column unitproperty01 varchar(1024),  add column unitproperty02 varchar(1024),  add column unitproperty03 varchar(1024),  add column unitproperty04 varchar(1024),  add column unitproperty05 varchar(1024);"
#define E_UPGRADE_WARNING_CONFIG2 "alter table beopdata.warning_config add column infodetail varchar(2000);"


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


#define E_DROPTB_BEOPUSER	"DROP TABLE IF EXISTS `beopdata`.`beopuser`;"
#define E_INITUSER_BEOPUSER		"insert into beopdata.`beopuser` (`userid`, `username`, `userpwd`, `userofrole`) values(0,'admin','111',3)"

#define E_DROPTB_DTU_SEND_BUFFER	"DROP TABLE IF EXISTS `beopdata`.`dtu_send_buffer`;"
#define E_CREATETB_DTU_SEND_BUFFER		"CREATE TABLE  `beopdata`.`dtu_send_buffer` (\
										`id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
											`time` datetime NOT NULL,\
											`filename` varchar(256) DEFAULT NULL,\
											`subtype` int(10) unsigned NOT NULL DEFAULT '0',\
											`trycount` int(10) unsigned NOT NULL DEFAULT '0',\
											`content` text,\
											PRIMARY KEY (`id`)\
											) ENGINE=InnoDB AUTO_INCREMENT=49 DEFAULT CHARSET=utf8;"

#define E_DROPTB_DTU_RECEIVE_BUFFER	"DROP TABLE IF EXISTS `beopdata`.`dtu_receive_buffer`;"
#define E_CREATETB_DTU_RECEIVE_BUFFER		"CREATE TABLE  `beopdata`.`dtu_receive_buffer` (\
											`id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
												`time` datetime NOT NULL,\
												`type` int(10) unsigned NOT NULL DEFAULT '0',\
												`content` text,\
												PRIMARY KEY (`id`)\
												) ENGINE=InnoDB AUTO_INCREMENT=49 DEFAULT CHARSET=utf8;"

#define E_DROPTB_DTU_UPDATE_BUFFER	"DROP TABLE IF EXISTS `beopdata`.`dtu_update_buffer`;"
#define E_CREATETB_DTU_UPDATE_BUFFER		"CREATE TABLE  `beopdata`.`dtu_update_buffer` (\
											`time` datetime NOT NULL,\
												`filename` varchar(255) DEFAULT NULL,\
												`cmdid` varchar(255) DEFAULT NULL\
												) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_INSERT_VALUE_UNIT01_DEFAULT		"INSERT INTO  `beopdata`.`unit01` (unitproperty01, unitproperty02) values('storehistory', '1')"




#define E_CREATETB_LOGIC_RECORD	"CREATE TABLE if not exists beopdata.`logic_output_point_record` (\
									`logicname` text,\
									`pointtime` text,\
									`pointname` text,\
									`pointvalue` text,\
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



#define E_CREATETB_REPORT	"CREATE TABLE if not exists beopdata.`report_history` (\
									`id` INT(10) unsigned NOT NULL,\
									`name` varchar(255),\
									`description` varchar(1024),\
									`gentime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
									`filesize` INT(10),\
									`author` varchar(255),\
									`url` varchar(655),\
									`reportTimeFrom` timestamp,\
									`reportTimeTo` timestamp,\
									`reportTimeType` INT(10),\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									PRIMARY KEY (`id`)\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_CREATETB_SYSLOGIC		"CREATE TABLE  if not exists `beopdata`.`sys_logic` (\
								`name` varchar(255) DEFAULT NULL,\
								`author` varchar(255) DEFAULT NULL,\
								`version` varchar(255) DEFAULT NULL,\
								`group_name` varchar(255) DEFAULT NULL,\
									`order_index` INT(10),\
									`description` varchar(255) DEFAULT NULL,\
									`param_input` TEXT DEFAULT NULL,\
									`param_output` TEXT DEFAULT NULL,\
									`unitproperty01` varchar(255) DEFAULT NULL,\
									`unitproperty02` varchar(255) DEFAULT NULL,\
									`unitproperty03` varchar(255) DEFAULT NULL,\
									`unitproperty04` varchar(255) DEFAULT NULL,\
									`unitproperty05` varchar(255) DEFAULT NULL\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_CREATETB_REALTIMEDATA_INPUT_OPCUA	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_opcua` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_OPCUA "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_opcua` (\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"



#define E_CREATETB_ENV	"CREATE TABLE IF NOT EXISTS beopdata.`env` (\
											`id` INT(10),\
											`type` INT(10),\
											`enabled` INT(10),\
											`name` varchar(255) NOT NULL DEFAULT 'unnamed',\
											`description` varchar(2550) NOT NULL DEFAULT '',\
											`tags` varchar(2550) NOT NULL DEFAULT '',\
											`creator` varchar(255) NOT NULL DEFAULT '',\
											`createtime`  timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
											PRIMARY KEY (`id`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_CREATETB_ENV_DETAIL	"CREATE TABLE IF NOT EXISTS beopdata.`env_detail` (\
											`envid` INT(10),\
											`pointname` varchar(64) NOT NULL DEFAULT '',\
											`pointvalue` varchar(20000) NOT NULL DEFAULT '',\
											PRIMARY KEY (`envid`, `pointname`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"




#define E_CREATETB_MODE	"CREATE TABLE IF NOT EXISTS beopdata.`mode` (\
											`id` INT(10),\
											`type` INT(10),\
											`enabled` INT(10),\
											`name` varchar(255) NOT NULL DEFAULT 'unnamed',\
											`description` varchar(2550) NOT NULL DEFAULT '',\
											`tags` varchar(2550) NOT NULL DEFAULT '',\
											`creator` varchar(255) NOT NULL DEFAULT '',\
											`createtime`  timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
											PRIMARY KEY (`id`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_CREATETB_MODE_DETAIL	"CREATE TABLE IF NOT EXISTS beopdata.`mode_detail` (\
											`modeid` INT(10),\
											`triggerTime` varchar(64) NOT NULL DEFAULT '',\
											`triggerTimeType` INT(10) NOT NULL DEFAULT 0,\
											`triggerTurn` INT(10) NOT NULL DEFAULT 0,\
											`triggerEnvId` INT(10) NOT NULL DEFAULT -1,\
											`triggerPointNameList` TEXT DEFAULT NULL,\
											`triggerValue` varchar(255) NOT NULL DEFAULT '' \
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_CREATETB_MODE_CALENDAR	"CREATE TABLE IF NOT EXISTS beopdata.`mode_calendar` (\
											`ofDate` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`modeid` INT(10) NOT NULL DEFAULT -1,\
											`creator` varchar(255) NOT NULL DEFAULT '',\
											`ofSystem` INT(10) NOT NULL DEFAULT -1,\
											PRIMARY KEY (`ofDate`, `modeId`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"



#define E_CREATETB_WEATHER	"CREATE TABLE IF NOT EXISTS beopdata.`weather_calendar` (\
											`ofDate` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`calendar` TEXT DEFAULT NULL,\
											`forcast` TEXT DEFAULT NULL,\
											`realtime` TEXT DEFAULT NULL,\
											`rawdata` TEXT DEFAULT NULL,\
											`raw_update_time` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`unitproperty01` TEXT DEFAULT NULL,\
											`unitproperty02` TEXT DEFAULT NULL,\
											`unitproperty03` TEXT DEFAULT NULL,\
											`unitproperty04` TEXT DEFAULT NULL,\
											`unitproperty05` TEXT DEFAULT NULL,\
											`unitproperty06` TEXT DEFAULT NULL,\
											`unitproperty07` TEXT DEFAULT NULL,\
											`unitproperty08` TEXT DEFAULT NULL,\
											`unitproperty09` TEXT DEFAULT NULL,\
											`unitproperty10` TEXT DEFAULT NULL,\
											PRIMARY KEY (`ofDate`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_CREATETB_REALTIMEDATA_INPUT_BACNET	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_bacnetpy` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_BACNET "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_bacnetpy` (\
`pointname` varchar(64) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"




#define E_CREATETB_FIX	"CREATE TABLE IF NOT EXISTS beopdata.`fix` (\
											`id` INT(10) NOT NULL DEFAULT 0,\
											`title` varchar(1024) NOT NULL DEFAULT '',\
											`reportTime` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`importance` INT(10),\
											`urgent` INT(10),\
											`content` TEXT DEFAULT NULL,\
											`result` INT(10),\
											`closeTime` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`attachments` varchar(1024) NOT NULL DEFAULT '',\
											`ofEquipments` varchar(1024) NOT NULL DEFAULT '',\
											`reportUser` varchar(255) NOT NULL DEFAULT '',\
											`solveUser` varchar(1024) NOT NULL DEFAULT '',\
											`specy` varchar(1024) NOT NULL DEFAULT '',\
											`tags` varchar(1024) NOT NULL DEFAULT '',\
											`energyEffects` INT(10),\
											`unitproperty01` TEXT DEFAULT NULL,\
											`unitproperty02` TEXT DEFAULT NULL,\
											`unitproperty03` TEXT DEFAULT NULL,\
											`unitproperty04` TEXT DEFAULT NULL,\
											`unitproperty05` TEXT DEFAULT NULL,\
											`unitproperty06` TEXT DEFAULT NULL,\
											`unitproperty07` TEXT DEFAULT NULL,\
											`unitproperty08` TEXT DEFAULT NULL,\
											`unitproperty09` TEXT DEFAULT NULL,\
											`unitproperty10` TEXT DEFAULT NULL,\
											PRIMARY KEY (`id`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"



#define E_CREATETB_PAGE_CONTAIN_FIX	"CREATE TABLE IF NOT EXISTS beopdata.`page_contain_fix` (\
											`pageId` VARCHAR(255) NOT NULL DEFAULT '',\
											`fixId` INT(10) NOT NULL DEFAULT 0,\
											`posX` INT(10),\
											`posY` INT(10),\
											`visible` INT(10),\
											`unitproperty01` TEXT DEFAULT NULL,\
											`unitproperty02` TEXT DEFAULT NULL,\
											`unitproperty03` TEXT DEFAULT NULL,\
											`unitproperty04` TEXT DEFAULT NULL,\
											`unitproperty05` TEXT DEFAULT NULL,\
											PRIMARY KEY (`pageId`, `fixId`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"