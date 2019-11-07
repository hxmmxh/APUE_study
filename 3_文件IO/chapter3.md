# 文件IO


### 文件描述符
1. 文件描述符是一个非负整数，当打开一个现有文件或创建一个新文件时，内核向进程返回一个文件描述符
2. 通常，文件描述符0与进程的标准输入关联，1与标准输出相连，2与标准错误相连
3. 文件描述符的变化范围是0-OPEN_MAX-1

### 打开，创建，关闭文件

```cpp
#include <fcntl.h>
int open(const char *path, int oflag, ..., mode_t mode);
int openat(int fd, const char *path,int oflag,..., mode_t mode);
//若成功返回文件描述符，出错则返回-1
//path是要打开或创建的文件的名字
int creat(const char *path, mode_t mode);//等价于open(path,O_WRONLY|O_CREAT|O_TRUNC,mode)
//若成功返回只写打开的文件描述符，出错则返回-1
#include <unistd.h>
int close(int fd);
//成功返回0，出错返回-1
```
* 返回的文件描述符一定是最小的未用描述符数字
* 当一个进程终止时，内核会自动关闭它所有的打开文件
* fd参数的可选项
    * path参数指定的是绝对路径，fd参数被忽略
    * path参数指定的是相对路径名，fd参数指出相对路径名的开始地址，通过打开相对路径名所在的目录得到
    * fd指定为AT_FDCWD,path为
* oflag参数的可选项(前五项必须指定一个且只能指定一个):

|选项|含义|
|:---:|:---:|
O_RDONLY|只读
O_WRONLY|只写
O_RDWR|读写
O_EXEC|只执行打开
O_SEARCH|只搜索打开，适用于目录，目前操作系统都还没支持
O_APPEND|每次写时都追加到文件的尾端
O_CLOEXEC|
O_CREAT|若文件不存在则创建它，此时需要指定mode参数，指定该新文件的访问权限，若文件已存在则出错
O_DIRECTORY|如果path引用的不是目录，则出错
O_EXCL|文件如果不存在则创建它，是原子操作
O_NOCTTY|
O_NOFOLLOW|如果path引用的是符号链接，则出错
O_NONBLOCK|非阻塞
O_SYNC|每次写入需要等待物理IO操作完成
O_TRUNC|将文件长度截断为0
O_TTY_INIT|
O_DSYNC|
O_RSYNC|


### 文件偏移量
* 通常是非负整数，用以度量从文件开始处计算的字节数
* 在某些设备上允许出现负的偏移量
* 打开一个文件时，除非指定了O_APPEND,不然偏移量都被设置成0
* 调用lseek可以显示的为一个打开文件设置偏移量
```cpp
#include <unistd.h>
off_t lseek(int fd, off_t offset, int whence);
//若成功则返回新的文件偏移量，出错则返回-1
```
|whence取值|含义
|:---:|:---:|
SEEK_SET|把偏移量设置为距文件开始处offset个字节
SEEK_CUR|当前值加offset
SEEK_END|文件长度加offset



### 文件读写操作
* 读操作将从文件的当前偏移量开始，返回前，偏移量将增加实际读到的字节数
```cpp
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t nbytes);
//返回读到的字节数，如已到达文件的尾部，则返回0，若出错，则返回-1
ssize_t write(int fd, const void *buf, size_t nbytes)
//返回已写的字节数，若出错，则返回-1
```

### 文件共享
* 内核使用三种数据结构来表示打开的文件：
    * 每个进程都包含一张文件描述符表，包含文件描述符标志和一个指向一个文件表项的指针
    * 内核为所有打开文件维持一张文件表，每个文件表项包含：文件状态标志，当前文件偏移量，指向该文件v结点表项的指针
    * 每个打开的文件都有一个v节点结构，包含文件类型和对此文件进行操作的函数指针。大多数文件还包含i节点
* 多个进程打开同一个文件时，每个进程都有自己的文件表项，也有自己的当前文件偏移量
* 可能有多个文件描述符项指向同一个文件表项，例如fork后的父进程和子进程
* 文件描述符标志只用于一个进程的一个描述符，文件状态标志则应用于指向该给定文件表项的任何进程中的所有描述符

### 文件的原子操作
* 原子操作值的是由多步组成的一个步骤，如果该操作原子的执行，则要么执行完所有步骤，要么一步也不执行，不可能执行所有步骤的一个子集
* 任何要求多余一个函数调用的操作都不是原子操作，因为在两个函数调用之间，内核可能会临时挂起进程
```cpp
#include <unistd.h>
ssize_t pread(int fd, void* buf, size_t nbytes, off_t offset);
//相当于调用lseek后调用read,是原子操作，且不更新当前文件偏移量
ssize_t pwrite(int fd,const void *buf, size_t nbytes, off_t offset);
//相当于调用lseek后调用write
```


### 复制现有文件描述符
```cpp
#include <unistd.h>
int dup(int fd);
//返回的一定是当前可用文件描述符中的最小数值
int dup2(int fd, int fd2);
//fd为指定的新描述符的值，如果fd2已经打开，则先将其关闭，如果fd等于fd2则直接返回不关闭
```
* 返回的新文件描述符与参数fd共享同一个文件表项
* dup2是原子操作

### 缓冲操作
* 内核中设有缓冲区高速缓存或页高速缓存，大多数磁盘IO都通过缓冲区进行
* 向文件写入数据时，内核通常先将数据复制到到缓冲区中，然后排入队列，晚些时候再写入磁盘
* 当内核需要重用缓冲区来存放其他磁盘块数据时，会把所有延迟写数据块写入磁盘
```cpp
#include <unistd.h>
int fsync(int fd);
//会等待写磁盘操作结束再返回
int fdatasync(int fd);
//只影响文件的数据部分，fysnc还会同步更新文件的属性
void sync();
//将所有修改过的块缓冲区排入写队列，然后返回，不等待实际写磁盘操作结束
//成功返回0，失败返回-1
```

### 改变文件属性
```cpp
#include <fcntl.h>
int fcntl(int fd, int cmd,int arg);
//成功的返回依赖于cmd的值，失败返回-1
```
* cmd共有11中可选参数，分为5种功能，分别如下(这里列出8个)：

|cmd|含义|arg功能|返回值|补充说明|
|:--:|:---:|:---:|:---:|:---:|
|F_DUPFD|复制已有描述符|返回的新文件描述符大于等于arg值|返回新文件描述符，可用的最小值|会清除FD_CLOEXEC，共享同一文件表项，但有独立的文件描述符标志
|F_DUPFD_CLOEXEC|同上|同上|同上|会设置FD_CLOEXEC|
|F_GETFD|获取文件描述符标志|无|返回文件描述符标志|当前只定义了一个FD_CLOEXEC
|F_SETFD|设置文件描述符标志|为新标志的值|||
|F_GETFL|返回文件状态标志|||
|F_SETFL|设置文件状态标志|要设置的值|||
|F_GETOWN|获取当前接收SIGIO喝SIGURG信号的进程ID或进程组ID
|F_SETOWN|设置当前接收SIGIO喝SIGURG信号的进程ID或进程组ID|正值表示进程ID,负值表示进程组ID

### 
