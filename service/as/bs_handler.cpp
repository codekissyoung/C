#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdarg.h>
#include <inttypes.h>
#include "log.h"
#include "buffer.h"
#include "interface.h"
#include "conf.h"
#include "bs_handler.h"

// set file description to non-block
static int _set_non_block(int sock) 
{
    int val = fcntl(sock, F_GETFL, 0);
    if (val == -1) {
        log_txt_err("fcntl[F_GETFL] error, fd:[%d]", sock);
        return -1;
    }
    if (fcntl(sock, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
        log_txt_err("fcntl[F_SETFL] error, fd:[%d]", sock);
        return -1;
    }

    return 0;
}

static int _bs_connect(char *host, short port)
{
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        log_txt_err("create socket failed, cmd:[socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)]");
        return -1;
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family			= AF_INET;
    servaddr.sin_port			= htons(port);
    servaddr.sin_addr.s_addr	= inet_addr(host);
	
	if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		close(sock);
		return  -1;
	}

    if (_set_non_block(sock) < 0) {
        log_txt_err("set socket[%d] non-block failed", sock);
        close(sock);
        return -1;
    }

    return sock;
}

static size_t _max_val(int count, ...)
{
    size_t max = 0;
    size_t tmp = 0;
    va_list arg_ptr;

    va_start(arg_ptr, count);
    for (int i = 0; i < count; i++) {
        tmp = va_arg(arg_ptr, size_t);
        if (tmp > max) {
            max = tmp;
        }
    }
    va_end(arg_ptr);

    return max;
}

// 初始化bs，每个线程都会调用一次
bs_service_t *bs_init(bs_group_conf_t *bs_conf, int bs_group_num)
{
    bs_service_t *bsc = (bs_service_t *)calloc(1, sizeof(bs_service_t));

    bsc->_bs_group_num = bs_group_num;
    bsc->_bs_svr = (bs_svr_t *)calloc(bs_group_num, sizeof(bs_svr_t));
    if (bsc->_bs_svr == NULL) {
        log_txt_err("calloc failed! size:[%d]", (int)(bs_group_num * sizeof(bs_svr_t)));
        return NULL;
    }

    int bs_node_num = 0;
    for (int i = 0; i < bs_group_num; i++) {
        bsc->_bs_svr[i]._cur_idx = -1;

        // 分配发送与接收的buffer
        size_t len = _max_val(3, sizeof(as2bs_get_post_t), 
                              sizeof(as2bs_set_post_t), 
                              sizeof(as2bs_get_post_cnt_t));
        bsc->_bs_svr[i]._send_buff = (req_pack_t *)malloc(len);
        bsc->_bs_svr[i]._send_buff_size = len;

        //len = sizeof(bs2as_get_post_t) + sizeof(uint64_t) * MAX_RET_POST_NUM;
        len = _max_val(3, sizeof(bs2as_get_post_t), sizeof(bs2as_set_post_t), sizeof(bs2as_get_post_cnt_t));
        bsc->_bs_svr[i]._recv_buff = (rsp_pack_t *)malloc(len);
        bsc->_bs_svr[i]._recv_buff_size = len;

        bsc->_bs_svr[i]._lft_len = 0;

        // 与bs建立连接
        memcpy(&(bsc->_bs_svr[i]._conf), bs_conf + i, sizeof(bs_group_conf_t));
        bs_group_conf_t *conf = bs_conf + i;
        for (int j = 0; j < conf->_bak_num; j++) {
            bsc->_bs_svr[i]._socket[j] = _bs_connect((conf->_conf)[j]._host, (conf->_conf)[j]._port);
            if (bsc->_bs_svr[i]._socket[j] < 0) {
                log_txt_err("connect bs failed, host[%s] port[%d]", (conf->_conf)[i]._host, (conf->_conf)[i]._port);
                bsc->_bs_svr[i]._health_flag[j] = -1;
                continue;
            }
            else if (bsc->_bs_svr[i]._cur_idx == -1) {
                bsc->_bs_svr[i]._cur_idx = j;
            }

            bs_node_num ++;
            bsc->_bs_svr[i]._health_flag[j] = 0;
        }
    }

    // 初始化epoll相关变量
    bsc->_epoll_events = (struct epoll_event *)calloc(bs_node_num, sizeof(struct epoll_event));
    bsc->_epoll_fd = epoll_create(bs_node_num);
    if (bsc->_epoll_fd < 0) {
        log_txt_err("epool_create failed, fd num:[%d]", bs_node_num);
        return NULL;
    }

    return bsc;
}

