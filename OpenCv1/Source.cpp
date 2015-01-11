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
#define EDGE_SIZE 1
using namespace std;
using namespace cv;

enum directions {RIGHT,LEFT,UP,DOWN};
enum indices {AVG,DIFF,EQL,COV,DEV,HIST};
enum states{DURING,UNUSED,DONE,ROW_DONE};//These are the states of the current image
enum orientation{HOZ,VERT};
int MeasureBy=COV;

vector<string> *FindMatchingEdges(Mat A[9]);
Mat reAssamble(Mat A[9],vector<string> matches);
double * CompareEdges(Mat Ed1,Mat Ed2,const short th);
vector<Mat> *ExtEdges(Mat img);
Mat FindMatchingRows(Mat *imgarr,int size);
Mat* GetImages(string path,int size);
vector<string>& GetFileNames(string path);
Mat* CreatePuzzle(Mat img);
int *calcHisto(Mat img);
Mat SolvePuzzle(Mat* imgarr,int size);
pair<int,pair<int,int>> getBestMatch(Mat *images,unsigned int * notused,int size,int side=HOZ);//first is the index and second is the side 

//-------------------------------------------------------------
int main(int argc,char **argv){

	if(argc!=2){
		cout<<"Enter the full path to the image directory"<<endl;
		//return -1;
	}
	//vector<string> FilePath=GetFileNames("c:/example");
	Mat* Files;
	Mat *FilesTwo;
	int pieces=16;
	try{
		//Files= GetImages("c:/IP/IP/3x3/IMAGE1/",pieces);
		FilesTwo=GetImages("c:/IP/IP/4x4/IMAGE3/",pieces);
	}
	catch(exception e){
	cout<<e.what();
	return 0;
	}
	try{
		Mat sol=SolvePuzzle(FilesTwo,pieces);
		namedWindow("Display1",WINDOW_AUTOSIZE);
		imshow("Display1",sol);
	}
	catch (exception e){
	cout<<e.what();
	
	}

	waitKey(0);
	return 0;
	
} 


//Compares two images, used to compare two edges although works for matrices as well
//th is a treshold for the comparison operation if the difference is greater then the th then the vectors are not equal
//returns an array indicating the difference between the two edges by avg value of each vector
//and by the number of differences higher then a scalar x

double* CompareEdges(Mat Ed1,Mat Ed2,const short th){
	if(Ed1.cols!=Ed2.cols&&Ed1.rows!=Ed2.rows){
		cout<<Ed1.size()<<","<<Ed2.size();

		throw  exception("The edges size is different");
	}
	double *res=new double[5];
	double avgEd1=0,avgEd2=0;//avg value
	float deved1=0,deved2=0;
	 int *H1 =calcHisto(Ed1);
	int *H2=calcHisto(Ed2);
	int result=0;
	for (int i = 0; i < 256; i++)
	{
		int tempRes=H1[i]-H2[i];
		result+=abs(tempRes);
	}
	double cov=0;
	int difference1=0,difference2=0;
	Mat A=abs(Ed1-Ed2);
		int equality=sum(A)[0];
	int len=Ed1.cols*Ed1.rows;
	for (int i = 0; i < Ed1.cols; i++)
	{
		for (int j = 0; j < Ed1.rows; j++)
		{
			avgEd1+=(double)Ed1.at<uchar>(j,i)/len;
			avgEd2+=(double)Ed2.at<uchar>(j,i)/len;
			cov+=(abs(avgEd2-Ed2.at<uchar>(j,i))*abs(avgEd1-Ed1.at<uchar>(j,i)))/len;
			if(avgEd1>avgEd2)
			{
				difference1++;
			}
			else
				difference2++;
	}

	}
	deved1=sqrt(deved1);//deviation from the mean edge1
	deved2=sqrt(deved2);//deviation from the mean edge2
	res[DIFF]=abs(difference1-difference2);
	res[AVG]=abs(avgEd1-avgEd2);
	res[EQL]=equality;
	res[COV]=cov;
	res[DEV]=abs(deved1-deved2);
	res[HIST]=result;
	return res;
}
//----------------------------------------------------------------------------------------
//Extracting the Edges of a single image
//returns a vector of size 4 and it contain 1 px wide matrices which represent the 4 edges of an image; 

