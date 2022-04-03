#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <vector>k
#include <stdio.h>

using namespace std;

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

struct pid_stat
{
    pid_t  pid;
    bool access;
};

vector<pid_stat> pids;

void manager_access_stdout(int signum)
{
    if(pids.size()==1)
        kill(pids[0].pid, SIGUSR2);
    if(pids.size()>1)
    {
        for(int i = 0; i<pids.size();i++)
        {
            if(pids[i].access)
            {
                pids[i].access=false;
                if(i+1 == pids.size())
                {
                    pids[0].access=true;
                    kill(pids[0].pid, SIGUSR2);
                    return;
                }
                else
                {
                    pids[i+1].access=true;
                    kill(pids[i+1].pid, SIGUSR2);
                    return;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    struct sigaction
    signal(SIGUSR1,manager_access_stdout);
    while(1)
    {
        char vvod = getch();
        switch(vvod)
        {
            case '+':
            {
                pid_t pid = fork();
                if(pid == -1){
                    cout << "ERROR: process wasn't create" << endl;
                    return 0;
                }
                if(pid>0) {
                    cout << "Process C_" << pids.size() << " was created"<< endl;
                    pid_stat data_c_pid;
                    data_c_pid.pid = pid;
                    if(pids.size()==1) {
                        data_c_pid.access = true;
                    }
                    else
                        data_c_pid.access = false;
                    pids.push_back(data_c_pid);
                }
                if(pid == 0)
                {
                    char **arg = (char**) calloc(1, sizeof(char*));
                    if(pids.empty())
                        arg[0]="true";
                    else
                        arg[0]="false";
                    execv("child",arg);
                }
                break;
            }
            case '-':
            {
                if(!pids.empty()) {
                    kill(pids[pids.size() - 1].pid, SIGKILL);
                    if(pids[pids.size()-1].access)
                    {
                        pids[0].access = true;
                        kill(pids[0].pid, SIGUSR2);
                    }
                    cout << "Process C_" << pids.size()-1 << " was killed"<< endl;
                    pids.pop_back();
                    cout << pids.size() << " processes left" << endl;
                }
                else
                    cout << "Child processes wasn't created" << endl;
                break;
            }
            case 'k':
            {
                for(;!pids.empty();)
                {
                    kill(pids[pids.size() - 1].pid, SIGKILL);
                    pids.pop_back();
                }
                cout << "All processes was killed" << endl;
                break;
            }
            default: {
                for(;!pids.empty();)
                {
                    signal(SIGCHLD, SIG_IGN);
                    kill(pids[pids.size() - 1].pid, SIGKILL);
                    pids.pop_back();
                }
                cout << "Exiting..." << endl;
                exit(0);
            }
        }
    }
}
