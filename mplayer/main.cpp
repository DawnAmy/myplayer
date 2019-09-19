#include "mainwindow.h"
#include <QApplication>

#include<stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

struct MyPipeline
{
    int fifo_fd;
    int fd[2];
}MyPipeline;

int main(int argc, char *argv[])
{

    //创建一个无名管道 获取mplayer回应

    pipe(MyPipeline.fd);
    pid_t pid = fork();
    if(pid == 0)//子进程
    {
        //创建一个管道
        mkfifo("fifo_cmd",0666);
        //将1设备重定向到fd[1]
        dup2(MyPipeline.fd[1],1);

        //使用execlp启动mplayer
        execlp("mplayer","mplayer","-idle","-slave","-quiet",\
        "-input","file=./fifo_cmd", "00.mp3",NULL);
    }
    else//父进程
    {
        //创建一个管道
        mkfifo("fifo_cmd",0666);
        MyPipeline.fifo_fd = open("fifo_cmd",O_WRONLY);

        //ui设计
        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        return a.exec();
    }
    return 0;
}