vector<Mat> *ExtEdges(Mat img){

	vector<Mat>* Edges=new vector<Mat>;
	Mat edgeR,edgeL,edgeU,edgeD;
	edgeL=img(Rect(0,0,EDGE_SIZE,img.rows));
	edgeR=img(Rect(img.cols-EDGE_SIZE,0,EDGE_SIZE,img.rows));
	edgeU=img(Rect(0,0,img.cols,EDGE_SIZE));
	edgeD=img(Rect(0,img.rows-EDGE_SIZE,img.cols,EDGE_SIZE));

	Edges->push_back(edgeR);//Edges[0]right
	Edges->push_back(edgeL);//Edges[1]left
	Edges->push_back(edgeU);//Edges[2]up
	Edges->push_back(edgeD);//Edges[3]down
	return Edges;
}

//---------------------------------------------------------------------------------------//find
//returns the Full Image
Mat FindMatchingRows(Mat* imgarr,int size){//throws exception on error
	int newSize=(int)pow(size,0.5);

	if(newSize*newSize!=size){
		throw exception("The number pieces is not a squere number");//cant work;
	}	

	int *notused=new int[size];
	for (int i = 0; i < size; i++){notused[i]=UNUSED;}//nullify array
	int finalsize=imgarr[0].cols*newSize;
	double edgeL=THOLD;
	double edgeR=THOLD;
	
	unsigned short *rows=new unsigned short[newSize];
	for(int j=0;j<newSize;j++)//for all of the rows 
	{
		int x=-1; 
		for(int z=0;z<size;z++)//looking for unused Images
			if(notused[z]==UNUSED)//
			{x=z;//setting the current index as x
				notused[x]=DURING;//Image being worked on right now
				break;
			}//
		int counter=0;
	while(imgarr[x].cols!=finalsize&&counter!=1000){
		counter++;//safety feature
		//making the full rows 
		for (int q = 0; q < size; q++)
		{
			cout<<notused[q]<<",";//debug
		}
		cout<<endl;//debug
	vector<Mat> edgesa=*ExtEdges(imgarr[x]);
	edgeR=THOLD;
	edgeL=THOLD;
	short bestMatchLR=-1;
	short bestMatchRL=-1;
	for(int i=0;i<size;i++){//looking for the image 
		vector<Mat> edgesb=*ExtEdges(imgarr[i]);//looking for a match
		if(notused[i]==UNUSED&&notused[x]==DURING){

			if(CompareEdges(edgesa[LEFT],edgesb[RIGHT],4)[MeasureBy]<edgeL)//checking match on the left side
			{
				edgeL=CompareEdges(edgesa[LEFT],edgesb[RIGHT],4)[MeasureBy];
				bestMatchLR=i;
			}
			 if(CompareEdges(edgesa[RIGHT],edgesb[LEFT],4)[MeasureBy]<edgeR)//checking match on the right side
			 {
					edgeR=CompareEdges(edgesa[RIGHT],edgesb[LEFT],4)[MeasureBy];//set edgeR to the current best
					bestMatchRL=i;//keep the index of the best match on the right side
				}

	}//end if(notused[i]==0&&notused[x]==2)
	}//end for(int i=0;i<9;i++)
	//connect the images

		if(edgeR<=edgeL){//right is better then left for x 
			if(bestMatchRL==bestMatchLR&&bestMatchLR!=-1){//if the peices are the same
					hconcat(imgarr[x],imgarr[bestMatchRL],imgarr[x]);//connect the x right side to the matches left side
					notused[bestMatchLR]=DONE;
				}
			else if(bestMatchRL>=0)
		 {//found a match on the right side
			hconcat(imgarr[x],imgarr[bestMatchRL],imgarr[x]);
			notused[bestMatchRL]=DONE;//setting the used images as -1 in the notused array
		}
			}
		else{
			if(bestMatchRL==bestMatchLR&&bestMatchLR!=-1){
					hconcat(imgarr[bestMatchLR],imgarr[x],imgarr[x]);//concat to the left side store it in the spot 
					notused[bestMatchLR]=DONE;//setting the used images as -1 in the notused array
					
			}
			else if(bestMatchLR>=0){//found a match on the left side
			hconcat(imgarr[bestMatchLR],imgarr[x],imgarr[x]);//concat to the left side store it in the spot 
			notused[bestMatchLR]=DONE;//setting the used images as -1 in the notused array
		}
		
		}

	}//end while(imgarr[x].cols!=finalsize)
	notused[x]=ROW_DONE;
	rows[j]=x;
	
	}//end for(int j=0;j<3;j++)
	//***********************************************************
	//phase one complete //rows are constructed now and all ready for phase two
	//connect the rows together
	
	int finalRows=imgarr[rows[0]].rows*newSize;
				//vector<Mat> edgeA=*ExtEdges(imgarr[rows[1]]);//always take the first image and build the full image up on it 
				//vector<Mat> edgeB=*ExtEdges(imgarr[rows[2]]);
				//cout<<"************"<<CompareEdges(edgeA[UP],edgeB[DOWN],THOLDAVG)[MeasureBy]<<"************"<<endl;
				//cout<<"************"<<CompareEdges(edgeA[DOWN],edgeB[UP],THOLDAVG)[MeasureBy]<<"************"<<endl;
				free(notused);
				notused=new int[newSize];
				for (int i = 0; i < newSize; i++)notused[i]=UNUSED;//initialize array 
	unsigned int counter=0;
	while(imgarr[rows[0]].rows!=finalRows&&counter!=100){
		unsigned long bestU=THOLD;
		unsigned long bestD=THOLD;
		int idxD=-1,idxU=-1;
		counter++;//safety
	for (int i =1; i < newSize; i++)
	{//for each row 
		if(notused[i]==UNUSED){
		vector<Mat> edgeA=*ExtEdges(imgarr[rows[0]]);//always take the first image and build the full image up on it 
		vector<Mat> edgeB=*ExtEdges(imgarr[rows[i]]);//extract the i row in the 


		if(bestU>CompareEdges(edgeA[UP],edgeB[DOWN],THOLDAVG)[MeasureBy])
		{
			bestU=CompareEdges(edgeA[UP],edgeB[DOWN],THOLDAVG)[MeasureBy];
			idxU=i;
		}
		
		if(bestD>CompareEdges(edgeA[DOWN],edgeB[UP],THOLDAVG)[MeasureBy]){
			bestD=CompareEdges(edgeA[DOWN],edgeB[UP],THOLDAVG)[MeasureBy];
			idxD=i;
		}
			}
	}
	
	if(bestD>bestU&&idxU!=-1){
				vconcat(imgarr[rows[idxU]],imgarr[rows[0]],imgarr[rows[0]]);
				notused[idxU]=DONE;
			cout<<"**********"<<bestD<<","<<bestU<<"**********"<<endl;
			cout<<"******U****"<<idxD<<","<<idxU<<"**********"<<endl;
			}
	else if(idxD!=-1){
				vconcat(imgarr[rows[0]],imgarr[rows[idxD]],imgarr[rows[0]]);
				notused[idxD]=DONE;
			cout<<"**********"<<bestD<<","<<bestU<<"**********"<<endl;
			cout<<"*****D*****"<<idxD<<","<<idxU<<"**********"<<endl;
			}

	
	}
	int idx=rows[0];
	free(rows);
	free(notused);
	return imgarr[idx];
}

