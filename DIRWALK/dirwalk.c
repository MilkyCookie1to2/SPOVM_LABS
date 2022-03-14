#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

void print_dir(char *path, char* args)
{
    struct dirent **namelist;
    int n;
    if(args && strstr(args,"s"))
        n=scandir(path, &namelist, NULL, alphasort);
    else
        n= scandir(path,&namelist, NULL, NULL);
    if(n<0)
    {
        puts("ERROR: No such directory");
        return;
    }
    for(int i=0;i<n;i++)
    {
        if(!args || strcmp(args,"s")==0) {
            if(strcmp(namelist[i]->d_name,".")!=0 && strcmp(namelist[i]->d_name,"..")!=0 ) {
                printf("%s", path);
                puts(namelist[i]->d_name);
                if (namelist[i]->d_type == DT_DIR) {
                    char *path1 = (char *) calloc((sizeof(path) + sizeof(namelist[i]->d_name)), sizeof(char));
                    path1 = strcat(path1, path);
                    path1 = strcat(path1, namelist[i]->d_name);
                    path1 = strcat(path1, "/");
                    print_dir(path1, args);
                    continue;
                }
            }
            continue;
        }
        if(strstr(args,"l")&&namelist[i]->d_type==DT_LNK) {
            printf("%s", path);
            puts(namelist[i]->d_name);
            continue;
        }
        if(strstr(args,"f")&&namelist[i]->d_type==DT_REG)
        {
            printf("%s",path);
            puts(namelist[i]->d_name);
            continue;
        }
        if(namelist[i]->d_type==DT_DIR)
        {
            if(strcmp(namelist[i]->d_name,".")!=0 && strcmp(namelist[i]->d_name,"..")!=0 ) {
                if(strstr(args,"d")) {
                    printf("%s", path);
                    puts(namelist[i]->d_name);
                }
                char *path1=(char*) calloc((sizeof (path) + sizeof (namelist[i]->d_name)),sizeof (char ));
                path1 = strcat(path1, path);
                path1 = strcat(path1,namelist[i]->d_name);
                path1 = strcat(path1,"/");
                print_dir(path1, args);
            }
            continue;
        }
    }
}

int main(int argc, char* argv[]) {
    char *arg=NULL;
    char buf[PATH_MAX]; /* PATH_MAX incudes the \0 so +1 is not required */
    char *dir = realpath("./", buf);
    strcat(dir, "/");
    for(int i=1;i<argc;i++)
    {
        if(strcmp(argv[i],"-type")==0)
        {
            i++;
            if(argv[i])
            {
                for(int j=0; argv[i][j]!='\0';j++)
                {
                    if(!arg)
                        arg = (char *) calloc(1,sizeof(char));
                    if(argv[i][j]=='l')
                    {
                        if(strstr(arg,"l")==NULL)
                            arg = strcat(arg,"l");
                        continue;
                    }
                    if(argv[i][j]=='f')
                    {
                        if(strstr(arg,"f")==NULL)
                            arg = strcat(arg,"f");
                        continue;
                    }
                    if(argv[i][j]=='d')
                    {
                        if(strstr(arg,"d")==NULL)
                            arg = strcat(arg,"d");
                        continue;
                    }
                    puts("ERROR: Wrong input type");
                    free(arg);
                    return 0;
                }
                continue;
            }
            else
            {
                puts("ERROR: Wrong input type");
                return 0;
            }
        }
        if(argv[i][0]=='-')
        {
            for(int j=1; argv[i][j]!='\0';j++)
            {
                if(!arg)
                    arg = (char *) calloc(1,sizeof(char));
                if(argv[i][j]=='l')
                {
                    if(strstr(arg,"l")==NULL)
                        arg = strcat(arg,"l");
                    continue;
                }
                if(argv[i][j]=='f')
                {
                    if(strstr(arg,"f")==NULL)
                        arg = strcat(arg,"f");
                    continue;
                }
                if(argv[i][j]=='d')
                {
                    if(strstr(arg,"d")==NULL)
                        arg = strcat(arg,"d");
                    continue;
                }
                if(argv[i][j]=='s')
                {
                    if(strstr(arg,"s")==NULL)
                        arg = strcat(arg,"s");
                    continue;
                }
                puts("ERROR: Wrong input type");
                free(arg);
                return 0;
            }
            continue;
        }
        if(i==1) {
            dir = argv[1];
            continue;
        }
        puts("ERROR: Wrong input");
        return 0;
    }
    print_dir(dir,arg);
    return 0;
}
