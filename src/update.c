#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dm/dm.h>
#include <dm/update.h>
#include <dm/update_api.h>

dm_res dm_update_ftp_download(const char *addr, int port,
		const char *id, const char *pw, const char *file)
{
	int ret = DM_FAIL;
	int res = 0;
	FTP_SERVER ftp_info;

	memset(&ftp_info, 0, sizeof(FTP_SERVER));
	strcpy(ftp_info.addr, addr);
	ftp_info.port = port;
	strcpy(ftp_info.id, id);
	strcpy(ftp_info.pass, pw);
	strcpy(ftp_info.file, file);

	res = ftpsvr_download(&ftp_info);
	if(res == success)
	{
		ret = DM_OK;
	}
	else
	{
		printf("[dmlib] %s : ftp download failture\n", __func__);
		ret = DM_FAIL;
	}
	return ret;
}
/*
dm_res dm_update_cmd(int vers, const char *file, const char *path)
{
	int ret = DM_FAIL;
	int res = 0;
	UPDATE_VERS update_data;
	update_data.version = UPDATE_VCMD;
	res = version_update(&update_data);
	if(res == success)
	{
		ret = DM_OK;
	}
	else
	{
		printf("[dmlib] %s : update process failture\n", __func__);
		ret = DM_FAIL;
	}
	return ret;
}
*/

dm_res dm_update(void)
{
	dm_res res = DM_FAIL;
	UPDATE_VERS update_data;
	update_data.version = UPDATE_VCMD;

	dm_info* dmi = dm_get_info();
	if (dmi && !dmi->update_info) {
		printf("[dmlib] %s : update info is not exist\n", __func__);
		return DM_FAIL;
	}
	
	res = dm_update_ftp_download(dmi->update_info->addr,
							  dmi->update_info->port,
							  dmi->update_info->id,
							  dmi->update_info->pw,
							  dmi->update_info->file);
	if (res == DM_OK) {
		res = version_update(&update_data);
		if(res == success)
		{
			res = DM_OK;
		}
		else
		{
			printf("[dmlib] %s : update process failture\n", __func__);
			res = DM_FAIL;
		}
	}
//		res = dm_update_cmd(1, "NONE", "NONE");

	return res;
}
