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
    MyPipeline.fd[2];
    pipe(MyPipeline.fd);

    //int fd[2];
    //pipe(fd);

    pid_t pid = fork();
    if(pid == 0)//子进程
    {
        //创建一个管道
        mkfifo("fifo_cmd",0666);
        //将1设备重定向到fd[1]
        dup2(MyPipeline.fd[1],1);

        //使用execlp启动mplayer
        execlp("mplayer","mplayer","-idle","-slave","-quiet",\
        "-input","file=./fifo_cmd", "01.mp3",NULL);
    }
    else//父进程
    {

#if 0
        //创建一个接受mplayer回应的线程
        pthread_t mplayer_ack;
        pthread_create(&mplayer_ack,NULL,deal_fun , &fd[0]);
        //xiancheng fenli
        pthread_detach(mplayer_ack);
#endif

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
