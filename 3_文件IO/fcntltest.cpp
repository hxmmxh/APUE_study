#include <iostream>
#include <fcntl.h>

using namespace std;

int main(int argc, char *argv[])
{
    int val;
    val = fcntl(atoi(argv[1]), F_GETFL, 0);
    switch (val&O_ACCMODE)
    {
    case O_RDONLY:
        cout << "read only";
        break;
    case O_WRONLY:
        cout << "write only";
        break;
    case O_RDWR:
        cout << "read write";
        break;
    default:
        cout << "unkown access mode";
        break;
    }
    if(val&O_APPEND)
        cout << ", append";
    if(val&O_NONBLOCK)
        cout << ", nonblocking";
    if(val&O_SYNC)
        cout << ", synchronous writes";
    cout << '\n';

}