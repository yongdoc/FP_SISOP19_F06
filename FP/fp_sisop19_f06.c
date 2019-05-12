#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

const char * path="/home/yong/Documents/buat_crontab.txt";

struct tipecron{
    int crtime[5];
    char fpath[256];
    int exc;
    int line;
};

struct tipecron crontab[15];
int lastmod=0;

void getTime(int l, char cron[])
{
    int t=0,c=0;
    crontab[l].line=l;
    while(1){
        if(t==5) {crontab[l].exc=1;break;}
        if(cron[c]==' '){
            c++;
        }else if(cron[c]=='*'){
            crontab[l].crtime[t]=-1;
            t++;c++;
        }else if((int)'0'<=(int)cron[c]<=(int)'9'){
            int temp=0;
            while(cron[c]!=' '){
                temp*=10;
                temp+=cron[c]-'0';
                c++;
            }
            crontab[l].crtime[t]=temp;
            if(t==3)crontab[l].crtime[t]-=1;
            t++;
        }else if(t<5) {
            crontab[l].exc=0;
            break;
        }
    }
    char *path=strrchr(cron,' ');
    char *temp=strchr(path,'/');
    strcpy(crontab[l].fpath,temp);
}

void *cronex(void* ar)
{
    struct tipecron *arg=(struct tipecron *)ar;
    struct stat ts;
    time_t rawtime,cek;
    struct tm *curtime;
    stat(path,&ts);
    while(lastmod==ts.st_mtime){
        time(&rawtime);
        curtime=localtime(&rawtime);
        if(arg->exc){
            if(arg->crtime[0]==curtime->tm_min||arg->crtime[0]<0){
                if(arg->crtime[1]==curtime->tm_hour||arg->crtime[1]<0){
                    if(arg->crtime[2]==curtime->tm_mday||arg->crtime[2]<0){
                        if(arg->crtime[3]==curtime->tm_mon||arg->crtime[3]<0){
                            if(arg->crtime[4]==curtime->tm_wday||arg->crtime[4]<0){
                                pid_t child;
                                child=fork();
                                if(child!=0){
                                    char *p=strrchr(arg->fpath,'/');
                                    printf("%s has executed\n",p);
                                }else {
                                    char *argv[]={"bash",arg->fpath,NULL};
                                    execv("/bin/bash",argv);
                                }
                            }
                        }
                    }
                }
            }
            sleep(60);
            time(&cek);
            // while(difftime(cek,rawtime)<=60.0) {
            //     sleep(1);
            //     time(&cek);
            // }
        }else {
            char *p=strrchr(arg->fpath,'/');
            printf("file %s on line %d is not executed (check your crontab syntax !)\n",p,arg->line);
        }
    }
}

void getFile(FILE* file){
    int line=0;
    char ch;
    int i=0;
    char conf[256];
    pthread_t thr[15];
    memset(conf,'\0',sizeof(conf));
    while(fscanf(file,"%c",&ch)!=EOF){
        if(ch=='\n'){
            if(conf[0]=='\0')continue;
            strcpy(crontab[line].fpath,conf);
            getTime(line,crontab[line].fpath);
            struct tipecron *p_cron = &crontab[line];
            pthread_create(&thr[line],NULL,cronex,(void*)p_cron);
            memset(conf,'\0',sizeof(conf));
            line++;
            i=0;

        }else {
            i++;
            conf[i]=ch;
        }
    }
}

int main() {
    pid_t pid, sid;

    FILE* cronfile;

    pid = fork();

    if (pid < 0) {
    exit(EXIT_FAILURE);
    }

    if (pid > 0) {
    exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();

    if (sid < 0) {
    exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int z=0;
    for(;z<15;z++){
        crontab[z].exc=1;
    }

    while(1) {
        struct stat st;
        stat(path,&st);
        if(lastmod!=st.st_mtime){
            lastmod=st.st_mtime;
            cronfile=fopen(path,"r");
            getFile(cronfile);
        }
        sleep(1);
    }

    exit(EXIT_SUCCESS);
}
