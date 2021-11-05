#include <signal.h>
#include "event.h"
#include "buffer.h" 
#include "thread.h" 
#include "protocol.h" 
#include "server.h"
#include "conf.h"
#include "log.h"
#include <unistd.h>

extern char *optarg;
extern int optind, opterr, optopt;

struct setting setting;
static const char *pid_file = "./pid";

void io_thread_init(int nthreads);
void dispatch_io(int fd, int event_flag, enum io_cmd cmd, struct io_buff *buff);
void io_thread_stop();

static void set_non_block(int sock)
{
	int val = fcntl(sock, F_GETFL, 0);
	if (val == -1) {
		log_txt_err("fcntl[F_GETFL] error");
		exit(1);
	}
	if (fcntl(sock, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
		log_txt_err("fcntl[F_SETFL] error");
		exit(1);
	}
}

static int server_sock_init(char *ip, int port) 
{
	int sock;
	struct sockaddr_in server_addr;
    int enable = 1;

	/* create socket */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		log_txt_err("socket error");
		exit(1);
	}

    // set reuseaddr
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		log_txt_err("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family		= AF_INET;
    server_addr.sin_port		= htons(port);
	if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
		log_txt_err("inet_pton error");
		exit(1);
	}
	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		log_txt_err("bind error");
		exit(1);
	}
    if (listen(sock, setting.backlog) < 0) {
		log_txt_err("listen error");
		exit(1);
    }

	return sock;
}

static int server_accept(int server_sock)
{
	struct sockaddr addr;
	socklen_t len;

	memset(&addr, 0, sizeof(addr));
	len = sizeof(addr);

	return accept(server_sock, &addr, &len);
}

static void read_conf(char *conf_file)
{
	int ret = conf_init(conf_file);
	if (ret < 0) {
		fprintf(stderr, "read config error\n");
		exit(1);
	}
	read_conf_str("SERVER", "IP", setting.ip, sizeof(setting.ip), "192.168.1.101");
	read_conf_int("SERVER", "PORT", &setting.port, 9009);
	read_conf_int("SERVER", "BUFFER", &setting.buff_size, 1024);
	read_conf_int("SERVER", "MAX_CLIENT", &setting.max_client, 1024);
	read_conf_int("SERVER", "IO_THREAD", &setting.io_thread_num, 2);
	read_conf_int("SERVER", "PROC_THREAD", &setting.proc_thread_num, 4);
	read_conf_int("SERVER", "PROC_THREAD_ASYNC", &setting.proc_thread_async, 0);
	read_conf_int("SERVER", "BACKLOG", &setting.backlog, 5);
	read_conf_int("MODULE", "MODULE_COUNT", &setting.mod_num, 1);
	read_conf_str("LOG", "LOGPATH", setting.log_path, sizeof(setting.log_path), "../log/");
	read_conf_int("LOG", "LOGLEVEL", &setting.log_level, 5);
	read_conf_int("LOG", "LOGFILESIZE", &setting.log_file_size, 10);

	/* read module config */
	struct mod_info *mod = NULL;
	mod = (struct mod_info *)calloc(setting.mod_num, sizeof(struct mod_info)); 
	if (mod == NULL) {
		perror("calloc");
		exit(1);
	}
	setting.mod = mod;

	for (int i = 0; i < setting.mod_num; i++)
	{	
		char name[100];
		struct mod_info *mod_item = mod + i;

		sprintf(name, "MODULE_%d", i + 1);
		read_conf_str(name, "NAME", mod_item->mod_name, sizeof(mod_item->mod_name), "");
		read_conf_str(name, "FILE", mod_item->mod_file, sizeof(mod_item->mod_file), "");
		read_conf_str(name, "CONFIG", mod_item->mod_conf, sizeof(mod_item->mod_conf), "");
		read_conf_int(name, "COMMAND_BEGIN", (int *)&mod_item->cmd_begin, 0);
		read_conf_int(name, "COMMAND_END", (int *)&mod_item->cmd_end, 0);
		read_conf_int(name, "MAGIC", (int *)&mod_item->magic_code, 0);
		read_conf_str(name, "MODULE_INITIAL", mod_item->init_func_name, sizeof(mod_item->init_func_name), "");
		read_conf_str(name, "MODULE_PROCESS", mod_item->proc_func_name, sizeof(mod_item->proc_func_name), "");
		read_conf_str(name, "MODULE_DESTROY", mod_item->uninit_func_name, sizeof(mod_item->uninit_func_name), "");
	}

	/* destroy config */
	conf_uninit();
}

static void server_init(char *conf) {
	read_conf(conf);

	int i = 0;
	for (i = 0; i < setting.mod_num; i++) {
		struct mod_info *mod = setting.mod + i;

		/* load dynamics library */
		errno = 0;
		mod->mod_id = dlopen(mod->mod_file, RTLD_LAZY);
		if (mod->mod_id == NULL) {
			fprintf(stderr, "dlopen error, %s %s\n", mod->mod_file, dlerror());
			exit(1);
		}

		/* get function address */
		char *error = NULL;
		dlerror();
		/* get initial function */
		if (strlen(mod->init_func_name)) {
			mod->init_func = (int (*)(char *))dlsym(mod->mod_id, mod->init_func_name); 
			if ((error = dlerror()) != NULL) {
				fprintf(stderr, "dlsym error,func:%s msg:%s\n", mod->init_func_name, strerror(errno));
				exit(1);
			}
            int init_code = mod->init_func(mod->mod_conf) ;
			if ( init_code ) {
				fprintf(stderr, "module[%s] initialization error, code:%d\n", mod->mod_name,init_code);
				exit(1);
			}
		} else {
			mod->init_func = NULL;
		}

		/* get  process function*/
		if (strlen(mod->proc_func_name))
		{
			mod->proc_func = (int (*)(struct io_buff *))dlsym(mod->mod_id, mod->proc_func_name);
			if ((error = dlerror()) != NULL) {
				fprintf(stderr, "dlsym error\n");
				exit(1);
			}
		} else {
			mod->proc_func = NULL;
		}

		/* get destroy module function*/
		if (strlen(mod->uninit_func_name)) {
			mod->uninit_func = (int (*)())dlsym(mod->mod_id, mod->uninit_func_name); 
			if ((error = dlerror()) != NULL) {
				fprintf(stderr, "dlsym error\n");
				exit(1);
			}
		} else {
			mod->uninit_func = NULL;
		}
	}
}

