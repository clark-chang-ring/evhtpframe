#ifndef CONFIGER_H_
#define CONFIGER_H_
#include <ext/hash_map>
#include <stdio.h>
#include <string>
#include "hash_wrap.h"
#include "ComDef.h"
using namespace std;
using namespace __gnu_cxx;
class Configer{
    public:
        Configer(){};
        ~Configer(){};
    public:
        bool parseConfig(HSS& hssConf){
            const char* env_name[] = {
                "USER_PROFILE_REDIS_GROUP",
                "USER_PROFILE_B_DISTRIBUTE",
                "USER_PROFILE_C_DISTRIBUTE",
                "USER_PROFILE_REDIS_GROUP_MOBILE",
                "USER_PROFILE_REDIS_GROUP_2ND"
            };
            char* env_val;
            for(size_t i = 0; i < sizeof(env_name)/sizeof(env_name[0]); i++){
                env_val = getenv(env_name[i]);
                if(env_val == NULL){
                    fprintf(stderr, "[WARNING] get environment value [%s] NULL\n", env_name[i]);
                }else{
                    hssConf.insert(make_pair(env_name[i], env_val));
                }
            }
            return true;
        }
};
#endif