//------------------------------------------------------------------------------------------------

Mat SolvePuzzle(Mat* imgarr,int size){//throws exception
	int newSize=(int)pow(size,0.5);
	if(newSize*newSize!=size){
		throw exception("The number pieces is not a squere number");//cant work;
	}	
	Mat* Rows=new Mat[newSize];
	unsigned int *notused=new unsigned int[size];
	for (int i = 0; i < size; i++)
		{notused[i]=UNUSED;}//nullify array
	int finalsize=imgarr[0].cols*newSize;
	double edgeL=THOLD;
	double edgeR=THOLD;
	unsigned short counter=0;
	for(int j=0;j<((newSize-1)*newSize);j++)//for all of the rows 
	{
		pair<int,pair<int,int>> nextBest=getBestMatch(imgarr,notused,size,HOZ);
		if (nextBest.second.second==LEFT&&imgarr[nextBest.first].cols!=finalsize){
		hconcat(imgarr[nextBest.second.first],imgarr[nextBest.first],imgarr[nextBest.first]);//concat to the left side store it in the spot 
		notused[nextBest.second.first]=DURING;
		if(imgarr[nextBest.first].cols==finalsize){
			notused[nextBest.first]=ROW_DONE;
			Rows[counter]=imgarr[nextBest.first];
			counter++;
			continue;}
		}
		else if (nextBest.second.second==RIGHT&&imgarr[nextBest.first].cols!=finalsize){
				hconcat(imgarr[nextBest.first],imgarr[nextBest.second.first],imgarr[nextBest.first]);//concat to the left side store it in the spot 
				notused[nextBest.second.first]=DURING;
					if(imgarr[nextBest.first].cols==finalsize)
					{
					notused[nextBest.first]=ROW_DONE;
					Rows[counter]=imgarr[nextBest.first];
					counter++;
					continue;
					}
		}
	
	}//end for
	//***********************************************************
	finalsize=Rows[0].rows*newSize;
	free(notused);  

	notused=new unsigned int[newSize];
	counter=0;
	for (int i = 0; i < newSize; i++){notused[i]=UNUSED;}
	for(int j=0;j<(newSize-1);j++)//for all of the rows 
	{
		pair<int,pair<int,int>> nextBest=getBestMatch(Rows,notused,newSize,VERT);
		if (nextBest.second.second==UP&&Rows[nextBest.first].cols!=finalsize){
		vconcat(Rows[nextBest.second.first],Rows[nextBest.first],Rows[nextBest.first]);//concat to the left side store it in the spot 
		notused[nextBest.second.first]=DURING;
		if(Rows[nextBest.first].cols==finalsize){
			notused[nextBest.first]=ROW_DONE;
			Rows[counter]=imgarr[nextBest.first];

			return Rows[counter];
		}
		}
		else if (nextBest.second.second==DOWN&&Rows[nextBest.first].cols!=finalsize){
				vconcat(Rows[nextBest.first],Rows[nextBest.second.first],Rows[nextBest.first]);//concat to the left side store it in the spot 
				notused[nextBest.second.first]=DURING;
					if(Rows[nextBest.first].cols==finalsize)
					{
					notused[nextBest.first]=ROW_DONE;
					Rows[counter]=imgarr[nextBest.first];
					return Rows[counter];
			
					}
		}

}


	free(notused);
	return Rows[0];

}
//-----------------------------------------------------------------------------------------------
//the function checks the best match in the unsued images and return the data neccessary to concat the vectors
//this works only for horizontal edges 

