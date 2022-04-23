#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <time.h>
#include <vector>

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

pthread_mutex_t mutex;

void *print_thread_info(void *arg)
{
    int id = *(int *) arg;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    char text[]="This thread is ";
    struct timespec tw = {0,100000000};
    struct timespec tr;
    while(1) {
        pthread_mutex_lock(&mutex);
        //const char*exit_th = "SUCCESS";
        for(int i=0; text[i]!='\0'; i++)
        {
            printf("%c\n", text[i]);
            nanosleep(&tw,&tr);
        }
        printf("%d\n", id);
        pthread_mutex_unlock(&mutex);
        nanosleep(&tw,&tr);
    }
}

int main()
{
    struct timespec tw = {0,100000000};
    struct timespec tr;
    printf("Enter + to create thread\n");
    int id=0;
    std::vector<pthread_t> threads;
    pthread_mutex_init(&mutex, NULL);
    while(1) {
        char vvod = getch();
        switch (vvod) {
            case '+': {
                pthread_mutex_lock(&mutex);
                printf("\nCreate thread %d\n", id+1);
                pthread_t thread;
                if(pthread_create(&thread, NULL, print_thread_info, &id)!=0)
                    perror("\nError create thread\n");
                else
                {
                    id++;
                    threads.push_back(thread);
                }
                pthread_mutex_unlock(&mutex);
                nanosleep(&tw,&tr);
                break;
            }
            case '-': {
                pthread_mutex_lock(&mutex);
                printf("\nDelete thread %d\n", id);
                id--;
                pthread_cancel(threads[threads.size()-1]);
                threads.pop_back();
                pthread_mutex_unlock(&mutex);
                nanosleep(&tw,&tr);
                break;
            }
            default: {
                pthread_mutex_destroy(&mutex);
                return 0;
            }
        }
    }
    return 0;
}

