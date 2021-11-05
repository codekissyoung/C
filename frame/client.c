#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iconv.h>
#include <signal.h>
#include <errno.h>
#include "protocol.h"
#include <sys/types.h>

extern char *optarg;
extern int optind, opterr, optopt;

#define MAX_REQUEST_LEN 1024 * 512 

struct setting {
	char host[50];
	unsigned int port;
	unsigned int clients;
	unsigned int req_num;
	unsigned int req_len;
    int is_long_connection;
	int debug;
};
static struct setting setting;
static char request[MAX_REQUEST_LEN] = {0};
static int pipe_rslt[2] = {0};
static struct timeval time_start;
static struct timeval time_end;

static int conn_init(char *host, int port) {
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family			= AF_INET;
    servaddr.sin_port			= htons(port);
    servaddr.sin_addr.s_addr	= inet_addr(host);

	/* create socket */
	int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
		return -1;

	/* connect */
	if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		printf("connect error\n");
		close(sock);
		return  -1;
	}
	return sock;
}

/*
 * close connection
 */
static int conn_close(int sock) {
	close(sock);
	return 0;
}

/*
 * 	query
 */
static int query(char *ip, int port, char *buf, int buf_len, char *ret, int ret_buf_len, int *ret_len) {
	int sock, send_len = 0, recv_len = 0;
	int tmp = 0;
	
	/* create connection */
	sock = conn_init(ip, port);
	if (sock < 0) {
		return -1;
	}

	/* send */
	tmp = 0;
	while (1) {
		tmp = write(sock, buf + send_len, buf_len - send_len);
		if (tmp < 0) {
			perror("send");
			goto error;
		}

		send_len += tmp;

		if (send_len >= buf_len) {
			break;
		}
	}

	/* recv */
	recv_len = recv(sock, ret, 4, MSG_WAITALL);
	if (recv_len != 4) {
		//perror("recv1");
        fprintf(stderr, "recv1 failed, recv_len:%d msg:%s\n", recv_len, strerror(errno));
		goto error;
	}
	*ret_len = *(unsigned int *)ret;
	if (*ret_len > ret_buf_len) {
		printf("xiao\n");
		goto error;
	}
	while (1) {
		tmp = read(sock, ret + recv_len, *ret_len - recv_len);
		if (tmp < 0) {
			perror("recv2");
			goto error;
		}
		if (tmp + recv_len >= *ret_len) {
			break;
		}
		else {
			recv_len += tmp;
		}
	}

	/* close connection */
	conn_close(sock);

	return 0;

error:
	conn_close(sock);
	return -2;
}

static int long_query(int socket, char *buf, int buf_len, char *ret, int ret_buf_len, int *ret_len) {
	int send_len = 0, recv_len = 0;
	int tmp = 0;
	
	/* send */
	tmp = 0;
	while (1) {
		tmp = write(socket, buf + send_len, buf_len - send_len);
		if (tmp < 0) {
			perror("send");
			goto error;
		}

		send_len += tmp;

		if (send_len >= buf_len) {
			break;
		}
	}

	/* recv */
	recv_len = recv(socket, ret, 4, MSG_WAITALL);
	if (recv_len != 4) {
		//perror("recv1");
        fprintf(stderr, "recv1 failed, recv_len:%d msg:%s\n", recv_len, strerror(errno));
		goto error;
	}
	*ret_len = *(unsigned int *)ret;
	if (*ret_len > ret_buf_len) {
		printf("xiao\n");
		goto error;
	}
	while (1) {
		tmp = read(socket, ret + recv_len, *ret_len - recv_len);
		if (tmp < 0) {
			perror("recv2");
			goto error;
		}
		if (tmp + recv_len >= *ret_len) {
			break;
		}
		else {
			recv_len += tmp;
		}
	}

	return 0;

error:
	conn_close(socket);
	return -2;
}

/*
 *	caculate cost time
 */
static char *get_cost_time(struct timeval *time_end, struct timeval *time_start) {
	static char cost[1024] = {0};
	int sec = time_end->tv_sec - time_start->tv_sec;
	int msec= (int)(time_end->tv_usec - time_start->tv_usec) / 1000;	
	unsigned int cost_msec = sec * 1000 + msec;
	sec = 0;
	msec = 0;
	
#if 0	
	memset(cost, 0, sizeof(cost));
	sprintf(cost, "%u", cost_msec);
#endif	

	if (cost_msec > 1000)
		sec = cost_msec / 1000;	
	msec = cost_msec % 1000;

	sprintf(cost, "%ds %dms", sec, msec);
	return cost;
}

/* 
 * 	fork n process to request 
 */
