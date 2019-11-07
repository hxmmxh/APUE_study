#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

using namespace std;

#define FILE_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
#define BUFFSIZE 4096

void testStdIO()
{
    int n;
    char buf[BUFFSIZE];
    while((n=read(STDIN_FILENO,buf,BUFFSIZE))>0)
        if(write(STDOUT_FILENO,buf,n)!=n)
            cout << "write error";   
}

void testFileIO() 
{
    int fd;
    fd = creat("hello", FILE_MODE);
    char buf[BUFFSIZE] = "123456789";
    write(fd, buf, BUFFSIZE);
    cout << lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);
    char buf2[BUFFSIZE];
    read(fd, buf2, BUFFSIZE);
    cout << buf2;
}

int main()
{
    //testStdIO();
    testFileIO();
}