pair<int,pair<int,int>> getBestMatch(Mat *images,unsigned int * notused,int size,int side)
{
if(!images||!notused||!size)//input checking 
	throw exception("param error");

	switch (side){
	case HOZ:{
		double left_right=THOLD;
		double right_left=THOLD;
		int BestMatchLR[2]={-1,-1};
		int BestMatchRL[2]={-1,-1};
		pair<int,pair<int,int>> bestMatch;//first is the index and second.first is the index and second.second is the side

	for(int i=0;i<size;i++){
		if(notused[i]==UNUSED){
		vector<Mat> edgesi=*ExtEdges(images[i]);
	
		for(int z=i+1;z<size;z++){
			if(notused[z]==UNUSED){
			vector<Mat> edgesz=*ExtEdges(images[z]);
			edgesi[LEFT];
			edgesz[RIGHT];
			double x=CompareEdges(edgesi[LEFT],edgesz[RIGHT],THOLD)[MeasureBy];
			if(x<left_right)
			{
					left_right=x;
					BestMatchLR[0]=i;
					BestMatchLR[1]=z;
			}
			double y=CompareEdges(edgesi[RIGHT],edgesz[LEFT],THOLD)[MeasureBy];
			if(y<right_left)
			{
					right_left=y;
					BestMatchRL[0]=i;
					BestMatchRL[1]=z;
				}
			}//if z unused
			}//for z...
		}
	}//if notused.
	//for i... 
					//phase  one complete updating best score
				if(left_right<right_left)//the pair 
				{bestMatch.first=BestMatchLR[0];
				bestMatch.second.first=BestMatchLR[1];
				bestMatch.second.second=LEFT;//connect the 
				}

				else 
				{
				bestMatch.first=BestMatchRL[0];
				bestMatch.second.first=BestMatchRL[1];
				bestMatch.second.second=RIGHT;//connect the sec
				
				
				}

				return bestMatch;
	
	}
	case VERT:{
		double up_down=THOLD;
		double down_up=THOLD;
		int BestMatchUD[2]={-1,-1};
		int BestMatchDU[2]={-1,-1};
		pair<int,pair<int,int>> bestMatch;//first is the index and second.first is the index and second.second is the side

	for(int i=0;i<size;i++){
		if(notused[i]==UNUSED){
		vector<Mat> edgesi=*ExtEdges(images[i]);
	
		for(int z=i+1;z<size;z++){
			if(notused[z]==UNUSED){
			vector<Mat> edgesz=*ExtEdges(images[z]);
			edgesi[UP];
			edgesz[DOWN];
			double x=CompareEdges(edgesi[UP],edgesz[DOWN],THOLD)[MeasureBy];
			if(x<up_down)
			{
					up_down=x;
					BestMatchUD[0]=i;
					BestMatchUD[1]=z;
			}
			double y=CompareEdges(edgesi[DOWN],edgesz[UP],THOLD)[MeasureBy];
			if(y<down_up)
			{
					down_up=y;
					BestMatchDU[0]=i;
					BestMatchDU[1]=z;
				}
			}//if z unused
			}//for z...
		}
	}//if notused.
	//for i... 
					//phase  one complete updating best score
				if(up_down<down_up)//the pair 
				{bestMatch.first=BestMatchUD[0];
				bestMatch.second.first=BestMatchUD[1];
				bestMatch.second.second=UP;//The side of the connection from the bestMatch.first point of view 
				}

				else 
				{
				bestMatch.first=BestMatchDU[0];
				bestMatch.second.first=BestMatchDU[1];
				bestMatch.second.second=DOWN;//connect the sec
				
				
				}

				return bestMatch;
	
	}
	default:{
				 throw exception("Invalid side value");
				 
				 }
	}
	}


