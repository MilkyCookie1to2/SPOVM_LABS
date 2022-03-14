#include <iostream>
#include <cstdlib>
#include <list>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

using namespace std;

extern char **environ;

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

char** build_name_proc(char** args, int number)
{
    *args = (char*) calloc(9,sizeof (char ));
    strcpy(args[0], "child_");
    if(number>=0 && number<=9)
    {
        strcat(args[0],"0");
        strcat(args[0], to_string(number).c_str());
    } else
        strcat(args[0], to_string(number).c_str());
    return args;
}

int main(int argc, char **argv, char **envp) {
    int number = 0;
    char **args = (char**) calloc(2,sizeof (char*));
    if(argc==2)
    {
        args[1] = (char *) calloc(sizeof(argv[1]),sizeof (char));
        strcpy(args[1], argv[1]);
    }
    else
    {
        cout << "ERROR: Invalid arguments" << endl;
        return 0;
    }
    list<string> listenv;
    for(char **env = envp; *env != 0; env++)
    {
        char *thisEnv = *env;
        listenv.emplace_back(thisEnv);
    }
    listenv.sort();
    for(string var:listenv)
        cout << var << endl;
    int status;
    while (1) {
        char h=getch();
        if(h=='+' || h=='*' || h=='&') {
            build_name_proc(args, number);
            number++;
            pid_t pid = fork();
            if (pid > 0) {
                wait(&status);
                cout << endl<<args[0]<<" exit with status "<<status << endl;
                if (h == '&')
                    return 0;
                else
                    continue;
            }
            if (pid==0)
            {
                switch (h) {
                    case '+': {
                        char *path = getenv("CHILD_PATH");
                        if (!path) {
                            cout << "ERROR: Environment variable isn't set" << endl;
                            return 0;
                        }
                        strcat(path, "/child");
                        execve(path, args, envp);
                    }
                    case '*': {
                        char *path = NULL;
                        for (int i = 0; envp[i]; i++) {
                            if (strstr(envp[i], "CHILD_PATH")) {
                                path = (char *) calloc(strlen(envp[i]) - 11, sizeof(char));
                                strcat(path, envp[i] + 11);
                                strcat(path, "/child");
                            }
                        }
                        if (!path) {
                            cout << "ERROR: Environment variable isn't set" << endl;
                            return 0;
                        }
                        execve(path, args, envp);
                    }
                    case '&': {
                        char *path = NULL;
                        for (int i = 0; environ[i]; i++) {
                            if (strstr(environ[i], "CHILD_PATH")) {
                                path = (char *) calloc(strlen(environ[i]) - 11, sizeof(char));
                                strcat(path, environ[i] + 11);
                                strcat(path, "/child");
                            }
                        }
                        if (!path) {
                            cout << "ERROR: Environment variable isn't set" << endl;
                            return 0;
                        }
                        execve(path, args, envp);
                    }
                    default:
                        return 0;
                }
            }
        }
        return 0;
    }
    return 0;
}
