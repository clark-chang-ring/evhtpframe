#include "ServerFrame.h"
#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
using namespace Json;
bool CServerFrame::m_bShutdown = false;

evbase_t*       evbase = event_base_new();
evhtp_t*        htp    = evhtp_new(evbase, NULL);
pthread_key_t   pthread_rdkey_main;
pthread_key_t   pthread_rdkey_mbvw;
pthread_key_t   pthread_rdkey_view;
pthread_key_t   pthread_key_b_flow;
pthread_key_t   pthread_key_c_flow;

CServerFrame::CServerFrame(){
    m_b_init_ok = true;
}

bool CServerFrame::createServer(u_short nPort,u_short nThreadCount){
    m_lsnPort      = nPort;
    m_nThreadCount  = nThreadCount;
    Configer cfger;
    cfger.parseConfig(m_hssCfg);
    return true;
}

void CServerFrame::health_check(evhtp_request_t* req, void* para){
    char* ret = new char[10];
    strcpy(ret, "OK");
    evbuffer_add_reference(req->buffer_out, 
                ret, strlen(ret), free_buf, NULL);
    evhtp_send_reply(req, EVHTP_RES_OK);
    return;
}

void CServerFrame::pc(evhtp_request_t* req, char *ret, HSS& hss_config, timeval start_time){
    char key[1024];
    my_redis *ptr_redis = (my_redis*)pthread_getspecific(pthread_rdkey_main);
    const char* user_id = evhtp_kv_find(req->uri->query, "user_id");
    hash_map<string, hash_map<string, string> > h;
    snprintf(key, 1024, "%s_o", user_id);
    ptr_redis->getUserOrder(key, REDIS_DB_ORDER, h);
    hash_map<string, hash_map<string, string> >::iterator iter;
    hash_map<string, string>::iterator iter2;
    for(iter = h.begin(); iter != h.end(); iter++){
        printf("----%s\n", (iter->first).c_str());
        for(iter2 = (iter->second).begin(); iter2 != (iter->second).end(); iter2++)
            printf("--------%s:%s\n", (iter2->first).c_str(), (iter2->second).c_str());
    }
    evbuffer_add_reference(req->buffer_out, 
                ret, strlen(ret), free_buf, NULL);
    evhtp_send_reply(req, EVHTP_RES_OK);
}

void CServerFrame::user(evhtp_request_t* req, void* para){ 
    timeval start_time;
    gettimeofday(&start_time, NULL);

    HSS hss_config = (*(param*)para).hssCfg;
    char* ret = new char[MAX_RETURN_LEN];
    pc(req, ret, hss_config, start_time);
}

void CServerFrame::free_buf(const void *data, 
        size_t datalen, void *extra){
    delete[] (char*)data;
}

bool CServerFrame::runServer(){
    if(!m_b_init_ok)
        return false;

    param st_ptr = {m_hssCfg};
    evhtp_set_cb(htp, "/_health_check", health_check, NULL);
    evhtp_set_cb(htp, "/user", user, &st_ptr);
    evhtp_set_cb(htp, "/user/", user, &st_ptr);
    pthread_key_create(&pthread_rdkey_main, NULL);
    pthread_key_create(&pthread_rdkey_mbvw, NULL);
    pthread_key_create(&pthread_rdkey_view, NULL);
    pthread_key_create(&pthread_key_b_flow, NULL);
    pthread_key_create(&pthread_key_c_flow, NULL);
    evhtp_use_threads(htp, evhtp_thread_register, m_nThreadCount, &m_hssCfg);
    int iBindRet;
    if((iBindRet = evhtp_bind_socket(htp, "0.0.0.0", 
                    m_lsnPort, 1024)) != 0){
        printf("bind port %d failed, exit num %d\n", m_lsnPort, iBindRet);
        return false;
    }
    event_base_loop(evbase, 0); 
    return true;
}

void CServerFrame::evhtp_thread_register(evhtp_t * htp, 
            evthr_t * thr, void *config){
    my_redis *ptr_redis = new my_redis((*(HSS*)config)["USER_PROFILE_REDIS_GROUP"]);
    my_redis *ptr_redis_mb = new my_redis((*(HSS*)config)["USER_PROFILE_REDIS_GROUP_MOBILE"]);
    my_redis *ptr_redis_view = new my_redis((*(HSS*)config)["USER_PROFILE_REDIS_GROUP_2ND"]);
    ptr_redis->connect();
    ptr_redis_mb->connect();
    ptr_redis_view->connect();
    pthread_setspecific(pthread_rdkey_main, ptr_redis);
    pthread_setspecific(pthread_rdkey_mbvw, ptr_redis_mb);
    pthread_setspecific(pthread_rdkey_view, ptr_redis_view);
}

bool CServerFrame::closeServer(){
    m_bShutdown = true;
    return true;
}

string CServerFrame::_param_err(char* ret, timeval start_time){
    timeval end_time;
    gettimeofday(&end_time, NULL);
    u64 time_use = 1000000 * (end_time.tv_sec - start_time.tv_sec)
        + (end_time.tv_usec - start_time.tv_usec);
    char p[128];
    snprintf(p, 128, "%lluus", time_use);
    Value root, info;
    info["process"] = p;
    info["message"] = "method can't be null";
    root["info"] = info;
    root["status_code"] = 400;
    snprintf(ret, MAX_RETURN_LEN, root.toStyledString().c_str());
    return ret;
}


void CServerFrame::ret_str(char* ret, char* user_type, timeval start_time){
    Value root, data, info;
    char p[128];
    timeval end_time;
    gettimeofday(&end_time, NULL);
    u64 time_use = 1000000 * (end_time.tv_sec - start_time.tv_sec)
        + (end_time.tv_usec - start_time.tv_usec);
    snprintf(p, 128, "%lluus", time_use);
    root["code"] = 200;
    if(NULL != user_type)
        data["user_type"] = user_type;
    info["process"] = p;
    root["data"] = data;
    root["info"] = info;
    snprintf(ret, MAX_RETURN_LEN, root.toStyledString().c_str());
}