//---------------------------------------------------------------------------------
//returns the matching images by indices and directions 

vector<string> *FindMatchingEdges(Mat A[9]){

	vector<string>* Matches=new vector<string>();
	for(int i=0;i<9;i++){
		vector<Mat> edgesi=*ExtEdges(A[i]);
		int BestMatchLR[2]={-1,-1};
		int BestMatchRL[2]={-1,-1};
		int BestMatchUD[2]={-1,-1};
		int BestMatchDU[2]={-1,-1};
		int left_rightD=THOLD;
		int down_upD=THOLD;
		int up_downD=THOLD;
		int right_leftD=THOLD;

		double left_right=THOLDAVG;
		double down_up=THOLDAVG;
		double up_down=THOLDAVG;
		double right_left=THOLDAVG;
		for(int z=i+1;z<9;z++){
			vector<Mat> edgesz=*ExtEdges(A[z]);

			cout<<"the Sides being tested are "<<i<<","<<z<<endl;
			cout<<"left"<<"right"<<endl;
			if(CompareEdges(edgesi[LEFT],edgesz[RIGHT],THOLD)[AVG]<left_right)
			{

				if(left_rightD>CompareEdges(edgesi[LEFT],edgesz[RIGHT],THOLD)[MeasureBy])
				{
					left_rightD=CompareEdges(edgesi[LEFT],edgesz[RIGHT],THOLD)[MeasureBy];
					left_right=CompareEdges(edgesi[LEFT],edgesz[RIGHT],THOLD)[AVG];
					BestMatchLR[0]=i;
					BestMatchLR[1]=z;
				}
			}
			cout<<"up"<<"down"<<endl;
			if(CompareEdges(edgesi[UP],edgesz[DOWN],THOLD)[AVG]<up_down)
			{
				if(up_downD>CompareEdges(edgesi[UP],edgesz[DOWN],THOLD)[MeasureBy]){
					left_rightD=CompareEdges(edgesi[UP],edgesz[DOWN],THOLD)[MeasureBy];
					up_down=CompareEdges(edgesi[UP],edgesz[DOWN],THOLD)[AVG];
					BestMatchUD[0]=i;
					BestMatchUD[1]=z;}
			}
			cout<<"right"<<"left"<<endl;
			if(CompareEdges(edgesi[RIGHT],edgesz[LEFT],THOLD)[AVG]<right_left)
			{

				if(right_leftD>CompareEdges(edgesi[RIGHT],edgesz[LEFT],THOLD)[MeasureBy]){
					right_leftD=CompareEdges(edgesi[RIGHT],edgesz[LEFT],THOLD)[MeasureBy];
					right_left=CompareEdges(edgesi[RIGHT],edgesz[LEFT],THOLD)[AVG];
					BestMatchRL[0]=i;
					BestMatchRL[1]=z;}
			}
			cout<<"down"<<"up"<<endl;
			if(CompareEdges(edgesi[DOWN],edgesz[UP],THOLD)[AVG]<down_up)
			{
				if(down_upD>CompareEdges(edgesi[DOWN],edgesz[UP],THOLD)[MeasureBy]){
					down_upD=CompareEdges(edgesi[DOWN],edgesz[UP],THOLD)[MeasureBy];
					down_up=CompareEdges(edgesi[DOWN],edgesz[UP],THOLD)[AVG];
					BestMatchDU[0]=i;
					BestMatchDU[1]=z;}
			}
			if(z==8)
			{
				if(down_up<THOLDAVG&&BestMatchDU[0]>=0&&BestMatchDU[1]>=0){
					//if(BestMatchDU[0]==BestMatchUD[0]||BestMatchDU[1]==BestMatchUD[1]
					Matches->push_back(to_string(BestMatchDU[0])+string(",")+to_string(BestMatchDU[1])+string(",")+string("down_up"));
				}
				if(up_down<THOLDAVG&&BestMatchUD[0]>=0&&BestMatchUD[1]>=0)
					Matches->push_back(to_string(BestMatchUD[0])+string(",")+to_string(BestMatchUD[1])+string(",")+string("up_down"));

				if(left_right<THOLDAVG&&BestMatchLR[0]>=0&&BestMatchLR[1]>=0)
					Matches->push_back(to_string(BestMatchLR[0])+string(",")+to_string(BestMatchLR[1])+string(",")+string("left_right"));

				if(right_left<THOLDAVG&&BestMatchRL[0]>=0&&BestMatchRL[1]>=0)
					Matches->push_back(to_string(BestMatchRL[0])+string(",")+to_string(BestMatchRL[1])+string(",")+string("right_left"));

			}
		}


	}
	return Matches;
}
//------------------------------------------------------------------------
//return images from a directory
Mat* GetImages(string path,int size){
	vector<string> FileNames=GetFileNames(path);
	if(FileNames.size()!=size)
		throw exception("The size parameter doesn't match\n");
Mat* Images=new Mat[size];
for (int i = 0; i < FileNames.size(); i++)
{
	Images[i]=imread(path+FileNames.at(i));
}
return Images;

}


