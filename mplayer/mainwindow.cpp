#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include<iostream>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <dirent.h>
#include <string>
#include <time.h>
#include <QMovie>
//#include <QListWidgetItem>
#include "song.h"

using namespace std;
extern struct MyPipeline{
   int fifo_fd;
   int fd[2];
}MyPipeline;

bool state = true;
bool state_bar = true;
bool sign = true;
int MainWindow::speed = 0;
int MainWindow::this_time = 0;
int MainWindow::length_time = 0;
//int MainWindow::vol = 50;

int num =1;
int vol = 50;
int song_numb = 0;
int song_new = 0;
int pattern = 2;//有序
LRC *lrc_hand = NULL;
int lrc_num = 0;
bool only_song = false;
vector<char *> song_list;

void *deal_fun1(void *);
void *deal_fun2(void *);

void player_mode(Ui::MainWindow *ui);
void lrc_handle();
//void Highlight(Ui::MainWindow *ui);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //去除边框
    //this->setWindowFlags(Qt::FramelessWindowHint);
    this->setFixedSize(1024,600);
    QMovie *movie = new QMovie(":/gifbj002.gif");
    movie->setScaledSize(QSize(260,360));
    ui->label_9->setMovie(movie);
    movie->start();

    play_state = true;
    song_new = 0;
    ui->pushButton_7->setVisible(false);
    ui->pushButton_8->setVisible(false);

    pthread_t tid1;
    pthread_t tid2;
    pthread_create(&tid1,NULL,deal_fun1,(void *)ui);
    pthread_create(&tid2,NULL,deal_fun2,NULL);
    pthread_detach(tid1);
    pthread_detach(tid2);

#if 1
    char buf[64] = "";
    sprintf(buf,"%s %d 1","volume",vol);
    printf("%s",buf);
    write(MyPipeline.fifo_fd,buf,strlen(buf));
    MainWindow::mydir();
    write(MyPipeline.fifo_fd,"pause\n",strlen("pause\n"));
#endif

}

