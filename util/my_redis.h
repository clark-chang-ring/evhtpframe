#ifndef _MY_REDIS_H_
#define _MY_REDIS_H_
#include <stdlib.h>
#include <unistd.h>
#include <hiredis/hiredis.h>
#include "ComDef.h" 
extern string run_path;

//using namespace std;
class my_redis{
    private:
        //grp
        //  servers
        vector<vector<redisContext*> >    rd_grp;
        redisReply      *rd_reply;
        //grp
        //  grp_servers
        //      host port
        vector<vector<vector<string> > >rd_svr_grp;
    private:
        int _hash(string key);
        int _get_grp_num(int hash);
        redisContext* _get_redis_ctx(string key);
        bool reconnect(int grp_num, int server_num);
        bool select(redisContext* c, int db);
    public:
        my_redis(string rd_config);
        ~my_redis(){
            //redisFree(rd);
        }
        bool connect();
        char* get(string key, int db);
        int hLen(string h, int db);
        bool zRange(string key, int start, int end , 
                hash_map<string, string>& res, int db);
        int zSize(string key, int db);
        int zRemRangeByScore(string key, int start, int end, int db);
        bool set(string key, string val, int db);
        bool expireAt(string key, time_t expire_t, int db);
        int incrBy(string key, int n, int db);
        bool getUserOrder(string key, int db,
                hash_map<string, hash_map<string, string> > &h);
};
#endif
