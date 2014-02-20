/*
 *author wuxudong
 *donald.wu@vipshop.com
 * */
#ifndef SERVERFRAME_H_
#define SERVERFRAME_H_
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <evhtp.h>
#include "ComDef.h"
#include "hash_wrap.h"
#include "nokey_static_hash.h"
#include "json/json.h"
#include "my_func.h"
#include "my_redis.h"
#include "Configer.h"

#define SOCKET_ERROR -1
#define INVALID_SOCKET  -1 

#define USER_ID "u_id"
#define MALL_ID "m_id"
#define RECO_ID "r_id"
#define DEBUG_MODE "debug"
#define ETH_NAME "eth0"
#define MAX_RETURN_LEN 10240
#define VIEW_MAX_DAY 7
#define VIEW_MAX_CNT 50
#define REDIS_DB_UID_VIEW 0
#define REDIS_DB_CID_VIEW 1
#define REDIS_DB_PC_TAG   2
#define REDIS_DB_FRESH    3
#define REDIS_DB_ORDER    4
#define REDIS_DB_SEX      5
#define VP_NH             1

//using namespace sd;
typedef unsigned long long  u64;
typedef unsigned long       ul;
typedef unsigned int        u32;
typedef int             SOCKET;
typedef int             vip_id_t;

extern string run_path;
/*
char buf[128];
getcwd(buf, 128);
runPath = buf;
*/
//runPath += "/";

/*
 *主线程->线程通信结构
 *数据由主线程加载
 * */
struct param{
    HSS     hssCfg;         //解析的配置
    //int*    ptr_user_flow//用户分流配置
};

class CServerFrame{
protected:
    static void user(evhtp_request_t* req, void* a);
    static void health_check(evhtp_request_t* req, void* a);
    static string _param_err(char* err, timeval start_time);
    static void ret_str(char* ret, char* user_type, timeval start_time);
    static void evhtp_thread_register(evhtp_t * htp, 
            evthr_t * thr, void * arg);

    static void free_buf(const void *data, size_t datalen, void *extra);
    static void pc(evhtp_request_t* req, char *ret, HSS&, timeval start_time);
    static void mobile(evhtp_request_t* req, char *ret, timeval start_time);
protected:
    static bool has_web_order(my_redis* ptr_redis, const char *user_id);
    static bool has_wap_order(my_redis* ptr_redis, const char *user_id);
    static void get_mobile_type(my_redis *ptr_redis, char *user_type, const char *user_id);
    static int* get_flow(string s);

public:
    CServerFrame();
    ~CServerFrame(void){};

    /*
     *function create http server
     *para @nPort start port
     *para @nThreadCount thread count
     *return bool if create suceessfully
     * */
    bool createServer(u_short nPort,u_short nThreadCount);
    /*
     *function close server
     *return if close successfully
     * */
    bool closeServer();
    /*
     *function start run server
     *return if run server successfully
     * */
    bool runServer();
    
private:
    u_short m_nThreadCount;             //server thread count
    u_short m_lsnPort;
    static bool m_bShutdown ;           
    vector<const char*> m_v_data;
    static_hash_map<int, int> m_sh;
    HSS m_hssCfg;
    bool m_b_init_ok;
    string m_runPath;
};
#endif
