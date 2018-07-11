#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include "log.h"

/* log file path */
static char log_file_dir[255]  = {0};
static char log_file_path[255] = {0};

/* max size of log file */
static unsigned int log_file_size;

/* log level */
int log_level = 5;

/* 
 * initialize log 
 */
int log_init(int level, char *dir, int file_size) {
	log_level = level;
	strcpy(log_file_dir, dir);
	sprintf(log_file_path, "%s/_tmp.log", dir);
	log_file_size = file_size * 1024 * 1024;

	DIR *d = opendir(log_file_dir);
	if (d == NULL) {
		return -1;
	}
	closedir(d);

	return 0;
}

/*
 * check file size
 */
static void chek_log_file() {
	int ret;
	struct stat s;

	ret = lstat(log_file_path, &s);
	if (ret == 0 && s.st_size >= log_file_size) {
		char new_file[255];
		time_t t_now = time(NULL);
		struct tm t;		
		struct timeval tp;

		localtime_r(&t_now, &t);
		gettimeofday(&tp, NULL);

		sprintf(new_file, 
			"%s/%04d%02d%02d%02d%02d%02d%03d.log", 
			log_file_dir, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000));
		rename(log_file_path, new_file);

		unlink(log_file_path);
	}
}

/*
 * write text log
 */
int log_write_txt(int level, const char *file, const int line, const char *func, const char *msg) {
	char log[LOGMSG_MAX_LEN * 2];

	/* 记录时间的变量 */
	time_t t_now = time(NULL);
	struct tm t;
	struct timeval tp;
	
	/* 获取时间 */
	localtime_r((const time_t *)&t_now, &t);	
	gettimeofday(&tp, NULL);

	/* [年月日时分秒毫秒][文件名][行号][函数名][信息] */
	sprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%d][%s]%s\n", 
		t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
		file, line, func, msg);

	/* 检查日志文件大小 */
	chek_log_file();

	/* 写文件 */
	int fd = open(log_file_path, O_CREAT|O_APPEND|O_WRONLY, 0666);
	if (fd == -1)
	{
		return -1;
	}
	write(fd, log, strlen(log));
	close(fd);

	return 0;
}

/* 
 * write binary log 
 */
int log_write_bin(int level, const char *file, const int line, const char *func, char *bin, int len) {
	char log[LOGMSG_MAX_LEN * 2], binary[100];
	char *ptr = binary;
	int i = 0;

	/* 记录时间的变量 */
	time_t t_now = time(NULL);
	struct tm t;
	struct timeval tp;
	
	/* 获取时间 */
	localtime_r((const time_t *)&t_now, &t);	
	gettimeofday(&tp, NULL);

	/* [年月日时分秒毫秒][文件名][行号][函数名][信息] */
	sprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%d][%s]binary begin\n", 
		t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
		file, line, func);

	/* 检查日志文件大小 */
	chek_log_file();

	/* 写文件 */
	int fd = open(log_file_path, O_CREAT|O_APPEND|O_WRONLY, 0666);
	if (fd == -1)
	{
		return -1;
	}
	write(fd, log, strlen(log));

	ptr = binary;
	for (i = 0; i < len; i++) {
		if (i && i%16 == 0) /* 换行 */
		{
			sprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%d][%s]%s\n", 
				t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
				file, line, func, binary);
			write(fd, log, strlen(log));

			ptr = binary;
			*ptr = 0x00;
		}

		sprintf(ptr, "%02X ", (unsigned char)bin[i]);
		ptr += 3;
	}

	sprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%d][%s]binary end\n", 
		t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
		file, line, func);
	write(fd, log, strlen(log));

	close(fd);
	return 0;
}