static inline bs_svr_t *_get_bs_svr(bs_service_t *bsc, uint64_t sharding_key)
{
    if (bsc == NULL)
        return NULL;
    bs_svr_t *bs = NULL;

    int group_idx = sharding_key % bsc->_bs_group_num;
    bs = bsc->_bs_svr + group_idx;
    if (bs == NULL) {
        log_txt_err("bs_svr invalid idx:[%d]", group_idx);
        return NULL;
    }

    // 如果所有可用节点都 Down 了, 则尝试重连.
    if (bs->_cur_idx == -1) {
        log_txt_err("all server was down. Trying to reconnect them.");
        
		bs_group_conf_t *conf = & (bs->_conf) ;

		while ( bs->_cur_idx == -1) {
			for (int j = 0; j < conf->_bak_num; j++) {
				bs->_socket[j] = _bs_connect((conf->_conf)[j]._host, (conf->_conf)[j]._port);
				if (bs->_socket[j] < 0) {
					log_txt_err("reconnet bs failed: host[%s] port[%d]", (conf->_conf)[j]._host, (conf->_conf)[j]._port);
					bs->_health_flag[j] = -1;
					continue;
				}
				else if (bs->_cur_idx == -1) {
					bs->_cur_idx = j;
				}
				bs->_health_flag[j] = 0 ;
			}

			/* 2000 ms 进行一次全重连. */
			timespec time, remainder ;
			time.tv_sec  = 2 ;
			time.tv_nsec = 0 ;
			while( nanosleep(&time, &remainder) == -1 )
				time = remainder ;
		}
	}
    return bs;
}

static void _deal_with_lost(bs_service_t *bsc, bs_svr_t *bs)
{
    close(bs->_socket[bs->_cur_idx]);

    bs->_health_flag[bs->_cur_idx] = -1;
    if (bs->_cur_idx + 1 < (bs->_conf)._bak_num) {
        bs->_cur_idx ++;
    }
    else { // 所有节点都已经出错，则尝试重新建立连接
        bs->_cur_idx = -1;

        // 与bs建立连接
        bs_group_conf_t *conf = & (bs->_conf) ;
        for (int j = 0; j < conf->_bak_num; j++) {
            bs->_socket[j] = _bs_connect((conf->_conf)[j]._host, (conf->_conf)[j]._port);
            if (bs->_socket[j] < 0) {
                log_txt_err("reconnect bs failed, host[%s] port[%d]", (conf->_conf)[j]._host, (conf->_conf)[j]._port);
                bs->_health_flag[j] = -1;
                continue;
            }
            else if (bs->_cur_idx == -1) {
                bs->_cur_idx = j;
            }
			bs->_health_flag[j] = 0 ;
        }
    }
}