static void server_uninit() {
	int i = 0;
	struct mod_info *mod;
	
	/* destroy modules */
	for (i = 0; i < setting.mod_num; i++) {
		 /* uninit module */
		 mod = setting.mod + i;
		 mod->uninit_func();
		 dlclose(mod->mod_id);

		 free(setting.mod + i);
	}

	log_txt_err("server destroy ok");
}

/*
 * accept a new connection
 */
static int accept_new(int server_sock, void *arg) {
	int client_sock;
	struct io_buff *buff;

	while(1) {
		client_sock = server_accept(server_sock);
		if (client_sock < 0) {
			if (errno == EAGAIN) {
				break;
			} else {
				log_txt_err("accept error");
				exit(0);
			}
		}
		
		/* get a free buffer from buffer list */
		buff = buff_from_freelist();
		if (buff != NULL) {
			/* process the connection */
			set_non_block(client_sock);
			dispatch_io(client_sock, EPOLLIN | EPOLLET, io_cmd_read, buff);
		} else {
			log_txt_err("buff is null when accept a new connection:%d", client_sock);
			close(client_sock);
		}
	}

	return 0;
}

/*
 *	get module by command number
 */
struct mod_info *get_mod(unsigned int cmd) {
	struct mod_info *mod = NULL;
	int i = 0;
	for (; i < setting.mod_num; i++) {
		mod = setting.mod + i;
		if (mod->cmd_begin <= cmd && mod->cmd_end >= cmd) {
			break;
		}

		mod = NULL;
	}

	return mod;
}

/*
 * save process id into file
 */
static void save_pid(pid_t pid) {
	FILE *fd;	
	
	if ((fd = fopen(pid_file, "w+")) == NULL) {
		log_txt_err("open process id file %s for write failed", pid_file);
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
		log_txt_err("remove process id file %s failed", pid_file);		
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
	sprintf(cmd, "ps -p %ld | grep %ld | wc -l", (long)pid, (long)pid);
	pfd = popen(cmd, "r");
	if (pfd == NULL) {
		fprintf(stderr, "popen failed\n");
		exit(1);
	}
	fread(result, 1, sizeof(result) - 1, pfd);
	pclose(pfd);
	return strtol(result, NULL, 10);
}

static void server_stop() {
	pid_t pid = get_pid();
	kill(pid, SIGUSR1);
	exit(0);
}

static void state() {
	pid_t pid = get_pid();
	kill(pid, SIGUSR2);
	exit(0);
}

static void stop_handle(int sig) {
	printf("stop called\n");

	/* stop I/O threads */
	io_thread_stop();

	/* stop business thread */
	thread_stop();

	/* release buffer */
	//buff_free();

	/* remove process id file */
	remove_pidfile();

	/* destroy modules */
	server_uninit();

	exit(0);
}

static void state_handle(int sig) {

}

static void parse_action(char *action) {
	if (strcmp(action, "start") == 0) {
		return;
	}
	else if (strcmp(action, "state") == 0) {
		state();
	}
	else if (strcmp(action, "stop") == 0) {
		server_stop();
	}
}

static int daemonize() {
	switch (fork())	{
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
	printf(PACKAGE " " VERSION "\n");		
	printf("-c <file> configure file, default read \"../conf/open-server.ini\" as configure file\n"
		   "-d          run as daemon\n"
		   "-h          print this help\n"
		   "start       start server\n"
		   "stop        stop server\n");

	exit(0);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
        usage();

    struct event_base *event_handle;
	int server_sock, c, daemon = 0;
	char conf_file[255], action[255];

	strcpy(conf_file, "./server.ini");
	
	while ((c = getopt(argc, argv, "c:dh?")) != -1)
	{
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

	parse_action(action);

	if (if_process_run())
    {
		fprintf(stdout, "the program is already running\n"); 
		exit(1);
	}

	/* initialize server setting */
	server_init(conf_file);

	/* make process to daemon process */
	if (daemon)
	{
		/* ignore SIGHUP */
		signal(SIGHUP, SIG_IGN);
		if (daemonize() < 0) {
			fprintf(stderr, "daemonize error\n");
			exit(1);
		}
	}

	/* initialize log */
	if (log_init(setting.log_level, setting.log_path, setting.log_file_size) < 0) {
		fprintf(stderr, "log_init error\n");
		exit(1);
	}
	
	signal(SIGUSR1, stop_handle);
	signal(SIGUSR2, state_handle);

	/* initialize free buffer list */
	freelist_init();
	/* initialize I/O threads */
	io_thread_init(setting.io_thread_num);
	/* initialize business threads */
	thread_init(setting.proc_thread_num, setting.proc_thread_async);

	/* create main thread's event handle */
	event_handle = event_init();

	/* create listen socket */
	server_sock  = server_sock_init(setting.ip, setting.port); 
	set_non_block(server_sock);

	/* save process id to file */
	if (daemon)
		save_pid(getpid());

	/* epoll event loop */
	event_add(event_handle, server_sock, EPOLLIN | EPOLLET, accept_new, NULL);
	event_loop(event_handle);
	return 0;
}
