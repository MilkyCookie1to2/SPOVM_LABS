#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fstream>

using namespace std;

int main(int argc, char **argv, char **envp) {
    cout << endl;
    cout << "NAME: "<< argv[0] << endl;
    cout << "PID: "<<getpid() << endl;
    cout << "PPID: " <<getppid() << endl;
    fstream f;
    f.open(argv[1],ios::in);
    if(!f.is_open())
    {
        cout << "ERROR: No such file or permission denied" << endl;
        return 0;
    }
    string str;
    while(f>>str)
    {
        for(char **env = envp; *env != 0; env++)
        {
            if(strncmp(*env,(str+"=").c_str(),(str+"=").length())==0)
                cout << *env << endl;
        }
    }
    f.close();
    exit(EXIT_SUCCESS);
}
