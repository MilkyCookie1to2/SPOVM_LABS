#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

bool access_stdout = false;

void change_access(int signum)
{
    access_stdout = true;
}


int main (int argc, char *argv[]) {
    signal(SIGUSR2, change_access);
    if(strcmp(argv[0],"true")==0)
        access_stdout = true;
    else
        access_stdout =false;
    const char *str="Process: ";
    while(1)
    {
        if(access_stdout)
        {
            for(int i=0; i< strlen(str); i++)
                printf("%c",str[i]);
            printf("%d\n", getpid());
            sleep(1);
            access_stdout = false;
            kill(getppid(), SIGUSR1);
        }
    }
}
