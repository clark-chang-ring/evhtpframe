#include "my_func.h"
#include "my_redis.h"

int main(int argc, char** argv){
    my_redis* ptr_rd = new my_redis("10.101.0.72:6379;10.101.0.72:6380;10.101.0.72:6381");
    ptr_rd->connect();
    ptr_rd->select(4);
    ptr_rd->
}