static int bench(void) {
	int sock = 0; 
    unsigned i = 0;
	pid_t pid = 0; 

  	/* check avaibility of target server */
#if 1
	sock = conn_init(setting.host, setting.port);
	if (sock < 0) {
		printf("server not avaibility\n");
		return 1;
	}
	close(sock);
#endif

	/* create pipe */
  	if(pipe(pipe_rslt))
  	{
		perror("pipe failed.");
	  	return 2;
  	}


  	/* fork childs */
	for(i = 0; i < setting.clients; i++) {
		pid = fork();
		if (pid < (pid_t)0) {
	    	sleep(1); /* make childs faster */
			break;
		}
		if (pid == (pid_t)0) {
			break;
		}
	}
	if( pid < (pid_t) 0) {
		fprintf(stderr,"problems forking worker no. %d\n",i);
	  	perror("fork failed.");
	  	return 3;
	}
	if (pid > 0) {
		/* record start time */
		gettimeofday(&time_start, NULL);
	}

	/* if child */
	if (pid == 0) {
#if 0
		printf("child\n");
#endif
		close(pipe_rslt[0]);
		char buf[MAX_REQUEST_LEN] = {0};
		int ret_len, ret = 0;
        unsigned i = 0;
		struct timeval per_time_start;
		struct timeval per_time_end;
		int sec, ms;
		unsigned int client_cost = 0;

        int socket = 0;
        if (setting.is_long_connection) {
            socket = conn_init(setting.host, setting.port);
            if (socket < 0) {
                perror("connect");
                return -1;
            }
        }

		gettimeofday(&per_time_start, NULL);
		for (i = 0; i < setting.req_num; i++) {


            if (!setting.is_long_connection) {
                if ((ret = query(setting.host, setting.port, request, setting.req_len, buf, MAX_REQUEST_LEN, &ret_len)) < 0) {
                    //perror("query");
                    fprintf(stderr, "query error in process:%d, ret_code:%d\n", (int)getpid(), ret);
                }
            }
            else {
                if ((ret = long_query(socket, request, setting.req_len, buf, MAX_REQUEST_LEN, &ret_len)) < 0) {
                    fprintf(stderr, "query error in process:%d, ret_code:%d\n", (int)getpid(), ret);
                    socket = conn_init(setting.host, setting.port);
                }
            }

			if (setting.debug)
				printf("%s\n", buf + sizeof(pack_header));	
		}

		gettimeofday(&per_time_end, NULL);
		sec = (per_time_end.tv_sec - per_time_start.tv_sec);
		ms  = (int)(per_time_end.tv_usec - per_time_start.tv_usec) / 1000;  
		client_cost = (sec * 1000 + ms);

        if (setting.is_long_connection) {
            close(socket);
        }

#if 0
		printf("client_cost:%u, start_usec:%u, end_usec:%u\n", client_cost, per_time_start.tv_usec, per_time_end.tv_usec);
#endif

		write(pipe_rslt[1], &client_cost, sizeof(client_cost));
		close(pipe_rslt[1]);
	} else {
#if 0
		printf("pid:%d\n", (int)pid);
#endif

		/* already finish request count */
		unsigned int finish_cnt = 0;
		unsigned int all_cost = 0;

		/* close write fd */
		close(pipe_rslt[1]);

		/* wait for all childrens finish */
		while (1) {
			unsigned int cost = 0;
			if (read(pipe_rslt[0], &cost, sizeof(cost)) < 0) {
				fprintf(stderr, "some of our childrens died\n");
				break;
			}
			all_cost += cost;

			if (++finish_cnt == setting.clients)
				break;
		}
		close(pipe_rslt[0]);

		//printf("all cost:%u\n", all_cost);

		/* calculate time takon for test */
		gettimeofday(&time_end, NULL);
		int sec = time_end.tv_sec - time_start.tv_sec;
		int ms  = (time_end.tv_usec - time_start.tv_usec) / 1000;
		unsigned int test_time = sec * 1000 + ms;
		
		/* print result */
		unsigned int all_req = setting.clients * setting.req_num;
		printf("Request times:%u\n", all_req);
		printf("Time taken for tests:%s\n", get_cost_time(&time_end, &time_start));
		printf("Requests per second:%f[#/sec]\n", all_req * 1.0 / test_time * 1000);
		printf("Time per request:%f[ms]\n", all_cost * 1.0 / all_req);
		printf("Time per request:%f[ms](across all concurrent requests)\n", test_time * 1.0 / all_req);
	}

    return 0;
}

/*
 *	how to use this soft
 */
static void usage(char *argv[])
{
   fprintf(stderr,
			"Usage:%s [option]... host\n"
			"-p <port>	server port<port>.\n"
			"-c <n>		run <n> clients at once. default one.\n"
			"-n <n>		request <n> times per-clients.\n"
			"-l <byte>	request <byte> one time. default one.\n"
			"-d 		print response.\n"
			"-k 		long connection.\n"
			"-?|-h		this information.\n",
	argv[0]
	);
}

int main(int argc, char *argv[]) {

	if (argc == 1) {
		usage(argv);
		exit(0);
	}

	/* initial setting */
	memset(&setting, 0, sizeof(setting));
	setting.clients = 1;
	setting.req_num = 1;
	setting.req_len = 1;
	setting.port	= 9008;
	setting.is_long_connection = 0;
	setting.debug	= 1;

	/* get option arg */
	int opt;
	while((opt = getopt(argc, argv, "p:c:n:l:dkh?")) != -1) {
		switch (opt) {
			case 'p':
				setting.port = atoi(optarg);
				break;
			case 'c':
				setting.clients = atoi(optarg);
				break;
			case 'n':
				setting.req_num = atoi(optarg);
				break;
			case 'l':
				setting.req_len = atoi(optarg);
				if (setting.req_len < 1) {
					setting.req_len = 1; 
				}
				setting.req_len *= 1;
				break;
			case 'd':
				setting.debug = 1;
				break;
            case 'k':
                setting.is_long_connection = 1;
                break;
			case 'h':
			case '?':
				usage(argv);
				exit(0);
				break;
		}
	}
	if (optind == argc) {
		fprintf(stderr, "miss host\n");
		exit(0);
	}
	strcpy(setting.host, argv[optind]);

	/* build request */
	char *ptr = request;
	pack_header head;
	head.len = sizeof(head) + setting.req_len;
	head.magic = 1;
	head.cmd = 1;
	head.sequence = 1;
	head.state = 0;
	memcpy(ptr, &head, sizeof(head));
	memset(ptr + sizeof(head), 'a', setting.req_len);
	setting.req_len += sizeof(head);

	/* signal */
	signal(SIGCLD, SIG_IGN);

	/* bench */
	return bench();
}