static int _bs_write(bs_service_t *bsc, bs_svr_t *bs)
{
    int ret = 0;
    struct epoll_event epoll_et;

    if (bs->_lft_len <= 0) {
        log_txt_err("nothing to be sent, fd:[%d]", bs->_socket[bs->_cur_idx]);
        return 0;
    }

    while (1) {

        ret = write(bs->_socket[bs->_cur_idx], bs->_send_buff, bs->_lft_len);

        if (ret < 0) {
            if (errno == EAGAIN) 
			{
					log_txt_err("EAGAIN.......") ;
					break;
			}
            else { /* real error */
                log_txt_err("write error, socket:[%d] host:[%s] port:[%d] msg:[%s]", 
                        bs->_socket[bs->_cur_idx], (bs->_conf)._conf[bs->_cur_idx]._host, 
                        (bs->_conf)._conf[bs->_cur_idx]._port, strerror(errno));

                _deal_with_lost(bsc, bs);
                return -1;
            }
        }

        // 发送完毕
        bs->_lft_len -= ret;
        if (bs->_lft_len <= 0) {
            log_txt_info("send finished, socket:[%d] host:[%s] port:[%d] send size:%d", 
                    bs->_socket[bs->_cur_idx], (bs->_conf)._conf[bs->_cur_idx]._host, 
                    (bs->_conf)._conf[bs->_cur_idx]._port,ret);

            bs->_lft_len = 0;
            bs->_read_len = 0;

            epoll_et.data.ptr = bs;
            epoll_et.events  = EPOLLIN | EPOLLET;
            if (epoll_ctl(bsc->_epoll_fd, EPOLL_CTL_MOD, bs->_socket[bs->_cur_idx], &epoll_et) < 0) {
                log_txt_err("mod epoll event failed, fd:[%d] event:[EPOLLOUT | EPOLLET]", bs->_socket[bs->_cur_idx]);

                epoll_et.data.ptr = bs;
                epoll_et.events  = EPOLLOUT | EPOLLET;
                epoll_ctl(bsc->_epoll_fd, EPOLL_CTL_DEL, bs->_socket[bs->_cur_idx], &epoll_et);
                return -1;
            }
			
			break ;
        }
    }

    return 0;
}

static int _bs_read(bs_service_t *bsc, bs_svr_t *bs)
{
    int ret = 0;
    int read_len = 1024;
    char *ptr = (char *)(bs->_recv_buff) + bs->_read_len;
    struct epoll_event epoll_et;

    while (1) {
        ret = read(bs->_socket[bs->_cur_idx], ptr, read_len);

        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else if (errno == EINTR)
                continue;
            else { // real error
                log_txt_err("read error, socket:[%d] host:[%s] port:[%d] msg:[%s]", 
                        bs->_socket[bs->_cur_idx], (bs->_conf)._conf[bs->_cur_idx]._host, 
                        (bs->_conf)._conf[bs->_cur_idx]._port, strerror(errno));

                _deal_with_lost(bsc, bs);
                return -1;
            }
        }
        else if (ret == 0) {
            log_txt_err("read empty, socket:[%d] host:[%s] port:[%d] msg:[%s]", 
                    bs->_socket[bs->_cur_idx], (bs->_conf)._conf[bs->_cur_idx]._host, 
                    (bs->_conf)._conf[bs->_cur_idx]._port, strerror(errno));

            _deal_with_lost(bsc, bs);
            return -1;
        }

        ptr += ret;
        bs->_read_len += ret;

        if (bs->_lft_len == 0) {
            if (ret < 4) {
                continue;
            }

            bs->_lft_len = bs->_recv_buff->_header.len - ret;
        }
        else {
            bs->_lft_len -= ret;
        }

        // read finished
        if (bs->_lft_len <= 0) {
            epoll_et.data.ptr = bs;
            epoll_et.events  = EPOLLOUT | EPOLLET;
            epoll_ctl(bsc->_epoll_fd, EPOLL_CTL_DEL, bs->_socket[bs->_cur_idx], &epoll_et);
            break;
        }
    }

    return 0;
}

