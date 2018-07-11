#ifndef _LOG_H
#define _LOG_H

/* log级别，在记录log时会根据log级别，有选择性的记录 */
extern int log_level;

/* initialize log */
int log_init(int level, char *dir, int file_size);

/* write text log */
int log_write_txt(int level, const char *file, const int line, const char *func, const char *msg);

/* write binary log */
int log_write_bin(int level, const char *file, const int line, const char *func, char *bin, int len);

/* define log level */
#define LOG_LEVEL_NONE		0
#define LOG_LEVEL_ERR		1
#define LOG_LEVEL_WARNING	2
#define LOG_LEVEL_NOTICE	3
#define LOG_LEVEL_INFO		4
#define LOG_LEVEL_ALL		5

#define LOGMSG_MAX_LEN		4096

/* log message */
#define log_txt_info(fmt, args...) \
{	\
	if (LOG_LEVEL_INFO <= log_level) {	\
		char log_buf[LOGMSG_MAX_LEN];	\
		snprintf(log_buf, LOGMSG_MAX_LEN - 1, fmt, ##args);	\
		\
		log_write_txt(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, log_buf);	\
	}	\
}

/* log notice */
#define log_txt_notice(fmt, args...) \
{	\
	if (LOG_LEVEL_NOTICE <= log_level) {	\
		char log_buf[LOGMSG_MAX_LEN];	\
		snprintf(log_buf, LOGMSG_MAX_LEN - 1, fmt, ##args);	\
		\
		log_write_txt(LOG_LEVEL_NOTICE, __FILE__, __LINE__, __FUNCTION__, log_buf);	\
	}	\
}

/* log warning */
#define log_txt_warning(fmt, args...) \
{	\
	if (LOG_LEVEL_WARNING <= log_level) {	\
		char log_buf[LOGMSG_MAX_LEN];	\
		snprintf(log_buf, LOGMSG_MAX_LEN - 1, fmt, ##args);	\
		\
		log_write_txt(LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, log_buf);	\
	}	\
}

/* log error */
#define log_txt_err(fmt, args...) \
{	\
	if (LOG_LEVEL_ERR <= log_level) {	\
		char log_buf[LOGMSG_MAX_LEN];	\
		snprintf(log_buf, LOGMSG_MAX_LEN - 1, fmt, ##args);	\
		\
		log_write_txt(LOG_LEVEL_ERR, __FILE__, __LINE__, __FUNCTION__, log_buf);	\
	}	\
}

/********** binary log **********/
/* log message */
#define log_bin_info(msg, len) \
{	\
	if (LOG_LEVEL_INFO <= log_level) {	\
		log_write_bin(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, msg, len);	\
	}	\
}

/* log notice */
#define log_bin_notice(msg, len) \
{	\
	if (LOG_LEVEL_NOTICE <= log_level) {	\
		log_write_bin(LOG_LEVEL_NOTICE, __FILE__, __LINE__, __FUNCTION__, msg, len);	\
	}	\
}

/* log warning */
#define log_bin_warning(msg, len) \
{	\
	if (LOG_LEVEL_WARNING <= log_level) {	\
		log_write_bin(LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, msg, len);	\
	}	\
}

/* log error */
#define log_bin_err(msg, len) \
{	\
	if (LOG_LEVEL_ERR <= log_level) {	\
		log_write_bin(LOG_LEVEL_ERR, __FILE__, __LINE__, __FUNCTION__, msg, len);	\
	}	\
}

#endif
