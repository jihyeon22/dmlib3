#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dm/dm.h>
#include <dm/board_system.h>
#include <dm/constants.h>
#include <dm/safe.h>
#include <dm/version.h>

static dm_info *_dmi = NULL;

dm_res dm_global_init(void)
{
	if (!_dmi) {
		_dmi = calloc(1, sizeof(struct dm_info));
		if (!_dmi) return DM_FAIL;
	}

	/* Default Setting */
	_dmi->session.url = strdup(DM_URL);
	_dmi->session.path_event = strdup(PATH_EVENT);
	_dmi->session.timeout = DM_TIMEOUT;
	
	/* Curl global init */
	return dm_rest_init();
}

dm_res dm_global_shutdown(void)
{
	safefree(_dmi->session.url);
	safefree(_dmi->session.path_event);
	safefree(_dmi->version.hw);
	safefree(_dmi->version.amss);
	safefree(_dmi->version.linu);
	safefree(_dmi->package.name);
	safefree(_dmi->update_info);

	free(_dmi);
	_dmi = NULL;

	/* Curl global shutdown */
	return dm_rest_shutdown();
}

dm_info* dm_get_info(void)
{
	return _dmi;
}

dm_res dm_set_info(dm_info *dmi)
{
	dm_res res = DM_FAIL;
	if (!_dmi || !dmi)
		return res;
	
	if (memcpy(_dmi, dmi, sizeof(struct dm_info)) == 0)
		res = DM_OK;
	
	return res;
}

void dm_version_stamp(FILE *output)
{
	fprintf(output, "%s-%s\n", libdm_name, libdm_version);
}