static int _multi_io(bs_service_t *bsc, int (*ignore)(bs_svr_t *), int max_time = 300)
{
    int i = 0;
    bs_svr_t *bs = NULL;
    int event_fd_num = 0;
    struct epoll_event epoll_et;

    // 添加到epoll
    for (i = 0; i < bsc->_bs_group_num; i++) {
        bs = (bsc->_bs_svr) + i ;
        if (!bs || ignore(bs)) {
            continue;
        }

        epoll_et.data.ptr = bs;
        epoll_et.events  = EPOLLOUT | EPOLLET;
        if (epoll_ctl(bsc->_epoll_fd, EPOLL_CTL_ADD, bs->_socket[bs->_cur_idx], &epoll_et) < 0) {

			/* 如果报错且错误类型不是 File Exists, 说明添加失败. */

			if ( errno != 17) {
				log_txt_err("add fd[%d] to epoll failed, event[EPOLLOUT | EPOLLET], errno=%d", 
								bs->_socket[bs->_cur_idx],errno);
				continue;
			}
        }

        log_txt_info("begin one bs request, request-length:%d pack-sequence:%d", bs->_lft_len, ((as2bs_get_post_t *)bs->_send_buff)->_header.sequence);
        event_fd_num++;
    }

    // 记录开始时间
    struct timeval ts, te;
    gettimeofday(&ts, NULL);
    int ms = 0; // 耗时ms 

    // 读写网络数据
    int et_cnt = 0;
    int fin_lft = event_fd_num;
    int ret = 0;
    while (fin_lft > 0) {
        et_cnt = epoll_wait(bsc->_epoll_fd, bsc->_epoll_events, event_fd_num, 1000);
        if (et_cnt == -1) {
            continue;
        }

        for (i = 0; i < et_cnt; i++) {
            bs = (bs_svr_t *)bsc->_epoll_events[i].data.ptr ;
            if (bsc->_epoll_events[i].events & EPOLLOUT) {
		
                // write
                ret = _bs_write(bsc, bs);
                if (ret < 0) {
                    bs->_failed = 1;
                    fin_lft--;
                }
            }
            else if (bsc->_epoll_events[i].events & EPOLLIN) {
                // read
                ret = _bs_read(bsc, bs);
                if (ret < 0) {
                    bs->_failed = 1;
                    fin_lft--;
                }
                else if (bs->_lft_len <= 0) {
                    log_txt_info("finished one bs request, return-length:%d pack-sequence:%d length:%d cmd:%d status:%d", bs->_read_len, 
                                 ((bs2as_get_post_t *)bs->_recv_buff)->_header.sequence,
                                 ((bs2as_get_post_t *)bs->_recv_buff)->_header.len,
                                 ((bs2as_get_post_t *)bs->_recv_buff)->_header.cmd,
                                 ((bs2as_get_post_t *)bs->_recv_buff)->_header.state);
                    fin_lft--;
                }
            }
        }

        // 判断是否超时
        gettimeofday(&te, NULL);
        ms = (te.tv_sec - ts.tv_sec) * 1000 + (int)(te.tv_usec - ts.tv_usec) / 1000;
        if (ms > max_time) {
            for (i = 0; i < bsc->_bs_group_num; i++) {
                bs = (bsc->_bs_svr) + i ;
                if (ignore(bs)) {
                    continue;
                }

                if (bs->_lft_len <= 0) {
                    continue;
                }

                log_txt_notice("request bs[%d] timeout, %d ms", i, ms);
        
                /* 对于超时的节点，需要重新连接，为了防止下次请求的时候有无效数据 */
                int j = bs->_cur_idx;
                // 从epoll中移除
                epoll_ctl(bsc->_epoll_fd, EPOLL_CTL_DEL, bs->_socket[j], &epoll_et);
                // 关闭连接
                close(bs->_socket[j]);
                // 重新建立连接
                bs->_socket[j] = _bs_connect((bs->_conf)._conf[j]._host, (bs->_conf)._conf[j]._port);
                if (bs->_socket[j] < 0) {
                    log_txt_err("reconnet bs failed, host[%s] port[%d], when overtime reconnect", 
                                (bs->_conf)._conf[i]._host, (bs->_conf)._conf[i]._port);

                    _deal_with_lost(bsc, bs);
                    continue;
                }
            }

            break;
        }
    }

    return 0;
}

static int _ignore_get(bs_svr_t *bs)
{
    if (((as2bs_get_post_t *)(bs->_send_buff))->_user_num == 0) {
        return 1 ;
    }

    return 0;
}

static int _ignore_set(bs_svr_t *bs)
{
    if (((as2bs_set_post_t *)(bs->_send_buff))->_pair_cnt == 0) {
        return 1 ;
    }

    return 0;
}

static int _ignore_get_cnt(bs_svr_t *bs)
{
    if (((as2bs_get_post_cnt_t *)(bs->_send_buff))->_user_num == 0) {
        return 1 ;
    }

    return 0;
}

