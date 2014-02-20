#include "my_redis.h"
#include "my_func.h"

my_redis::my_redis(string rd_config){
    my_func mf;
    vector<string> tmp1, tmp2, tmp3;
    vector<vector<string> > grp;
    tmp1 = mf.explode(";", rd_config);
    for(size_t i = 0; i < tmp1.size(); i++){
        tmp2 = mf.explode(",", tmp1[i]);
        grp.clear();
        for(size_t j = 0; j < tmp2.size(); j++){
            tmp3 = mf.explode(":",tmp2[j]);
            grp.push_back(tmp3);
        }
        rd_svr_grp.push_back(grp);
    }
}

bool my_redis::connect(){
    rd_grp.clear();
    vector<redisContext*> grp_now;
    redisContext *rc;
    for(size_t i = 0; i < rd_svr_grp.size(); i++){
        grp_now.clear();
        for(size_t j = 0; j < rd_svr_grp[i].size(); j++){
            int port = atoi(rd_svr_grp[i][j][1].c_str());
            rc = redisConnect(rd_svr_grp[i][j][0].c_str(), port);
            grp_now.push_back(rc);
        }
        rd_grp.push_back(grp_now);
    }
    return true;
}

bool my_redis::select(redisContext *c, int db){
    if(c->err)
        return false;

    redisReply *reply;
    reply = (redisReply*)redisCommand(c,"select %d",db);
    if(NULL != reply){
        if(strcmp(reply->str, "OK") != 0){
            freeReplyObject(reply);
            return false;
        }
        freeReplyObject(reply);
        return true;
    }
    return false;
}

bool my_redis::reconnect(int grp_num, int server_num){
    redisContext *rc;
    int port = atoi(rd_svr_grp[grp_num][server_num][1].c_str());
    rc = redisConnect(rd_svr_grp[grp_num][server_num][0].c_str(), port);
    rd_grp[grp_num][server_num] = rc;
    return true;
}

int my_redis::_hash(string key){
    int h = 0;
    for(size_t i = 0; i < key.length(); i++)
        h += key[i];
    return h;
}

int my_redis::_get_grp_num(int h){
    int grp_cnt = rd_grp.size();
    return h%grp_cnt;
}

redisContext* my_redis::_get_redis_ctx(string key){
    int h = _hash(key);
    int grp_num = _get_grp_num(h);
    int svr_cnt_in_grp = rd_grp[grp_num].size();
    srand((unsigned)time(NULL));
    int rand_int = rand();//随机一台
    int server_num = rand_int % svr_cnt_in_grp;
    redisContext *c = rd_grp[grp_num][server_num];
    if(NULL == c || c->err){
        redisFree(c);
        reconnect(grp_num, server_num);
        c = rd_grp[grp_num][server_num];
        if(NULL == c || c->err){
            for(int i = 0; i < svr_cnt_in_grp - 1; i++){
                server_num = (server_num + 1) % svr_cnt_in_grp;
                c = rd_grp[grp_num][server_num];
                if(NULL != c && !c->err)
                    break;
            }
        }
    }
    return c;
}

char* my_redis::get(string key, int db){
    redisContext *c = _get_redis_ctx(key);
    if(c->err)
        return NULL;

    redisReply *reply;

    select(c, db);
    reply = (redisReply*)redisCommand(c,"get %s", key.c_str());
 
    if(NULL == reply)
        return NULL;
    

    if(0 == reply->len){
        freeReplyObject(reply);
        return NULL;
    }
    char* ret = new char[reply->len+1];
    snprintf(ret, reply->len+1, "%s", reply->str);
    freeReplyObject(reply);
    return ret;
}

bool my_redis::set(string key, string val, int db){
    int h = _hash(key);
    int grp_num = _get_grp_num(h);
    for(size_t i = 0; i < rd_grp[grp_num].size(); i++){
        redisContext *c = rd_grp[grp_num][i];

        if(c->err){
            reconnect(grp_num, i);
            c = rd_grp[grp_num][i];
        }

        if(c->err)
            return false;

        redisReply *reply;

        select(c, db);

        reply = (redisReply*)redisCommand(c,"set %s %s", key.c_str(), val.c_str());
        if(NULL == reply)
            return false;

        freeReplyObject(reply);
    }
    return true;
}

