#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <iostream>
#include <QListWidgetItem>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void paintEvent(QPaintEvent *event);
    static int speed;
    static int this_time;
    static int length_time;



public slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();




private slots:


    void on_pushButton_4_clicked();

    void on_horizontalSlider_actionTriggered(int action);

    void on_verticalSlider_actionTriggered(int vol);

    void on_pushButton_5_clicked();

    void on_turn_up_clicked();

    void on_turn_down_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_horizontalSlider_sliderPressed();

private:
    Ui::MainWindow *ui;

    int voice_num;
    void mydir();
    bool play_state;
    QTimer *time;
};

#endif // MAINWINDOW_H