// 获取post list
int bs_get_post(bs_service_t *bsc, bs_get_post_req_t *req, bs_get_post_rsp_t *rsp, int max_time)
{
    if (bsc == NULL || req == NULL || rsp == NULL) {
        return -1;
    }

    static unsigned pack_sequece = 0;

    int i = 0;
    bs_svr_t *bs = NULL;
    as2bs_get_post_t *req_pack = NULL;
    bs2as_get_post_t *rsp_pack = NULL;

    // reset各组bs
    for (i = 0; i < bsc->_bs_group_num; i++)
    {
        bs = bsc->_bs_svr + i;
        if (bs) {
            req_pack = (as2bs_get_post_t *)bs->_send_buff;
            req_pack->_user_num = 0;
            bs->_lft_len = sizeof(as2bs_get_post_t) - sizeof(req_pack->_user_ids);
            bs->_failed = 0;
            bs->_read_len = 0;
        }
    }

    // sharding request
    for (i = 0; i < req->_user_num; i++) 
    {
        bs = _get_bs_svr(bsc, req->_user_ids[i]);
        if (bs == NULL) {
            log_txt_err("lost bs for key[%" PRIu64 "]", req->_user_ids[i]);
            continue;
        }

        req_pack = (as2bs_get_post_t *)bs->_send_buff;
        int user_num = req_pack->_user_num;
        req_pack->_user_types[user_num] = req->_user_types[i];
        req_pack->_user_ids[user_num] = req->_user_ids[i];
        req_pack->_user_num++;
        /*
        req_pack->_start_idx = req->_start_idx;
        */
        req_pack->_start_idx = 0 ;
        req_pack->_req_num = req->_req_num;
        snprintf(req_pack->_tag, sizeof(req_pack->_tag), "%s", req->_tag);
        bs->_lft_len += sizeof(req->_user_ids[i]);
    }

    // 填写包头
    for (i = 0; i < bsc->_bs_group_num; i++) 
    {
        bs = bsc->_bs_svr + i;
        if (!bs) {
            continue;
        }

        req_pack = (as2bs_get_post_t *)bs->_send_buff;
        if (req_pack->_user_num == 0) {
            continue;
        }

        req_pack->_header.len = bs->_lft_len;
        req_pack->_header.magic = MAGIC_CODE_GET_POST;
        req_pack->_header.cmd = CMD_GET_POST;
        req_pack->_header.sequence = pack_sequece++;
        req_pack->_header.state = 0;
    }

    // 读写网络
    _multi_io(bsc, _ignore_get, max_time);

    // 处理返回结果
    rsp->_list_num = 0;
    for (i = 0; i < bsc->_bs_group_num; i++) {
        bs = bsc->_bs_svr + i ;
        if (bs->_failed || bs->_lft_len > 0) {
            continue;
        }
        if (_ignore_get(bs)) {
            continue;
        }

        rsp_pack = (bs2as_get_post_t *)(bs->_recv_buff);
        
        memcpy(rsp->_user_ids[rsp->_list_num], rsp_pack->_user_ids, 
                rsp_pack->_post_num * sizeof(rsp_pack->_user_ids[0]));
        
        memcpy(rsp->_user_types[rsp->_list_num], rsp_pack->_user_types, 
                rsp_pack->_post_num * sizeof(rsp_pack->_user_types[0]));

        memcpy(rsp->_lists[rsp->_list_num], rsp_pack->_posts, 
                rsp_pack->_post_num * sizeof(rsp_pack->_posts[0]));
        rsp->_lists[rsp->_list_num][rsp_pack->_post_num] = 0;
        rsp->_list_len[rsp->_list_num] = rsp_pack->_post_num;
        rsp->_list_num++;

        log_txt_info("bs[%d] return post-ids length[%d]", i, rsp_pack->_post_num);
        if (rsp_pack->_post_num > 0) {
            log_txt_info("bs[%d] return first-post-id [%lld]", i,  (unsigned long long)rsp->_lists[rsp->_list_num-1][0]);
        }
    }

    return 0;
}