bool my_redis::expireAt(string key, time_t expire_t, int db){
    int h = _hash(key);
    int grp_num = _get_grp_num(h);
    for(size_t i = 0; i < rd_grp[grp_num].size(); i++){
        redisContext *c = rd_grp[grp_num][i];

        if(c->err){
            reconnect(grp_num, i);
            c = rd_grp[grp_num][i];
        }

        if(c->err)
            return false;

        redisReply *reply;

        select(c, db);

        reply = (redisReply*)redisCommand(c,"expireAt %s %u", key.c_str(), expire_t);
        if(NULL == reply)
            return false;

        freeReplyObject(reply);
    }
    return true;
}

int my_redis::hLen(string h, int db){
    redisContext *c = _get_redis_ctx(h);
    if(c->err)
        return 0;

    redisReply *reply;

    select(c, db);

    reply = (redisReply*)redisCommand(c,"hLen %s", h.c_str());

    if(NULL == reply)
        return 0;

    int len = reply->integer;
    freeReplyObject(reply);
    return len;
}

bool my_redis::zRange(string key, int start, int end , hash_map<string, string>& res, int db){
    res.clear();
    redisContext *c = _get_redis_ctx(key);
    if(c->err)
        return false;

    redisReply *reply;

    select(c, db);

    reply = (redisReply*)redisCommand(c,"zRange %s %d %d withscores", key.c_str(), start, end);

    if(NULL != reply){
        for(size_t i = 0; i < reply->elements; i+=2)
            res.insert(make_pair(reply->element[i]->str,
                        reply->element[i+1]->str));
    }
    freeReplyObject(reply);
    return true;
}

int my_redis::zRemRangeByScore(string key, int start, int end, int db){
    redisContext *c = _get_redis_ctx(key);
    if(c->err)
        return false;

    redisReply *reply;

    select(c, db);

    reply = (redisReply*)redisCommand(c,"zRemRangeByScore %s %d %d", 
            key.c_str(), start, end);

    if(NULL == reply)
        return 0;

    int len = reply->integer;
    freeReplyObject(reply);
    return len;
}

int my_redis::zSize(string key, int db){
    redisContext *c = _get_redis_ctx(key);
    if(c->err)
        return 0;

    redisReply *reply;

    select(c, db);

    reply = (redisReply*)redisCommand(c,"zcard %s", key.c_str());
    
    if(NULL == reply)
        return 0;

    int len = reply->integer;
    freeReplyObject(reply);
    return len;
}

int my_redis::incrBy(string key, int n, int db){
    int h = _hash(key);
    int grp_num = _get_grp_num(h);
    int res = 0;
    for(size_t i = 0; i < rd_grp[grp_num].size(); i++){
        redisContext *c = rd_grp[grp_num][i];
        if(c->err){
            reconnect(grp_num, i);
            c = rd_grp[grp_num][i];
        }

        if(c->err)
            return 0;

        redisReply *reply;

        select(c, db);

        reply = (redisReply*)redisCommand(c,"incrby %s %d", key.c_str(), n);

        if(NULL == reply)
            return 0;

        res = reply->integer;
        freeReplyObject(reply);
    }
    return res;
}

bool my_redis::getUserOrder(string key, int db, 
        hash_map<string, hash_map<string, string> > &h){
    printf("run_path %s\n", run_path.c_str());
    redisContext *c = _get_redis_ctx(key);
    if(c->err)
        return false;

    redisReply *reply;

    select(c, db);

    string lua_script = "local res = redis.call('zrange', KEYS[1], 0, -1);";
    lua_script += "local orders={}; for key, order_id in pairs(res) do  table.insert(orders, 'order_id');";
    lua_script += "table.insert(orders, order_id) local res2 = redis.call('hgetall', order_id..'_f');";
    lua_script += "for key2, v in pairs(res2) do table.insert(orders, v) end;end; return orders";

    reply = (redisReply*)redisCommand(c, "eval %s 1 %s", lua_script.c_str(), key.c_str());

    if(NULL == reply){
        freeReplyObject(reply);
        return false;
    }
    char order_id[20];
    hash_map<string, string> h_fields;
    for(size_t i = 0; i < reply->elements; i += 2){
        if(strcmp("order_id", reply->element[i]->str) == 0){
            if(h_fields.size() != 0)
                h.insert(make_pair(order_id, h_fields));
            snprintf(order_id, 20, reply->element[i+1]->str);
            h_fields.clear();
            continue;
        }
        h_fields.insert(make_pair(reply->element[i]->str, reply->element[i+1]->str));
    }
    h.insert(make_pair(order_id, h_fields));
    freeReplyObject(reply);
    return true;
}
