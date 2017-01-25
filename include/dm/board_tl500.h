#pragma once

#define CONCAT_STR(X, Y)		X Y
#define SYSTEM_DIR				"/system/mds/system"

#define DM_INI					CONCAT_STR(SYSTEM_DIR, "/bin/dm.ini")
#define DM_SERVICE_PACKAGE		CONCAT_STR(SYSTEM_DIR, "/bin/PACKAGE")


#define SVRFTP					CONCAT_STR(SYSTEM_DIR, "/UPDATE/svrftp.ini")
#define FTPSCRIPT				CONCAT_STR(SYSTEM_DIR, "/UPDATE/ftp_script.sh")

#define UPDATE_DIR				CONCAT_STR(SYSTEM_DIR, "/UPDATE")

#define TREE_ROOT				SYSTEM_DIR
#define DIR_NEW_PKG				CONCAT_STR(SYSTEM_DIR, "/NEW")

#define CMD_POWER_OFF			"poweroff &"

#define STEP_DEBUG_MDOE_1		"echo 1 > /tmp/stat"
#define STEP_DEBUG_MDOE_2		"echo 2 > /tmp/stat"
#define STEP_DEBUG_MDOE_3		"echo 3 > /tmp/stat"
#define STEP_DEBUG_MDOE_4		"echo 4 > /tmp/stat"
#define STEP_DEBUG_MDOE_5		"echo 5 > /tmp/stat"
#define STEP_DEBUG_MDOE_6		"echo 6 > /tmp/stat"



//util.c : source level 검토 필요 ++
#define DEVICE_MODEL			"w200k"
#define KT_CARRIOR_MODEL		"KT"
#define SKT_CARRIOR_MODEL		"SKT"
//util.c : source level 검토 필요 ++