// 获取post list by page: added by Radio.
int bs_get_post_by_page(bs_service_t *bsc, bs_get_post_req_t *req, bs_get_post_rsp_t *rsp, int max_time)
{
    if (bsc == NULL || req == NULL || rsp == NULL) {
        return -1;
    }

    static unsigned pack_sequece = 0;

    int i = 0;
    bs_svr_t *bs = NULL;
    as2bs_get_post_t *req_pack = NULL;
    bs2as_get_post_t *rsp_pack = NULL;

    // reset各组bs
    for (i = 0; i < bsc->_bs_group_num; i++)
    {
        bs = bsc->_bs_svr + i;
        if (bs) {
            req_pack = (as2bs_get_post_t *)bs->_send_buff;
            req_pack->_user_num = 0;
            bs->_lft_len = sizeof(as2bs_get_post_t) - sizeof(req_pack->_user_ids);
            bs->_failed = 0;
            bs->_read_len = 0;
        }
    }

    // sharding request
    for (i = 0; i < req->_user_num; i++) 
    {
        bs = _get_bs_svr(bsc, req->_user_ids[i]);
        if (bs == NULL) {
            log_txt_err("lost bs for key[%" PRIu64 "]", req->_user_ids[i]);
            continue;
        }

        req_pack = (as2bs_get_post_t *)bs->_send_buff;
        int user_num = req_pack->_user_num;
        req_pack->_user_types[user_num] = req->_user_types[i];
        req_pack->_user_ids[user_num] = req->_user_ids[i];
        req_pack->_user_num++;
        /*
        req_pack->_start_idx = req->_start_idx;
        */
        req_pack->_start_idx = req->_start_idx;
        req_pack->_req_num = req->_req_num;
        snprintf(req_pack->_tag, sizeof(req_pack->_tag), "%s", req->_tag);
        bs->_lft_len += sizeof(req->_user_ids[i]);
    }

    // 填写包头
    for (i = 0; i < bsc->_bs_group_num; i++) 
    {
        bs = bsc->_bs_svr + i;
        if (!bs) {
            continue;
        }

        req_pack = (as2bs_get_post_t *)bs->_send_buff;
        if (req_pack->_user_num == 0) {
            continue;
        }

        req_pack->_header.len = bs->_lft_len;
        req_pack->_header.magic = MAGIC_CODE_GET_POST;
        req_pack->_header.cmd = CMD_GET_POST_BY_PAGE;;
        req_pack->_header.sequence = pack_sequece++;
        req_pack->_header.state = 0;
    }

    // 读写网络
    _multi_io(bsc, _ignore_get, max_time);

    // 处理返回结果
    rsp->_list_num = 0;
    for (i = 0; i < bsc->_bs_group_num; i++) {
        bs = bsc->_bs_svr + i ;
        if (bs->_failed || bs->_lft_len > 0) {
            continue;
        }
        if (_ignore_get(bs)) {
            continue;
        }

        rsp_pack = (bs2as_get_post_t *)(bs->_recv_buff);
        
        memcpy(rsp->_user_ids[rsp->_list_num], rsp_pack->_user_ids, 
                rsp_pack->_post_num * sizeof(rsp_pack->_user_ids[0]));
        
        memcpy(rsp->_user_types[rsp->_list_num], rsp_pack->_user_types, 
                rsp_pack->_post_num * sizeof(rsp_pack->_user_types[0]));

        memcpy(rsp->_lists[rsp->_list_num], rsp_pack->_posts, 
                rsp_pack->_post_num * sizeof(rsp_pack->_posts[0]));
        rsp->_lists[rsp->_list_num][rsp_pack->_post_num] = 0;
        rsp->_list_len[rsp->_list_num] = rsp_pack->_post_num;
        rsp->_list_num++;

        log_txt_info("bs[%d] return post-ids length[%d]", i, rsp_pack->_post_num);
        if (rsp_pack->_post_num > 0) {
            log_txt_info("bs[%d] return first-post-id [%lld]", i,  (unsigned long long)rsp->_lists[rsp->_list_num-1][0]);
        }
    }

    return 0;
}

