#pragma once
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<boost/lambda/lambda.hpp>
#include<boost/dirent.h>
#include<iostream>
#include <utility>  
#include<list>
#include<vector>
#include<math.h>

#define THOLD 1000000000000
#define THOLDAVG 4
//-----------------------------------------------
//parms
#define NUM 100

//-----------------------------------------------
using namespace std;
using namespace cv;
enum directions {RIGHT,LEFT,UP,DOWN};
enum indices {AVG,DIFF,EQL,COV,DEV,HIST,GRAD,DIFFSUM};
enum states{DURING,UNUSED,DONE,ROW_DONE};//These are the states of the current image
enum orientation{HOZ,VERT};
enum methods{L1,L2};

vector<unsigned int> FindMatchingEdges(Mat *A,int size);
Mat reAssamble(Mat A[9],vector<string> matches);
double * CompareEdges(Mat Ed1,Mat Ed2,const short method);
vector<Mat> *ExtEdges(Mat img);
Mat FindMatchingRows(Mat *imgarr,int size);
Mat FindMatchingCols(Mat *imgarr,int size);
Mat* GetImages(string path,unsigned int size);
vector<string>& GetFileNames(string path);
Mat* CreatePuzzle(Mat img,int size);
int *calcHisto(Mat img);
Mat SolvePuzzle(Mat* imgarr,int size);
pair<int,pair<int,int>> getBestMatch(Mat *images,unsigned int * notused,int size,int side=HOZ);//first is the index and second is the side 
pair <Mat,Mat>* CalcSobelDerrivetives(Mat src);
void SharpenImages(Mat* imarr,const double alpha,int size);
void SharpenImage(Mat &img,double alpha);
void FixImgSize(Mat &img,short Num);
bool compareImgaes(const Mat &im1,const Mat & im2);
void ShowImg(const Mat &Img);
bool checkSquereNum(int Num);
Mat ReadAndSolve(string path,Mat (*Solver)(Mat *,int));
