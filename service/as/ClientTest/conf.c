#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"

/* config file content */
static char **conf_content = NULL;
/* line count */
static int line_count = 0;

/* trim str */
static int trim(char *str) {
    int len = strlen(str);
    char *p = str + strlen(str) - 1;

    /* trim tail space, \n, \r, \t */
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') {
        *p = 0x00;
        p--;
        len--;
    }

    /* trim header space,\t */
    p = str;
    while (*p == ' ' || *p == '\t') {
        p++;
        len--;
    }

    /* copy */
    memmove(str, p, len + 1);

    /* return length of str */
    return len;
}

/* 
 * calculate line count 
 * return -1 on failed
 */
static int cal_line(const char *path) {
    int cnt = 0;

    /* open file */
    FILE *fd = fopen(path, "r");
    if (fd == NULL) {
        return -1;
    }
    
    while (!feof(fd)) {
        char tmp[1024];
        
        /* read one line */
        if (fgets(tmp, sizeof(tmp) - 1, fd) == NULL) {
            continue;
        }

        /* ignore space line & comment */
        if (trim(tmp) == 0 || tmp[0] == '#') {
            continue;
        }

        cnt++;
    }
    
    /* close file */
    fclose(fd);

    return cnt;
}

/*
 * load config file
 */
static int load_conf(const char *path, int line_count) {
    int n = 0;

    /* open file */
    FILE *fd = fopen(path, "r");
    if (fd == NULL) {
        return -1;
    }

    while (!feof(fd)) {
        /* to store each line */
        char line[1024];
        memset(line, 0, sizeof(line));

        /* read one line */
        if (fgets(line, sizeof(line) - 1, fd) == NULL) {
            continue;
        }

        /* ignore space line & comment */
        if (trim(line) == 0 || line[0] == '#') {
            continue;
        }

        conf_content[n++] = strdup(line);

        if (n >= line_count) {
            break;
        }
    }

    /* close file */
    fclose(fd);

    return 0;
}

/* 
 * initialize config
 * return -1 when failed
 */
int conf_init(const char *path) {
    int cnt = 0;

    /* calculate line count */
    cnt = cal_line(path);
    if (cnt < 0) {
        return -1;
    }
    line_count = cnt;

    /* allocate memory */
    conf_content = (char **)malloc(cnt * sizeof(char *));
    if (conf_content == NULL) {
        return -1;
    }
    memset(conf_content, 0, cnt * sizeof(char *));

    /* load */
    if (load_conf(path, cnt) < 0) {
        return -1;
    }

    return 0;
}

/* destroy config */
int conf_uninit() {
    int i = 0;

    if (conf_content) {
        for (; i < line_count; i++) {
            free(conf_content[i]);
        }
        free(conf_content);
        conf_content = NULL;
    }
    
    return 0;
}

/* search line */
static int search_line(const char *section, const char *name, char *value, int len) {
    char line[1024];
    int n    = 0;
    int flag = 0;
    
    sprintf(line, "[%s]", section);
    for (n = 0; n < line_count; n++) {
        /* find section */
        if (strcmp(line, conf_content[n]) == 0) {
            flag = 1;
            continue;
        }
        
        /* not find, break */
        if (flag && conf_content[n][0] == '[') {
            break;
        }
        
        /* loop until file section */
        if (!flag)
            continue;
        
        /* find seprator */
        char *str = strchr(conf_content[n], '=');
        if (str == NULL)
            continue;
        
        /* get key */
        char tmp[1024];
        strncpy(tmp, conf_content[n], str - conf_content[n]);
        tmp[str - conf_content[n]] = 0x00;
        trim(tmp);

        /* check name */
        if (strcmp(name, tmp) == 0) {
            strcpy(tmp, str + 1);
            
            /* trim */
            trim(tmp);

            /* copy value */
            if (strlen(tmp) >= (size_t)len) {
                return -1;
            } else {
                strcpy(value, tmp);
                return 0;
            }
        } else {
            continue;
        }
    }

    return -1;
}

/* get int */
int read_conf_int(const char *section, const char *name, int *value, const int def_val) {
    char buff[1024];

    /* get value */
    if (search_line(section, name, buff, sizeof(buff)) < 0) {
        /* not find */
        *value = def_val;
    } else {
        *value = atoi(buff);
    }

    return *value;
}

/* get float */
float read_conf_float(const char *section, const char *name, float *value, const float def_val) {
    char buff[1024];

    /* get value */
    if (search_line(section, name, buff, sizeof(buff)) < 0) {
        /* not find */
        *value = def_val;
    } else {
        *value = atof(buff);
    }

    return *value;
}

/* get string */
char *read_conf_str(const char *section, const char *name, char *ret, unsigned int len, const char *def_val) {
    /* get value */
    if (search_line(section, name, ret, len) < 0) {
        /* not find */
        strcpy(ret, def_val);
    }

    return ret;
}