// 获取post长度
int bs_get_post_cnt(bs_service_t *bsc, bs_get_post_cnt_req_t *req, int max_time)
{
    if (bsc == NULL || req == NULL) {
        return -1;
    }

    int i = 0;
    bs_svr_t *bs = NULL;
    as2bs_get_post_cnt_t *req_pack = NULL;
    bs2as_get_post_cnt_t *rsp_pack = NULL;

    // reset各组bs
    for (i = 0; i < bsc->_bs_group_num; i++)
    {
        bs = bsc->_bs_svr + i ;
        if (bs) {
            req_pack = (as2bs_get_post_cnt_t *)bs->_send_buff;
            req_pack->_user_num = 0;
            bs->_lft_len = sizeof(as2bs_get_post_cnt_t) - sizeof(req_pack->_user_ids);
            bs->_failed = 0;
            bs->_read_len = 0;
        }
    }

    // sharding request
    for (i = 0; i < req->_user_num; i++) {
        bs = _get_bs_svr(bsc, req->_user_ids[i]);
        if (bs == NULL) {
            log_txt_err("lost bs for key[%" PRIu64 "]", req->_user_ids[i]);
            continue;
        }

        req_pack = (as2bs_get_post_cnt_t *)bs->_send_buff;
        int user_num = req_pack->_user_num;
        req_pack->_user_ids[user_num] = req->_user_ids[i];
        req_pack->_user_num++;
        snprintf(req_pack->_tag, sizeof(req_pack->_tag), "%s", req->_tag);
        bs->_lft_len += sizeof(req->_user_ids[i]);
    }

    // 填写包头
    for (i = 0; i < bsc->_bs_group_num; i++) {
        bs = bsc->_bs_svr + i ;
        req_pack = (as2bs_get_post_cnt_t *)bs->_send_buff;

        if (req_pack->_user_num == 0) {
            continue;
        }

        req_pack->_header.len = bs->_lft_len;
        req_pack->_header.magic = MAGIC_CODE_GET_POST_CNT;
        req_pack->_header.cmd = CMD_GET_POST_CNT;
        req_pack->_header.sequence = 1;
        req_pack->_header.state = 0;
    }

    // 读写网络
    _multi_io(bsc, _ignore_get_cnt, max_time);

    // 处理返回结果
    int post_cnt = 0;
    for (i = 0; i < bsc->_bs_group_num; i++) {
        bs = bsc->_bs_svr + i ;
        if (bs->_failed || bs->_lft_len > 0) {
            continue;
        }

        rsp_pack = (bs2as_get_post_cnt_t *)(bs->_recv_buff);
        post_cnt += rsp_pack->_post_num;
    }

    return post_cnt;
}

