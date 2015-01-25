#include "PuzzleFunctions.h"

//Mat(*PuzzleSolver)(Mat*,int)=&SolvePuzzle;
Mat(*PuzzleSolver)(Mat*,int)=&FindMatchingRows;
//Mat(*PuzzleSolver)(Mat*,int)=&FindMatchingCols;

int main(int argc,char **argv){

	Mat *FilesTwo=nullptr;

		string path1="c:/IP/IP/3x3/IMAGE3/";
		string path2="c:/IP/IP/4x4/IMAGE1/";
		string path3="c:/testip/testip/TESTIP1/";
		string path4="c:/IP-TEST/IP-TEST/3x3/1/";
		string path5="c:/IP-TEST/IP-TEST/4x4/3/";
		string *path=&path4;
	double successRate=0;
	int failcounter=0;
	for (int i = 1; i <= NUM; i++)
	{string path("C:/Users/SAMSUNG/Desktop/TESTDIP/TESTDIP/");

	try{
		int current=25;
		path=path+" ("+to_string(i)+").JPEG";
		Mat test=imread(path,0);
		//SharpenImage(test,2);//
		if(test.data){
			FixImgSize(test,current);
		}
		else 
			throw exception ("main::Invalid File");

		FilesTwo=CreatePuzzle(test,current);
		Mat sol;

	if(FilesTwo!=nullptr){
		sol=PuzzleSolver(FilesTwo,current);
		delete [] FilesTwo;
		FilesTwo=nullptr;
	}
		//ShowImg(sol);
	if(compareImgaes(test,sol)){
		successRate++;
	cout<<"Done Successfully, "<<i<<endl;
	}
	else{
		cout<<"False, "<<i<<endl;
	}

	}
	catch(exception e){
		cout<<e.what()<<endl;
	cout<<"Can't find Sol"<<endl;
	failcounter++;
	if(FilesTwo!=nullptr)
		delete [] FilesTwo;
		FilesTwo=nullptr;
	}
	
	}
	if(NUM){
	cout<<"success rate is:"<<successRate/NUM<<endl;
	cout<<"fails rate is:"<<(double)failcounter/NUM<<endl;
	cout<<"false rate is:"<<1-(double)failcounter/NUM-successRate/NUM<<endl;

	}//if images has been read 
	try{
	//ShowImg(ReadAndSolve(*path,PuzzleSolver));
	}
	catch(exception e){
		cout<<e.what()<<endl;
	}
	return 0;
	
} 

 