//retrive a list of files in the directory 
vector<string>& GetFileNames( string path)
{
vector<string> *Files=new vector<string>();
DIR *dir;
struct dirent *ent;
cout<<path.data()<<endl;
if ((dir = opendir (path.data())) != NULL) {
  while ((ent = readdir (dir)) != NULL) {
	  if(!strcmp(ent->d_name+strlen(ent->d_name)-4,".jpg")||!strcmp(ent->d_name+strlen(ent->d_name)-4,".JPG")){
		  Files->push_back(ent->d_name);
	  }
  }
  closedir (dir);
} else {
  /* could not open directory */
  perror ("Error reading Directory");
	return *Files; //will be empty on error
}

return *Files;

}

//----------------------------------------------------------------------------------------
//returns an array of images of size size^0.5 
//output :NULL on error

Mat* CreatePuzzle(Mat img,int size){
	if(size==0){
	return NULL;
	}
	int peiceSize=(int)pow(size,0.5);
	if(peiceSize*peiceSize!=size)//the input number have to be a squere number
		return NULL;
	
	int newHeight=640-640%peiceSize;
	int newWidth=480-480%peiceSize;
	resize(img,img,Size(newHeight,newWidth));
	Mat* imgArr=new Mat[size]();
	
	short counter=0;
	for (int i = 0; i < peiceSize; i++)
	{
		for (int j = 0; j < peiceSize; j++){
			imgArr[counter++]=img(Rect(i*img.rows/peiceSize,j*img.cols/peiceSize,img.rows/peiceSize,img.cols/peiceSize));
		}
	}
	return imgArr;
}

//-------------------------------------------------------------------------

//output: An array of historgram 
 int * calcHisto(Mat img){
	 int *a=new int[256];

	 for (int k = 0; k < 256; k++)
	 {a[k]=0;
		 for(int j=0;j<img.cols;j++)
		 {
			 for(int t=0;t<img.rows;t++)
			 {
				 int x=(int)img.at<uchar>(t,j);
				 if(x==k)
				 {
					 a[k]+=1;
				 }
			 }
		}
	 }
	 
	 return a;

}