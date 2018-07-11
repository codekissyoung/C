// #include "conf.h"
// #include "log.h"
//
#include "company_match.h"
#include "conf.h"
#include "log.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool g_running = true;

static const char *pid_file = "./pid";

typedef struct {
  char log_path[260];
  char company_match_config[260];
  int log_level;
  int log_file_size;
} config_t;

static config_t g_config;

/*
 * initialize setting
 */
static void read_conf(char *conf_file) {

  /* initial config */
  int ret = conf_init(conf_file);
  if (ret < 0) {
    fprintf(stderr, "read config error\n");
    exit(1);
  }

  /* read server config */

  /* read log config */
  read_conf_str("LOG", "LOGPATH", g_config.log_path, sizeof(g_config.log_path),
                "../log/");
  read_conf_int("LOG", "LOGLEVEL", &g_config.log_level, 5);

  read_conf_int("LOG", "LOGFILESIZE", &g_config.log_file_size, 10);

  read_conf_str("COMPANY", "COMPANY_MATCH_CONFIG", g_config.company_match_config,
                sizeof(g_config.company_match_config), "company_match.ini");

  /* destroy config */
  conf_uninit();
}

/*
 * save process id into file
 */
static void save_pid(pid_t pid) {
  FILE *fd;

  if ((fd = fopen(pid_file, "w+")) == NULL) {
    printf("open process id file %s for write failed", pid_file);
    exit(0);
  }

  fprintf(fd, "%ld", (long)pid);
  fclose(fd);
}

static pid_t get_pid() {
  FILE *fd;
  char pid_buf[50];
  pid_t pid;

  if ((fd = fopen(pid_file, "r")) == NULL) {
    fprintf(stderr, "open process id file %s for read failed\n", pid_file);
    exit(0);
  }

  fread(pid_buf, 1, sizeof(pid_buf) - 1, fd);
  pid = (pid_t)atol(pid_buf);
  fclose(fd);

  return pid;
}

static void remove_pidfile() {
  if (unlink(pid_file) != 0) {
    printf("remove process id file %s failed", pid_file);
  }
}

static int if_process_run() {
  pid_t pid;
  FILE *pfd = NULL;
  char cmd[50] = {0}, result[20] = {0};

  /* if pid file exist */
  if (access(pid_file, F_OK) < 0)
    return 0;

  /* if process exist */
  pid = get_pid();
  sprintf(cmd, "ps -p %ld|grep %ld|wc -l", (long)pid, (long)pid);
  pfd = popen(cmd, "r");
  if (pfd == NULL) {
    fprintf(stderr, "popen failed\n");
    exit(1);
  }
  fread(result, 1, sizeof(result) - 1, pfd);

  pclose(pfd);

  return strtol(result, NULL, 10);
}

/*
 * stop server
 */
static void server_stop() {
  pid_t pid = get_pid();
  kill(pid, SIGUSR1);
  exit(0);
}

/*
 * get server status
 */
static void state() {
  pid_t pid = get_pid();
  kill(pid, SIGUSR2);
  exit(0);
}

/*
 * stop handle
 */
static void stop_handle(int /*sig*/) {
  printf("stop called\n");

  /* release buffer */
  // buff_free();

  /* remove process id file */
  remove_pidfile();

  /* destroy modules */
  g_running = false;
}

/*
 * state handle
 */
static void state_handle(int /*sig*/) {}

/*
 * parse action
 */
static void parse_action(char *action) {
  if (strcmp(action, "start") == 0) {
    return;
  } else if (strcmp(action, "state") == 0) {
    state();
  } else if (strcmp(action, "stop") == 0) {
    server_stop();
  }
}

/*
 * make process to daemon process
 */
static int daemonize() {
  switch (fork()) {
  case -1:
    printf("fork failed! msg=%s\n", strerror(errno));
    return -1;
  case 0:
    break;
  default:
    exit(0);
  }

  return 0;
}

void usage(void) {
  // printf(PACKAGE " " VERSION "\n");
  printf("-c <file> configure file\n"
         "-d          run as daemon\n"
         "-h          print this help\n"
         "start       start server\n"
         "stop        stop server\n");

  exit(0);
}

int main(int argc, char *argv[]) {
  int c, daemon = 0;
  char conf_file[255], action[255];

  /* if use crrect */
  if (argc == 1)
    usage();

  /* process arguments */
  while ((c = getopt(argc, argv, "c:dh?")) != -1) {
    switch (c) {
    case 'c':
      strcpy(conf_file, optarg);
      break;
    case 'd':
      daemon = 1;
      break;
    case 'h':
    case '?':
      usage();
      break;
    }
  }

  if (optind == argc) {
    usage();
  }
  strcpy(action, argv[optind]);

  /* parse action */
  parse_action(action);

  /* check if the program is running */
  if (if_process_run()) {
    fprintf(stdout, "the program is already running\n");
    exit(1);
  }

  /* make process to daemon process */
  if (daemon) {
    /* ignore SIGHUP */
    signal(SIGHUP, SIG_IGN);

    if (daemonize() < 0) {
      fprintf(stderr, "daemonize error\n");
      exit(1);
    }
  }
  read_conf(conf_file);

  /* initialize log */
  if (log_init(g_config.log_level, g_config.log_path, g_config.log_file_size) <
      0) {
    fprintf(stderr, "log_init error\n");
    exit(1);
  }

  signal(SIGUSR1, stop_handle);
  signal(SIGUSR2, state_handle);

  /* save process id to file */
  if (daemon)
    save_pid(getpid());

  int ret = company_match_init(g_config.company_match_config);
  if (ret) {
    fprintf(stderr, "company match init failed\n");
    exit(-1);
  }
  while (g_running) {
    ret = company_match_one();
    if (ret <= 0)
    {
      /*
      log_txt_info("no match, sleep 5");
      */
      sleep(5);
    }
  }
  return 0;
}
