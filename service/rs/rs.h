#ifndef QINGYI_SERVICE_RS_
#define QINGYI_SERVICE_RS_
#ifdef __cplusplus
extern "C" {
#endif

#define QY_MONGO_DB "qingyi"
#define QY_RECOMMEND_COLL "qy_recommend"
#define QY_RELATION_COLL "qy_relation"
#define QY_UID_KEY "uid"

int rs_init(const char *conf_file);
int rs_proc(struct io_buff *buff);
int rs_uninit();

#ifdef __cplusplus
}
#endif

#endif // QINGYI_SERVICE_RS_
