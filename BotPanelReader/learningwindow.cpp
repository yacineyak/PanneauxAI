#include "learningwindow.h"
#include "histogrammewindow.h"
#include "mainwindow.h"
#include "mainwindow.h"
#include <QFileDialog>
#include <opencv2\opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QMainWindow>
#include <vector>

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <stdlib.h>
#include <opencv2/ml.hpp>

using namespace cv;
using namespace ml;
using namespace std;

learningwindow::learningwindow(QWidget *parent) :
    QMainWindow(parent),
{

    /*
     *  train the classifier for digit recognition, this could be done only one time, this method save the result in a file and
     *  it can be used in the next executions
     *  in order to train user must enter manually the corrisponding digit that the program shows, press space if the red box is just a point (false positive)
     */
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void trainDigitClassifier()
    {
        Mat thr,gray,con;
        Mat src=imread("/Users/giuliopettenuzzo/Desktop/all_numbers.png",1);
        cvtColor(src,gray,COLOR_BGR2GRAY);
        threshold(gray,thr,125,255,THRESH_BINARY_INV); //Threshold to find contour
        imshow("ci",thr);
        waitKey(0);
        thr.copyTo(con);

        // Create sample and label data
        vector< vector <Point> > contours; // Vector for storing contour
        vector< Vec4i > hierarchy;
        Mat sample;
        Mat response_array;
        findContours( con, contours, hierarchy,cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE ); //Find contour

        for( int i = 0; i< contours.size(); i=hierarchy[i][0] ) // iterate through first hierarchy level contours
        {
            Rect r= boundingRect(contours[i]); //Find bounding rect for each contour
            rectangle(src,Point(r.x,r.y), Point(r.x+r.width,r.y+r.height), Scalar(0,0,255),2,8,0);
            Mat ROI = thr(r); //Crop the image
            Mat tmp1, tmp2;
            resize(ROI,tmp1, Size(10,10), 0,0,INTER_LINEAR ); //resize to 10X10
            tmp1.convertTo(tmp2,CV_32FC1); //convert to float

            imshow("src",src);

            int c=waitKey(0); // Read corresponding label for contour from keyoard
            c-=0x30;     // Convert ascii to intiger value
            response_array.push_back(c); // Store label to a mat
            rectangle(src,Point(r.x,r.y), Point(r.x+r.width,r.y+r.height), Scalar(0,255,0),2,8,0);
            sample.push_back(tmp2.reshape(1,1)); // Store  sample data
        }

        // Store the data to file
            Mat response,tmp;
            tmp=response_array.reshape(1,1); //make continuous
            tmp.convertTo(response,CV_32FC1); // Convert  to float

            FileStorage Data("TrainingData.yml",FileStorage::WRITE); // Store the sample data in a file
            Data << "data" << sample;
            Data.release();

            FileStorage Label("LabelData.yml",FileStorage::WRITE); // Store the label data in a file
            Label << "label" << response;
            Label.release();
            cout<<"Training and Label data created successfully....!! "<<endl;

            imshow("src",src);
            waitKey(0);

    }

    /*
     *  get digit from the image given in param, using the classifier trained before
     */
    string getDigits(Mat image)
    {
        Mat thr1,gray1,con1;
        Mat src1 = image.clone();
        cvtColor(src1,gray1,COLOR_BGR2GRAY);
        threshold(gray1,thr1,125,255,THRESH_BINARY_INV); // Threshold to create input
        thr1.copyTo(con1);


        // Read stored sample and label for training
        Mat sample1;
        Mat response1,tmp1;
        FileStorage Data1("TrainingData.yml",FileStorage::READ); // Read traing data to a Mat
        Data1["data"] >> sample1;
        Data1.release();

        FileStorage Label1("LabelData.yml",FileStorage::READ); // Read label data to a Mat
        Label1["label"] >> response1;
        Label1.release();



        Ptr<ml::KNearest>  knn(ml::KNearest::create());

        knn->train(sample1, ml::ROW_SAMPLE,response1); // Train with sample and responses
        cout<<"Training compleated.....!!"<<endl;

        vector< vector <Point> > contours1; // Vector for storing contour
        vector< Vec4i > hierarchy1;

        //Create input sample by contour finding and cropping
        findContours( con1, contours1, hierarchy1,cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE );
        Mat dst1(src1.rows,src1.cols,CV_8UC3,Scalar::all(0));
        string result;

        for( int i = 0; i< contours1.size(); i=hierarchy1[i][0] ) // iterate through each contour for first hierarchy level .
        {
            Rect r= boundingRect(contours1[i]);
            Mat ROI = thr1(r);
            Mat tmp1, tmp2;
            resize(ROI,tmp1, Size(10,10), 0,0,INTER_LINEAR );
            tmp1.convertTo(tmp2,CV_32FC1);
            Mat bestLabels;
            float p=knn -> findNearest(tmp2.reshape(1,1),4, bestLabels);
            char name[4];
            sprintf(name,"%d",(int)p);
            cout << "num = " << (int)p;
            result = result + to_string((int)p);

            putText( dst1,name,Point(r.x,r.y+r.height) ,0,1, Scalar(0, 255, 0), 2, 8 );
        }

        imwrite("dest.jpg",dst1);
        return  result ;
    }

}