void *deal_fun1(void *arg)
{
    Ui::MainWindow * ui = (Ui::MainWindow *)arg;
    ui->verticalSlider->setValue(vol);
    while(1)
    {
        char buf[128] = "";
        read(MyPipeline.fd[0],buf,sizeof(buf));

        char cmd[128] = "";
        float num = 0.0f;
        sscanf(buf,"%[^=]=%f",cmd,&num);

        //获得文件的播放进度,百分比
        if(strcmp(cmd,"ANS_PERCENT_POSITION") == 0)
        {
            MainWindow::speed =  (int)(num+0.5);
            //printf("\r已播放%05.2f%%\t",num);
            ui->horizontalSlider->setValue(MainWindow::speed);
            ui->label_4->setText(QString::number(MainWindow::speed));
        }
        //获得文件的当前位置，以秒为单位，精确到小数位1位
        else if(strcmp(cmd,"ANS_TIME_POSITION") == 0)
        {
            MainWindow::this_time = (int)(num*10+0.5);
            char buf[64] = "";
            sprintf(buf,"%02d:%02d.%d",MainWindow::this_time/10/60,\
                    MainWindow::this_time/10%60,MainWindow::this_time%10);
            ui->label_3->setText(QString(buf));

            if(MainWindow::this_time >= MainWindow::length_time*10-10 && MainWindow::this_time > 10)
            {
                player_mode(ui);
            }
            if(lrc_hand != NULL)
            {
                int lrc_time = myprint(lrc_hand,lrc_num)->time_min*600+\
                        myprint(lrc_hand,lrc_num)->time_sec*10+myprint(lrc_hand,lrc_num)->time_ms/10;
                if(MainWindow::this_time >= lrc_time)
                {
                    ui->label->setText(QString(myprint(lrc_hand,lrc_num)->buf));
                    if(myprint(lrc_hand,lrc_num)->next != NULL)
                        lrc_num++;
                }
            }

        }
        //获得文件的长度,以秒为单位
        else if(strcmp(cmd,"ANS_LENGTH") == 0)
        {
            MainWindow::length_time = (int)(num+0.5);
            //printf("播放总时间 %d", Widget::length_time);
            char buf[64] = "";
            sprintf(buf,"%02d:%02d",MainWindow::length_time/60,\
                    MainWindow::length_time%60);
            ui->label_5->setText(QString(buf));
        }
#if 1
        else if(strcmp(cmd,"ANS_FILENAME") == 0)
        {
            char name[128]="";
            sscanf(buf,"%[^=]=%s",cmd,name);
            ui->label_2->setText(name);
        }
#endif
        fflush(stdout);
    }
}
void *deal_fun2(void *)
{
    while(1)
    {
        while(state)
        {
            usleep(10*1000);
            write(MyPipeline.fifo_fd,"get_file_name\n",\
                  strlen("get_file_name\n"));//获得name
            usleep(10*1000);
            write(MyPipeline.fifo_fd,"get_percent_pos\n",\
                  strlen("get_percent_pos\n"));//获得百分比
            usleep(10*1000);
            write(MyPipeline.fifo_fd,"get_time_pos\n",\
                  strlen("get_time_pos\n"));//获得时间
            usleep(10*1000);
            write(MyPipeline.fifo_fd,"get_time_length\n",\
                  strlen("get_time_length\n"));//总长度
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
//绘制背景
void MainWindow::paintEvent(QPaintEvent *event)
{    
    char buf[64] = "";
    sprintf(buf,":/background/%d.jpg",num);

    //printf("%s\n",buf);
    QPainter *painter  = new QPainter(this);
    QPixmap pix;
    pix.load(buf);
    pix = pix.scaled(this->width(),this->height());
    painter->drawPixmap(0,0,pix.width(),pix.height(),pix);
    update();
}

#if 1
//更换背景
void MainWindow::on_pushButton_8_clicked()
{
    num+=1;
    if(num >7)
    {
        num = 1;

    }
    //cout<<"next picture"<<endl;
}
#endif
//下一曲
void MainWindow::on_pushButton_clicked()
{
    if(song_new >= song_numb-1)
        song_new = 0;
    else
        song_new++;
    state = true;
    play_state = false;
    char buf[64] = "";
    sprintf(buf,"%s ../mplayer/song/%s\n","loadfile",song_list[song_new]);
    write(MyPipeline.fifo_fd,buf,strlen(buf));
    usleep(1000);
    lrc_handle();
    //Highlight(ui);
}
//上一曲
void MainWindow::on_pushButton_2_clicked()
{
    if(song_new <= 0)
        song_new = song_numb-1;
    else
        song_new--;
    state = true;
    play_state = false;
    char buf[64] = "";
    sprintf(buf,"%s ../mplayer/song/%s\n","loadfile",song_list[song_new]);
    write(MyPipeline.fifo_fd,buf,strlen(buf));
    usleep(1000);
    lrc_handle();
    //Highlight(ui);
}
//暂停
void MainWindow::on_pushButton_3_clicked()
{
    QMovie *movie = new QMovie(":/cat_paper.gif");
    movie->setScaledSize(QSize(80,80));
    ui->label_8->setMovie(movie);

    if(state)
    {
        movie->stop();
        state = false;
        usleep(1000*50);
        write(MyPipeline.fifo_fd,"pause\n",strlen("pause\n"));

    }
    else if(!state)
    {
        movie->start();
        write(MyPipeline.fifo_fd,"pause\n",strlen("pause\n"));
        state = true;
        if(play_state)
        {
            char buf[128] = "";
            sprintf(buf,"%s ../mplayer/song/%s\n","loadfile",song_list[0]);
            write(MyPipeline.fifo_fd,buf,strlen(buf));
            play_state = false;
            usleep(1000);
            lrc_handle();

        }
        if(only_song)
        {
            char buf[64] = "";
            sprintf(buf,"%s ../mplayer/song/%s\n","loadfile",song_list[song_new]);
            write(MyPipeline.fifo_fd,buf,strlen(buf));
            usleep(1000);
            only_song = false;
            lrc_handle();
        }

    }
}
//进度条  滑块按下为真
void MainWindow::on_horizontalSlider_sliderPressed()
{
    state_bar = false;
}
void MainWindow::on_horizontalSlider_actionTriggered(int )
{
    if(ui->horizontalSlider->value() != MainWindow::speed)
    {
        //usleep(1000*50);
        int i = ((ui->horizontalSlider->value()-MainWindow::speed)*(MainWindow::length_time)/100);
        if(i == 0)
            return;
        char buf[128] = "";
        sprintf(buf,"%s %d\n","seek",i);
        //printf("%s",buf);
        write(MyPipeline.fifo_fd,buf,strlen(buf));
        if(!state)
            write(MyPipeline.fifo_fd,"pause\n",strlen("pause\n"));
         state_bar = true;
    }
}
//静音
void MainWindow::on_pushButton_4_clicked()
{
    mkfifo("fifo_cmd",0666);
    int fifo_fd = open("fifo_cmd",O_WRONLY);
    write(fifo_fd,"mute\n",strlen("mute\n"));
}

//音量加
void MainWindow::on_turn_up_clicked()
{
    if(vol>95)
        return;
    else
        vol +=5;
    ui->verticalSlider->setValue(vol);
    char buf[64] = "";
    sprintf(buf,"%s %d 1","volume",vol);
    write(MyPipeline.fifo_fd,buf,strlen(buf));
    ui->label_6->setText(QString::number(vol));
    //cout<<buf<<endl;

}
//音量减
void MainWindow::on_turn_down_clicked()
{
    if(vol<5)
        return;
    else
        vol -=5;
    ui->verticalSlider->setValue(vol);
    char buf[64] = "";
    sprintf(buf,"%s %d 1","volume",vol);
    write(MyPipeline.fifo_fd,buf,strlen(buf));
    ui->label_6->setText(QString::number(vol));
    //cout<<buf<<endl;

}
//音量条
void MainWindow::on_verticalSlider_actionTriggered(int)
{

    int i = ui->verticalSlider->value();
    char buf[128] = "";
    sprintf(buf,"%s %d 1","volume",i);
    write(MyPipeline.fifo_fd,buf,strlen(buf));
    ui->label_6->setText(QString::number(i));
}
//退出
void MainWindow::on_pushButton_5_clicked()
{

    write(MyPipeline.fifo_fd,"quit\n",strlen("quit\n"));

    for(int i = 0;i < song_numb;i++)
    {
        delete []song_list[i];
    }
    song_list.clear();
    this->close();
}
//打开文件
void MainWindow::mydir()
{
    int counter =ui->listWidget->count();
    for(int index=0;index<counter;index++)
    {
        //歌曲列表
        QListWidgetItem *item = ui->listWidget->takeItem(0);
        delete item;
    }
    DIR *dir = opendir("../mplayer/song");
    while(1)
    {
        struct dirent *ent =readdir(dir);
        if(ent == NULL)
            break;
        else if(ent->d_type == DT_REG)
        {
            char buf[32] = "";
            sscanf(ent->d_name,"%*[^.].%s",buf);
            if(strcmp("mp3",buf) == 0)
            {
                ui->listWidget->addItem(QString(ent->d_name));
                char *str = new char[strlen(ent->d_name)+1];
                strcpy(str,ent->d_name);
                song_list.push_back(str);
                //printf("----%s\n",ent->d_name);
            }

        }
    }
    song_numb = song_list.size();
}
//列表事件
void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    printf("%s\n",item->text().toUtf8().data());
    char buf[64] = "";
    sprintf(buf,"%s ../mplayer/song/%s\n","loadfile",item->text().toUtf8().data());
    for(int i = 0;i < song_numb;i++)
    {
        if(strcmp(song_list[i],item->text().toUtf8().data()) == 0)
        {
            song_new = i;
        }
    }
    printf("%s\n",buf);
    //Highlight(ui);
    write(MyPipeline.fifo_fd,buf,strlen(buf));
    //ui->pushButton_3->setIcon(QIcon(":/img/4.png"));
    state = true;
    play_state = false;
    usleep(1000);
    lrc_handle();
}
#if 0
void Highlight(Ui::MainWindow *ui)
{
    for(int i = 0;i<song_numb;i++)
    {
        ui->listWidget->item(i)->setTextColor(QColor(Qt::black));
        ui->listWidget->item(i)->setFlags(Qt::ItemIsEditable);
        if(strcmp(song_list[song_new],\
                  ui->listWidget->item(i)->text().toUtf8().data())==0)
        {
            ui->listWidget->item(song_new)->setTextColor(QColor(Qt::blue));
        }
    }
    //lrc_handle(ui);
}
#endif
//模式
void player_mode(Ui::MainWindow *ui)
{
    if(pattern == 0)
    {//播放一首
        only_song = true;
        return;
    }
    else if(pattern == 1)
    {//单曲循环
    }
    else if(pattern == 2)
    {//列表循环
        if(song_new >= song_numb-1)
            song_new = 0;
        else
            song_new++;

    }
    else if(pattern == 3)
    {//随机播放
        srand(time(NULL));
        song_new = rand()%song_numb;
    }
    else
        return;
    char buf[64] = "";
    sprintf(buf,"%s ../mplayer/song/%s\n","loadfile",song_list[song_new]);
    write(MyPipeline.fifo_fd,buf,strlen(buf));
    usleep(1000);
    lrc_handle();
}
void MainWindow::on_pushButton_6_clicked()
{
    if(pattern == 0)
    {
        pattern = 1;//单曲循环
        ui->pushButton_6->setIcon(QIcon(":/png/blogmarks.png"));
    }
    else if(pattern == 1)
    {
        pattern = 2;//列表循环
        ui->pushButton_6->setIcon(QIcon(":/png/blinklist.png"));
    }
    else if(pattern == 2)
    {
        pattern = 3;//随机播放
        ui->pushButton_6->setIcon(QIcon(":/png/favorites.png"));
    }
    else if(pattern == 3)
    {
        pattern = 0;//播放1
        ui->pushButton_6->setIcon(QIcon(":/png/onebit_15.png"));
    }
}
void MainWindow::on_pushButton_7_clicked()
{
    printf("刷新歌单\n");
    for(int i = 0;i < song_numb;i++)
        delete []song_list[i];
    song_list.clear();
    MainWindow::mydir();
}
void MainWindow::on_pushButton_9_clicked()
{
    static bool state = true;
    if(state)
    {
        ui->pushButton_8->setVisible(true);
        ui->pushButton_7->setVisible(true);
        state = false;
    }
    else
    {
        ui->pushButton_8->setVisible(false);
        ui->pushButton_7->setVisible(false);
        state = true;
    }
}

void lrc_handle()
{
    free_link(&lrc_hand);
    lrc_num = 0;
    char buf[64] = "";
    char str[128] = "";
    sscanf(song_list[song_new],"%[^.]",buf);
    strcat(buf,".lrc");
    sprintf(str,"../mplayer/song/%s",buf);
    //printf("%s\n",str);
    filelrc(&lrc_hand,str);
    //printf("----%d---\n",i);
    //printf("--%s--\n",lrc_hand->buf);
}




