#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <termios.h>
#include <vector>
#include <semaphore.h>
#include <signal.h>

#define IPC_RESULT_ERROR (-1)

sem_t *sem;

int getch()
{
    int ch;
    struct termios oldt, newt;
    tcgetattr( STDIN_FILENO, &oldt );
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
    return ch;
}

struct kol{
    char message[256] = "\0";
    int *import;
    int *exp;
    kol *next;
    kol *prev;
};

int get_shared_block(char *filename, int size, int id){
    key_t key;
    key = ftok(filename, id);
    if(key == IPC_RESULT_ERROR){
        return IPC_RESULT_ERROR;
    }
    return shmget(key, size, 0666 | IPC_CREAT);
}

void * attach_memory_block(char* filename, int size, int id){
    int shared_block_id = get_shared_block(filename, size, id);
    void * ring;
    ring = (void*)(shmat(shared_block_id, NULL, 0));
    if(ring == (void*)IPC_RESULT_ERROR){
        return NULL;
    }
    return ring;
}

bool detach_memory_block(kol* block){
    return (shmdt(block) != IPC_RESULT_ERROR);
}

bool destroy_memory_block(char *filename, int id){
    int shared_block_id = get_shared_block(filename, sizeof(kol),id);
    if(shared_block_id == IPC_RESULT_ERROR){
        return  NULL;
    }
    return(shmctl(shared_block_id,IPC_RMID, NULL) !=IPC_RESULT_ERROR);
}

int main() {
    kol* ring = (kol*)attach_memory_block("main.cpp", sizeof(kol), 0);
    if(ring == NULL){
        puts("ERROR_GET_SHARE_MEM");
        return 0;
    }
    ring->next=ring;
    ring->prev=ring;
    ring->message[0]='\0';
    ring->import = (int*) attach_memory_block("main.cpp", sizeof(int), 10);
    ring->exp = (int*) attach_memory_block("main.cpp", sizeof(int), 11);
    kol *tmp= ring;
    for(int i=1; i<10; i++){
        kol *next_tmp = (kol*)attach_memory_block("main.cpp", sizeof(kol), i);
        if(next_tmp == NULL){
            puts("ERROR_GET_SHARE_MEM");
            return 0;
        }
        next_tmp->import = (int*) attach_memory_block("main.cpp", sizeof(int), 10);
        next_tmp->exp = (int*) attach_memory_block("main.cpp", sizeof(int), 11);
        *next_tmp->import = 0;
        *next_tmp->exp =0;
        next_tmp->message[0]='\0';
        tmp->next = next_tmp;
        ring->prev = next_tmp;
        next_tmp->next = ring;
        next_tmp->prev = tmp;
        tmp = next_tmp;
    }

    sem_unlink("/semaphore");
    sem = sem_open("/semaphore", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
    if(sem == SEM_FAILED){
        perror("ERROR_OPENING_SEMAPHORE");
        return 0;
    }

    std::vector<pid_t> mnfct;
    std::vector<pid_t> cnsmr;

    struct timespec tw = {0,100000000};
    struct timespec tr;

    std::cout << "1. Create manufacturing process\n2. Create consumer process\n3. Delete manufacturing process\n4. Delete consumer process\n\nPress any key to exit\n"<<std::endl;
    while(1){
        char enter = getch();
        switch (enter) {
            case '1':{
                pid_t pid = fork();
                if(pid==-1) {
                    perror("ERROR_CREATE_PROCESS");
                    return 0;
                }
                if(pid > 0) {
                    std::cout << "Create " << mnfct.size()+1<< " manufacturing process" << std::endl;
                    mnfct.push_back(pid);
                }
                if(pid == 0) {
                    while(1){
                        sem_wait(sem);
                        kol *mnfct_tmp = ring;
                        for(int j=0; j<10;j++){
                            if(mnfct_tmp->message[0]=='\0')
                                break;
                            mnfct_tmp = mnfct_tmp->next;
                        }
                        if(mnfct_tmp->message[0]!='\0') {
                            sem_post(sem);
                            nanosleep(&tw, &tr);
                            continue;
                        }
                        int size = rand()%257;
                        for(int i =0; i<size;i++)
                            mnfct_tmp->message[i]='!'+rand()%95;
                        mnfct_tmp->message[size]='\0';
                        *mnfct_tmp->import += 1;
                        std::cout << std::endl << mnfct_tmp->message << std::endl <<  "Total produced messages: "<< *mnfct_tmp->import<< std::endl;
                        sleep(1);
                        sem_post(sem);
                        nanosleep(&tw, &tr);
                    }
                }
                break;
            }
            case '2':{
                pid_t pid = fork();
                if(pid==-1) {
                    perror("ERROR_CREATE_PROCESS");
                    return 0;
                }
                if(pid > 0) {
                    std::cout << "Create " << cnsmr.size()+1<< " consumer process" << std::endl;
                    cnsmr.push_back(pid);
                }
                if(pid == 0) {
                    while(1){
                        sem_wait(sem);
                        kol *cnsmr_tmp = ring->prev;
                        for(int j=0; j<10;j++){
                            if(cnsmr_tmp->message[0]!='\0')
                                break;
                            cnsmr_tmp = cnsmr_tmp->prev;
                        }
                        if(cnsmr_tmp->message[0]=='\0'){
                            sem_post(sem);
                            nanosleep(&tw, &tr);
                            continue;
                        }
                        *cnsmr_tmp->exp +=1;
                        std::cout << std::endl << cnsmr_tmp->message << std::endl <<  "Total ejected messages: "<< *cnsmr_tmp->exp<< std::endl;
                        cnsmr_tmp->message[0]='\0';
                        sleep(1);
                        sem_post(sem);
                        nanosleep(&tw, &tr);
                    }
                }
                break;
            }
            case '3':{
                if(!mnfct.empty()){
                    sem_wait(sem);
                    kill(mnfct[mnfct.size()-1], SIGKILL);
                    std::cout << "\n" << mnfct.size() << " manufacturing process was deleted\n" << std::endl;
                    mnfct.pop_back();
                    sem_post(sem);
                    nanosleep(&tw, &tr);
                }
                else
                    std::cout << "No one manufacturing process to delete" << std::endl;
                break;
            }
            case '4':{
                if(!cnsmr.empty()){
                    sem_wait(sem);
                    kill(cnsmr[cnsmr.size()-1], SIGKILL);
                    std::cout << "\n" << cnsmr.size() << " consumer process was deleted\n" << std::endl;
                    cnsmr.pop_back();
                    sem_post(sem);
                    nanosleep(&tw, &tr);
                }
                else
                    std::cout << "No one consumer process to delete" << std::endl;
                break;
            }
            default:{
                for(;!mnfct.empty();)
                {
                    signal(SIGCHLD, SIG_IGN);
                    kill(mnfct[mnfct.size() - 1], SIGKILL);
                    mnfct.pop_back();
                }

                for(;!cnsmr.empty();)
                {
                    signal(SIGCHLD, SIG_IGN);
                    kill(cnsmr[cnsmr.size() - 1], SIGKILL);
                    cnsmr.pop_back();
                }

                for(int i=0; i<12; i++){
                    destroy_memory_block("main.cpp", i);
                }

                sem_close(sem);
                sem_unlink("/semaphore");
                detach_memory_block(ring);
                return 0;
            }
        }
    }
}
