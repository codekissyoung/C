#ifndef _CONF_H
#define _CONF_H

/*
 * initialize config
 */
int conf_init(const char *file);

/*
 * read a string value
 */
char *read_conf_str(const char *section, const char *name, char *ret, unsigned int len, const char *def_val);

/*
 * read a int value
 */
int read_conf_int(const char *section, const char *name, int *ret, const int def_val);

/*
 * read a float value
 */
float read_conf_float(const char *section, const char *name, float *ret, const float def_val);

/*
 * destroy
 */
int conf_uninit();

#endif