// 发表文章
int bs_set_post(bs_service_t *bsc, bs_set_post_req_t *req, int set_post_user_type, int max_time)
{
    if (bsc == NULL || req == NULL) {
        return -1;
    }
	if ( set_post_user_type != SET_POST_USERID_SET && 
		 set_post_user_type != SET_POST_USERID_REMOVE ) {

		log_txt_err("set_post_user_type error: %d", set_post_user_type) ;
		return -1 ;
	}

    int i = 0;
    bs_svr_t *bs = NULL;
    as2bs_set_post_t *req_pack = NULL;
    int data_end_offset = 0;

    // reset各组bs
    for (i = 0; i < bsc->_bs_group_num; i++)
    {
        bs = bsc->_bs_svr + i ;
        if (bs)
        {
            req_pack = (as2bs_set_post_t *)bs->_send_buff;
            bs->_lft_len = 0;
            bs->_failed = 0;
            bs->_read_len = 0;
            
            req_pack->_pair_cnt = 0;
        }
    }

	/*    一篇post对应至少一个 user_id(作者和关联景点), 对应一种或多种 tag(默认all).     */
	/*  由于 user2post 索引中, 根据 user_id 和 tag 的多种不同组合，同一篇 post 会保存    */
	/*  多份, 因此二重循环枚举 user_id 和 tag 所有组合对作为键, 更新tag_user2post 索引.  */

    for (i = 0; i < req->_user_num; i++)
    {
        bs = _get_bs_svr(bsc, req->_user_ids[i]);
        if (bs == NULL) {
            log_txt_err("lost bs for key[%" PRIu64 "]", req->_user_ids[i]);
            continue;
        }

        req_pack = (as2bs_set_post_t *)bs->_send_buff;
        req_pack->_post_id = req->_post_id;
		req_pack->_set_post_user_type = set_post_user_type ;

        for (int j = 0; j < req->_tag_num; j++)
        {
            int currIdx = req_pack->_pair_cnt;
            
            if (currIdx >= (int)(sizeof(req_pack->_user_ids) / sizeof(req_pack->_user_ids[0]))) {
                log_txt_err("user_id was full, when set post:%llu", (unsigned long long)req->_post_id);
                break;
            }

            req_pack->_user_ids[currIdx] = req->_user_ids[i];
            
            if (currIdx == 0)
            {
                req_pack->_tags[currIdx] = 0;
            }

            int size = sizeof(req_pack->_data) - req_pack->_tags[currIdx];
            char *ptr = (char *)(req_pack->_data) + req_pack->_tags[currIdx];
            int ret = snprintf(ptr, size, "%s", req->_tags[j]);
            if (ret >= size) {
                log_txt_err("snprintf post tag failed, when set post, buffer was too short,"
                            "left size:[%d], tag:[%s], post_id:[%" PRIu64 "], user_id:[%" PRIu64 "]", 
                            size, req->_tags[j], req->_post_id, req->_user_ids[i]);
                //req_pack->_tags[currIdx+1] = req_pack->_data + sizeof(req_pack->_data);
                req_pack->_tags[currIdx+1] = sizeof(req_pack->_data);
                data_end_offset = sizeof(req_pack->_data);
                break;
            }
            
            req_pack->_pair_cnt++;
            
            data_end_offset = req_pack->_tags[currIdx] + ret + 1;

            if (currIdx + 1 < (int)(sizeof(req_pack->_tags) / sizeof(req_pack->_tags[0])))
            {
                req_pack->_tags[currIdx+1] = req_pack->_tags[currIdx] + ret + 1;
            }
            else
            {
                log_txt_err("as2bs_set_post_t::_tags full. post_id:[%llu], current idx:[%d], capacity:[%d]", 
                            (unsigned long long)req->_post_id, currIdx, 
                            (int)(sizeof(req_pack->_tags) / sizeof(req_pack->_tags[0])));
                break;
            }
        }
    }

    // 填写包头
    for (i = 0; i < bsc->_bs_group_num; i++)
    {
        bs = bsc->_bs_svr + i;
        req_pack = (as2bs_set_post_t *)bs->_send_buff;

        if (req_pack->_pair_cnt == 0)
        {
            continue;
        }
        
        int free_len = sizeof(as2bs_set_post_t) - sizeof(req_pack->_data) + data_end_offset;
        bs->_lft_len = free_len;
        req_pack->_header.len = bs->_lft_len;

        /*
        for (int j = 0; j < req_pack->_pair_cnt; j++) {
            log_txt_info("tag offset[%d]=%d", j, req_pack->_tags[j]);
            log_txt_info("tag[%d]=%s", j, (char *)(req_pack->_data) + req_pack->_tags[j]);
            log_txt_info("user_id[%d]=%llu", j, req_pack->_user_ids[j]);
        }
        */
        req_pack->_header.magic = MAGIC_CODE_SET_POST;
        req_pack->_header.cmd = CMD_SET_POST;
        req_pack->_header.sequence = 1;
        req_pack->_header.state = 0;
    }

    // 读写网络
    _multi_io(bsc, _ignore_set, max_time);

    return 0;
}

// 断开连接
void bs_free(bs_service_t *bsc)
{
    if (bsc == NULL) {
        return;
    }

    for (int i = 0; i < bsc->_bs_group_num; i++) {
        if (bsc->_bs_svr[i]._send_buff) {
            free(bsc->_bs_svr[i]._send_buff) ;
            bsc->_bs_svr[i]._send_buff = NULL;
        }

        if (bsc->_bs_svr[i]._recv_buff) {
            free(bsc->_bs_svr[i]._recv_buff) ;
            bsc->_bs_svr[i]._recv_buff = NULL;
        }
    }

    free(bsc->_bs_svr);
    free(bsc);
}

