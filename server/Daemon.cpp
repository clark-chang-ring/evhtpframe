#include "Daemon.h"

int CDaemon::m_nChildPid=0;
CServerFrame* CDaemon::m_pServer = NULL;
void CDaemon::initAsDaemon(){
    if(fork() > 0){
        exit(0);
    }
    setsid();

    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTERM, sigMasterHandler);
    signal(SIGINT,  sigMasterHandler);
    signal(SIGQUIT, sigMasterHandler);
    signal(SIGKILL, sigMasterHandler);
}

void CDaemon::sigMasterHandler(int sig){       
    kill(m_nChildPid, SIGUSR1);
    fprintf(stdout, "master [%d] sig [%d]!\n",getpid(), sig);
}

void CDaemon::sigChildHandler(int sig){
    m_pServer->closeServer();
    fprintf(stdout, "child  [%d] sig [%d]!\n",getpid(),sig);
}

bool CDaemon::run(int argc,char** argv){
    parseCmdLine(argc,argv);
    m_pName = strrchr(argv[0],'/');
    m_pName != NULL ? m_pName += 1 : m_pName = argv[0];

    HISI i=m_hisOptVal.find('k');

    if (i!=m_hisOptVal.end()){
        if (i->second=="start"){
            return start();
        }else if(i->second=="stop"){
            return stop();
        }else if(i->second=="restart"){
            if(stop()){
                return start();
            }
            return false;
        }
    }
    fprintf(stderr, "Usage sizeReco -k start|stop|restart [-n thread_number(%d) -p port(%d)]\n", DEFAULT_THREAD_NUM, DEFAULT_PORT);
    return false;
}

int CDaemon::parseCmdLine(int argc,char** argv){
    for (int i=1; i<argc; ++i){
        if(argv[i][0] != '-'){
            continue;
        }

        if(i == argc - 1){
            break;
        }

        if (argv[i+1][0] == '-'){
            m_hisOptVal[argv[i][1]]="";
            continue;
        }

        m_hisOptVal[argv[i][1]] = argv[i+1];
        ++i;
    }
    return m_hisOptVal.size();
}

/* modify from book apue
*  when son dies ,father restart it except for:1 son exit with "exit" or kill by signal 9
*/
bool CDaemon::isAbnormalExit(int pid, int status){
    bool bRestart = true;
    char buf[1024];
    if(WIFEXITED(status)){ //exit()or return
        sprintf(buf, "Child normal termination(pid [%d], status [%d])", pid, WEXITSTATUS(status));
        bRestart = false;
    }
    else if(WIFSIGNALED(status)){ //signal
        sprintf(buf, "Abnormal termination(pid [%d], signal number [%d]", pid, WTERMSIG(status));
        if(WTERMSIG(status) == SIGKILL){
            bRestart = false;
            sprintf(buf, "Killed by user?(exit pid [%d], status = %d, signal = [SIGKILL])", pid, WEXITSTATUS(status));
        }
    }
    else if(WIFSTOPPED(status)){//暂停的子进程退出
         sprintf(buf, "Child stopped(pid [%d], signal number [%d])", pid, WSTOPSIG(status));
    }
    else{
         sprintf(buf, "Unknown exit(pid [%d], signal number [%d]", pid, WSTOPSIG(status));
    }
    serverLog(buf);
    return bRestart;
}

bool CDaemon::start(){
    //get daemon pid from file
    char buf[640];
    int masterPid;
    
    string strName = m_runPath + daemon_pid_file;
    strName = strName + "." + m_pName;
    if ( 0<read1LineFromFile(strName.c_str(), buf, 64, "r") &&(masterPid = atoi(buf)) != 0){
        if(kill(masterPid, 0) == 0){
            printf("Instance's running, ready to quit!\n");
            return true;
        }
    }
    initAsDaemon();
    sprintf(buf, "%d", getpid());
    if(!writeBuff2File(strName.c_str(), buf, "w")){
        fprintf(stderr, "Write master pid fail!\n");
    }

    while(true){//daemon fork and son do jobs
        pid_t pid = fork();
        if (pid == 0){
            signal(SIGUSR1, sigChildHandler);
            signal(SIGPIPE, SIG_IGN);
            signal(SIGTTOU, SIG_IGN);
            signal(SIGTTIN, SIG_IGN);
            signal(SIGTERM, SIG_IGN);
            signal(SIGINT,  SIG_IGN);
            signal(SIGQUIT, SIG_IGN);
            
            if(!initServer()){
                fprintf(stderr, "Server init  fail!\n");
                return false;
            }   
            char buffer[1024];
            sprintf(buffer, "Server init  ok pid = %d", (int)getpid());
            serverLog(buffer);

            char buf[640];
            sprintf(buf, "%d", getpid());
            string strName = m_runPath + "daemon.child.pid";
            if(!writeBuff2File(strName.c_str(), buf, "w")){
                fprintf(stderr, "Write child pid fail!\n");
            }

            if(!runServer()){
                fprintf(stderr, "run fail!\n");
                exit(0);
                return false;
            }
            serverLog("server run finished");
            exit(0);
        }
        m_nChildPid=pid;
        int status;
        pid = wait(&status);
        printf("wait child over\n");
        if(!isAbnormalExit(pid, status)){
            serverLog("Child exit!");
            break;
        }
    }
    return true;
}

int CDaemon::read1LineFromFile(const char* fileName, char* buf, int maxCount, const char* mode){
    FILE* fp = fopen(fileName, mode);
    if (!fp){
        return 0;
    }
    int ret;
    fgets(buf, maxCount, fp) ? ret = 1 : ret = 0;
    fclose(fp);
    return ret;
}

int CDaemon::writeBuff2File(const char* fileName, const char* buf, const char* mode){
    FILE* fp = fopen(fileName, mode);
    if(!fp){
        return 0;
    }
    int n = fprintf(fp, "%s", buf);
    fclose(fp);
    return n;
}



bool CDaemon::initServer(){
    int nPort = 0, nThreadNum = 0;
    HISI i = m_hisOptVal.find('n');   
    if(i != m_hisOptVal.end()){
        nThreadNum = atoi(i->second.c_str());
    }else{
        nThreadNum = DEFAULT_THREAD_NUM;
    }

    i = m_hisOptVal.find('p');   
    if(i != m_hisOptVal.end()){
        nPort=atoi(i->second.c_str());
    }else{
        nPort = DEFAULT_PORT;
    }

    if(nThreadNum<=0||nPort<=0)
        return false;

    return m_pServer->createServer(nPort, nThreadNum);
}

bool CDaemon::stop(){
    char buf[640];
    int masterPid;//子进程pid
    string strName = m_runPath + "daemon.child.pid";

    if ( 0<read1LineFromFile(strName.c_str(), buf, 64, "r") &&(masterPid = atoi(buf)) != 0){
        if (kill(masterPid, 0) == 0){
            int tryTime = 200;      
            kill(masterPid, SIGKILL);
            while (kill(masterPid, 0) == 0 && --tryTime){
                sleep(1);           
            }
            if (!tryTime && kill(masterPid, 0) == 0){
                fprintf(stderr, "Time out shutdown fail!\n");       
                return false    ;
            }
            return true;
        }
    }
    return true;
}

int CDaemon::serverLog(const char* log){
    char buf[1024];
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    strftime(buf, 100, "%Y-%m-%d %H:%M:%S------", local);
    sprintf(buf + strlen(buf), "%s\n", log);
    return writeBuff2File("../log/server.log", buf, "a+");
}
