#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>


bool daemon() {
    int fd;

    switch (fork()) {
        case -1:
            // fork 出错，直接返回
            return false;
        case 0:
            // 第一个子进程
            break;
        default:
            // 父进程直接退出
            exit(0);
    }

    // setsid() 函数会创建一个新会话，目的是让上面创建的第一个子进程脱离控制终端和进程组，并让这个子进程成为会话组组长，与原来的登陆会话和进程组脱离
    // 到这，第一个子进程已经成为无终端的会话组组长，但是对于一个会话组组长来说，它依然能够重新申请打开一个控制终端
    if (setsid() == -1 ) {
        // setsid 失败，直接返回
        return false;
    }

    // 第二次 fork
    // 目的是为了让进程不再成为会话组长，避免它重新申请打开控制终端
    switch (fork()) {
        case -1:
            // fork 出错，直接返回
            return false;
        case 0:
            // 第二个子进程产生
            break;
        default:
            // 第一个子进程退出
            exit(0);
    }

    // 重新设置文件权限的掩码，目的是去除终端用户的默认文件、文件夹权限，比如 centos 普通用户 umask 为 0022，默认创建的文件权限为 644，文件夹权限为 755
    // 如果重新设置掩码 umask 为 0 的话，那么默认创建的文件权限就是 666，文件夹就是 777
    umask(0);


    // 守护进程不应该在终端有任何的输出，使用 dup2 函数把 stdin,stdout,stderr 重定向到 /dev/null
    fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        return false;
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
        return false;
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        return false;
    }

#if 0
    if (dup2(fd, STDERR_FILENO) == -1) {
        return;
    }
#endif

    if (fd > STDERR_FILENO) {
        if (close(fd) == -1) {
            return false;
        }
    }

    return true;
}

void do_work() {
    FILE *fp = fopen("/tmp/daemon.log", "w");
    if (!fp) {
        return;
    }
    while (true) {
        fprintf(fp, "ABCDEDFGH\n");
        fflush(fp);
        sleep(10);
    }
    fclose(fp);
}


int main() {

    if (!daemon()) {
        return -1;
    }

    do_work();

    return 0;
}
