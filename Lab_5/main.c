#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

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

void *print_thread_info(void *arg)
{
    int id = *(int*)arg;
    const char*exit_th = "SUCCESS";
    printf("\nIt's %d thread!\n",id);
    printf(">>Hello! !'m %d thread.\n",id);
    pthread_exit(exit_th);
}

int main()
{
    int id=0;
    void *exit_th;
    printf("Enter + to create thread\n");
    while(1) {
        char vvod = getch();
        switch (vvod) {
            case '+': {
                printf("\nCreating thread %d\n", id);
                pthread_t thread;
                if(pthread_create(&thread, NULL, print_thread_info, &id) != 0)
                    perror("Error create thread");
                else
                {
                    pthread_join(thread, &exit_th);
                    printf("Thread exit with %s\n", exit_th);
                    id++;
                }
                break;
            }
            default:
                return 0;
        }
    }
    return 0;
}