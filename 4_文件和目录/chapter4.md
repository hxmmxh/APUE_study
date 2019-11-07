# 文件和目录

### 1.stat函数
```cpp
#include <sys/stat.h>
int stat(const char* restrict pathname, struct stat* restrict buf);
int fstat(int fd, struct stat *buf);
int lstat(const char* restrict pathname, struct stat* restrict buf);
//当命名的文件是一个符号链接时，会返回该符号链接的相关信息，而stat会返回由该符号链接引用的文件的信息
int fstatat(int fd, const char* restrict pathname, struct stat *restrict buf, int flag);
//fd为AT_FDCWD时，pathname可以是相对路径，path为绝对路径时，fd会被忽略
//flag参数为AT_SYMLINK_NOFOLLOW时，返回符号链接本身的信息，否则返回指向的实际文件的信息
struct stat
{
    mode_t st_mode;           //文件对应的模式，文件，目录等
    ino_t st_ino;             //inode节点号
    dev_t st_dev;             //设备号码
    dev_t st_rdev;            //特殊设备号码
    nlink_t st_nlink;         //文件的连接数
    uid_t st_uid;             //文件所有者
    gid_t st_gid;             //文件所有者对应的组
    off_t st_size;            //普通文件，对应的文件字节数
    struct timespec st_atime; //文件最后被访问的时间
    struct timespec st_mtime; //文件内容最后被修改的时间
    struct timespec st_ctime; //文件状态改变时间
    blksize_t st_blksize;     //文件内容对应的块大小
    blkcnt_t st_blocks;       //文件内容对应的块数量
};
struct timespec
{
    time_t tv_sec;  //秒
    long tv_nsec;   //纳秒
};
```

### 2.文件类型
* 共有7种文件类型
    1. 普通文件,regular file, S_ISREG()
    2. 目录文件, directory file, 包含了其他文件的名字以及指向这些文件有关信息的指针,S_ISDIR()
    3. 块特殊文件, block special file, 提供对设备带缓冲的访问，每次访问以固定长度为单位进行, S_ISBLK()
    4. 字符特殊文件, character special file,  提供对设备不带缓冲的访问，每次访问长度可变, S_ISCHR()
    5. FIFO, 命名管道，用于进程间通信, S_ISFIFO()
    6. 套接字,socket, 用于进程间网络通信, S_ISSOCK()
    7. 符号链接,symbolic link, 指向另一个文件的文件, S_ISLNK()
* 系统中的所有设备要么是字符特殊文件，要么是块特殊文件
* 确定文件的类型，使用宏函数，宏函数的参数是stat结构中的st_mode成员
* 设置用户ID位:一般情况下进程的有效用户ID通常是实际用户ID,但在st_mode中设置用户ID位后，会在执行此文件时，将进程的有效用户ID设置成文件所有者的用户ID,可将st_mode与屏蔽字S_IFMT进行与运算后和S_ISUID对比获得
* 设置用户组ID位，S_ISGID

### 3. 文件访问权限
* 在st_mode中包含了对文件的访问权限位
* 每个文件共有9个访问权限位，用户/组/其他+读/写/执行，S_IRUSR/S_IWUSR/S_IXUSR/S_IRGRP.../S_IROTH...
* 打开任一类型的文件时，应该对该文件路径所在的每一个目录都具有执行权限
* 对目录具有读权限可以获得在该目录中所有文件名的列表，对目录的执行权限可以使我们通过该目录
* 在一个目录中创建一个新文件，必须对该目录具有写权限和执行权限
* 删除一个现有的文件，必须要对文件的目录具有写权限和执行权限，对该文件本身不需要有读/写文件

### access和faccessat
* 一般打开一个文件时是以进程的有效用户ID和有效值ID为基础来执行访问权限测试的
* 用这两个函数可以按照实际用户ID和实际组ID进行访问权限测试
```cpp
#include <unistd.h>
int access(const char *pathname, int mode);
int faccess(int fd, const char *pathname, int mode, int flag);
//计算相对于打开目录fd的相对路径pathname
//fd取AT_FDCWD为当前目录
//flag为AT_EACCESS时检查的是有效用户ID和有效组
```
* mode取值
    * F_OK,测试文件是否存在
    * R_OK,W_OK,X_OK,读/写/执行权限

### 屏蔽字与umask
* umask为进程设置文件模式创建屏蔽字，并返回之前的值
* 更改进程的文件模式创建屏蔽字并不影响父进程中的屏蔽字
```cpp
#include <sys/stat.h>
mode_t umask(mode_t cmask);
//cmake可以由S_IRUSE等九个常量构成，也可以设置成八进制数
//0400，0200，0100用户读写执行
//0040，0020，0010组读写执行
//0004,0002,0001其他读写执行
```
* umask中设置了对应的权限位后，所对应的权限就会被拒绝

### 更改访问权限
```cpp
#include <sys/stat.h>
int chmod(const char* pathname, mode_t mode);
int fchmod(int fd, mode_t mode);
int fchmodat(int fd, const char *pathname, mode_t mode, int flag);
//计算相对于打开目录fd的相对路径pathname,fd取AT_FDCWD为当前目录
//flag参数为AT_SYMLINK_NOFOLLOW时,不会跟随符号链接
```
* 为了改变文件的权限位，进程的有效用户ID必须等于文件的所有者ID，或者该进程具有超级用户权限
* mode的可选项，有15个, 包含前面9个文件访问权限位，以及
    * S_ISUID,S_ISGID, 执行时设置用户ID/执行时设置用户组ID
    * S_ISCTX, 保存正文，粘着位
    * S_IRWXU, S_IRWXG, S_IRWXO，用户/组/其他读写执行权限全开

### 更改文件的用户ID
```cpp
#include <unistd.h>
int chown(const char *pathname, uid_t owner, gid_t group);
int fchown(int fd, uid_t owner, gid_t group);
int fchownat(int fd, const char* pathname, uid_t owner, gid_t group, int flag);
int lchown(const char *pathway, uid_t owner, gid_t group);
```
* 在符号链接情况下，lchown和设置了AT_SYMLINK_NOFOLLOW的fchowner，更改符号链接本身的所有者，其他情况都是更改所指向文件的所有者

### 文件长度
* st_size表示以字节为单位的文件的长度，只对普通文件，目录文件和符号链接有意义，部分系统对管道也定义了文件长度
* 对于目录，文件长度通常是一个数的整数倍
* 对于符号链接，文件长度是在文件名中的实际字节数
* st_blksize是对文件IO较适合的块长度，st_blocks是所分配的块数