/**
* @file update_state.h
* @brief 
* @author Jinwook Hong
* @version 
* @date 2013-06-03
*/

#ifndef _UPDATE_STATE_H_
#define _UPDATE_STATE_H_

/* wrong /system file tree */
#define WRONG_SYSTEM_TREE	-1

/* wrong /system/x.x file tree | mismatch between image files and md5sum files */
#define WRONG_UPDATE_TREE	-2

/* error open/read/write/rmdir/unlink/symlink */
#define ERROR_FILE_HANDLING	-3

/* error making md5sum value from image file */
#define ERROR_MD5_FROM_IMG	-4

/* mismatch between .md5 file's md5check value and image file's md5check value */
#define MISMATCH_MD5		-5

/* do not need update */
#define DO_NOT_UPDATE		0

/* success update but can not file remove (old version image files / UPDATE archives) */
#define UPDATE_SUCCESS		1

/* complete update process */
#define UPDATE_COMPLETE		2


#endif /* _UPDATE_STATE_H_ */
