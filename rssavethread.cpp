#include "rssavethread.h"
using namespace cv;
rssavethread::rssavethread(QObject* parent)
    : QThread(parent)
{
    mutex.lock();
    abort = false;
    mutex.unlock();
    mlColor.clear();
    mlDepth.clear();
    mlImageName.clear();

    start();
}

rssavethread::~rssavethread()
{
    mutex.lock();
    abort = true;
    mutex.unlock();
    wait();
}

void rssavethread::stop(){
    mutex.lock();
    abort = true;
    mutex.unlock();
    //wait();
}

void rssavethread::run(){
    while (!abort) //While application is running
    {
        usleep(1000);
        if ((mlColor.size()>0) && (mlDepth.size()>0) && (mlImageName.size()>0)){
            QList<cv::Mat>::iterator itImColor = mlColor.begin();
            QList<cv::Mat>::iterator itImDepth = mlDepth.begin();
            QList<std::string>::iterator itName = mlImageName.begin();
            QFileInfo check_color(qColor_path);
            QFileInfo check_depth(qDepth_path);
            if (!check_color.exists())
            {
                qDebug() << "Mkdir color::" << qColor_path;
                QDir color_dir = QDir::root();
                color_dir.mkpath(qColor_path);
            }
            if (!check_depth.exists())
            {
                qDebug() << "Mkdir depth::" << qDepth_path;
                QDir depth_dir = QDir::root();
                depth_dir.mkpath(qDepth_path);
            }

            std::string Image_name = *itName;
            cv::Mat BufColor = *itImColor;
            cv::Mat BufDepth = *itImDepth;

            std::string Color_names = qColor_path.toStdString();
            imwrite(Color_names + Image_name, BufColor);

            std::string Depth_names = qDepth_path.toStdString();
            imwrite(Depth_names + Image_name, BufDepth);

            mutex.lock();
            mlColor.pop_front();
            mlDepth.pop_front();
            mlImageName.pop_front();
            mutex.unlock();

        }
    }

}

void rssavethread::save_RGBD_mat(cv::Mat &color_mat, cv::Mat &depth_mat, qint64 timestamp){
    if (!abort){
        Mat Buf_color_mat = color_mat.clone();
        Mat Buf_depth_mat = depth_mat.clone();
        char image_name[20];
        //qint64 currTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        sprintf(image_name, "%13ld.png", timestamp);
        mutex.lock();
        mlColor.push_back(Buf_color_mat);
        mlDepth.push_back(Buf_depth_mat);
        mlImageName.push_back(image_name);
        mutex.unlock();
    }
}

void rssavethread::receive_RGBD_name(QString Subject, QString Action){
    qColor_path = QString("/home/skaegy/Data/RGBD/%1/%2/rgb/").arg(Subject).arg(Action);
    qDepth_path = QString("/home/skaegy/Data/RGBD/%1/%2/depth/").arg(Subject).arg(Action);
}
