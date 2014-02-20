#include "Daemon.h"
#include <errno.h>

string run_path;

void get_run_path(){
    char* path = getenv("HIST_SIZE_ROOT");
    run_path = path;
};

int main(int argc, char* argv[]){
    get_run_path();
    CServerFrame csf;
    CDaemon daemon(&csf);
    daemon.run(argc, argv);
    return 0;
}

