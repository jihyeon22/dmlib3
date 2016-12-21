#ifndef _DM_CONFIG_H_
#define _DM_CONFIG_H_

dm_res dm_load_ini(dm_info *dmi, const char *file);
dm_res dm_save_ini(dm_info *dmi, const char *file);
dm_res dm_save_ini_dict(const char *key, const char *value, const char *file);

#endif
