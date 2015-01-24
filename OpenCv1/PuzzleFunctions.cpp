#include "PuzzleFunctions.h"
//--------------------------------------------------
const int EDGE_SIZE=1;
const int THOLDIFF=0 ;
const int MeasureBy=DIFF;
const int Method=L1;
//----------------------------------------------------------
//Compares two images, used to compare two edges although works for matrices as well
//th is a treshold for the comparison operation if the difference is greater then the th then the vectors are not equal
//returns an array indicating the difference between the two edges by avg value of each vector
//and by the number of differences higher then a scalar x

double* CompareEdges(Mat Ed1,Mat Ed2,const short method){

	if(Ed1.cols!=Ed2.cols&&Ed1.rows!=Ed2.rows){
		throw  exception("The edges size is different");
	}
	
	double avgEd1=0,avgEd2=0;//avg value
	double deved1=0,deved2=0;
	int len=Ed1.cols*Ed1.rows;
	double cov=0;
	int difference=0;
	double *res=new double[8];
	for (int i = 0; i < 8; i++) res[i]=0;//init 

	
	switch (MeasureBy){
	case HIST :{
		int result=0;
			int *H1 =calcHisto(Ed1);
			int *H2=calcHisto(Ed2);
			  for (int i = 0; i < 256; i++){
				  if(method==L1){
				int tempRes=H1[i]-H2[i];
				result+=abs(tempRes);}
				  else if(method==L2){
				int tempRes=(int)pow(H1[i]-H2[i],2);
				result+=tempRes;
				  }

			  }
			  free(H1);
			  free(H2);
			  if(method==L1)
			  res[HIST]=result;
			  else
			  res[HIST]=sqrt(result);
			  return res;
	}
	case EQL:{
				if(method==L1){
			   
			   Mat A=abs(Ed1-Ed2);
				double equality=sum(A)[0];
				res[EQL]=equality;
			   return res;}
			   
			 	if(method==L2){
				double temp=0;
			   int len=Ed1.cols*Ed1.rows;
			   for (int i = 0; i < Ed1.cols; i++)
			   {
				   for (int j = 0; j < Ed1.rows; j++)
				 temp+=pow(int(Ed1.at<uchar>(j,i))-int (Ed2.at<uchar>(j,i)),2);
				
			   }

			   res[EQL]=sqrt(temp);
			   return res;
				}
	}

	case GRAD:{
			   pair<Mat,Mat> Gradients1=*CalcSobelDerrivetives(Ed1);
			   pair<Mat,Mat> Gradients2=*CalcSobelDerrivetives(Ed2);
			   Mat A=abs(Gradients1.first-Gradients2.first);//gradients different by x
			   Mat B=abs(Gradients1.second-Gradients2.second);//gradients different by y
			   double equality=0;
			   if(Ed1.rows>Ed1.cols)
				equality=(double)sum(B)[0];//difference in gradients
			   else
			   equality=sum(A)[0];//difference in gradients
			   res[GRAD]=equality;
			   return res;
			   }
	case DIFF: case COV :case DEV: case AVG :case DIFFSUM:{
				double Diffsum=0;
				avgEd1=sum(Ed1)[0]/len;
				avgEd2=sum(Ed2)[0]/len;
				double x=0;
				double y=0,z=0;
	for (int i = 0; i < Ed1.cols; i++)
	{
		for (int j = 0; j < Ed1.rows; j++)
		{
			deved1+=abs(avgEd1-(double)Ed1.at<uchar>(j,i));
			deved2+=abs(avgEd2-(double)Ed1.at<uchar>(j,i));
			cov+=double(abs(avgEd2-Ed2.at<uchar>(j,i))*abs(avgEd1-Ed1.at<uchar>(j,i)))/len;
			x=abs((int)Ed1.at<uchar>(j,i)-(int)Ed2.at<uchar>(j,i));
			if(Ed1.rows>Ed1.cols){
				if(j!=Ed1.rows-1)
					y=(double)abs(Ed1.at<uchar>(j,i)-Ed2.at<uchar>(j+1,i));
				if (j>=1)
					z=(double)abs(Ed1.at<uchar>(j,i)-Ed2.at<uchar>(j-1,i));
			}
			else{
			if(i!=Ed1.cols-1)
					y=(double)abs(Ed1.at<uchar>(j,i)-Ed2.at<uchar>(j,i+1));
				if (i>=1)
					z=(double)abs(Ed1.at<uchar>(j,i)-Ed2.at<uchar>(j,i-1));
			
			}
			Diffsum+=x+y+z;
			if(x>=THOLDIFF)
				difference++;
			if(y>=THOLDIFF)
				difference++;
			if(z>=THOLDIFF)
				difference++;
			
	}

	}	
		res[DEV]=abs(deved1-deved2);
		res[AVG]=abs(avgEd1-avgEd2);
		res[DIFF]=difference;
		res[DIFFSUM]=Diffsum;
		res[COV]=cov;
		return res;
			  
			  }
			  
	default :{
		return res;}
	
	}

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
	int newSize=(int)sqrt(size);

	if(newSize*newSize!=size){
		throw exception("The number pieces is not a squere number");//cant work;
	}	

	unsigned int *notused=new unsigned int[size];
	for (int i = 0; i < size; i++){notused[i]=UNUSED;}//nullify array
	int finalsize=imgarr[0].cols*newSize;
	double edgeL=THOLD;
	double edgeR=THOLD;
	
	unsigned short *rows=new unsigned short[newSize];
	for(int j=0;j<newSize;j++)//for all of the rows 
	{
		int x=-1; 
				pair<int,pair<int,int>> currentBest=getBestMatch(imgarr,notused,size,HOZ);
				x=currentBest.first;//setting the current index as x
				
				/*if(currentBest.second.second==RIGHT&&notused[currentBest.second.first]==UNUSED)
					hconcat(imgarr[currentBest.second.first],imgarr[x],imgarr[x]);
				else if(currentBest.second.second==LEFT&&notused[currentBest.second.first]==UNUSED)
					hconcat(imgarr[x],imgarr[currentBest.second.first],imgarr[x]);*/
				notused[x]=DURING;//Image being worked on right now
		
		int counter=0;
	while(imgarr[x].cols!=finalsize&&counter!=1000){
		counter++;//safety feature
	vector<Mat> edgesa=*ExtEdges(imgarr[x]);
	edgeR=THOLD;
	edgeL=THOLD;
	short bestMatchLR=-1;
	short bestMatchRL=-1;
	for(int i=0;i<size;i++){//looking for the image 
		vector<Mat> edgesb=*ExtEdges(imgarr[i]);//looking for a match
		if(notused[i]==UNUSED&&notused[x]==DURING){
			double y=CompareEdges(edgesa[LEFT],edgesb[RIGHT],Method)[MeasureBy];
			if(y<edgeL)//checking match on the left side
			{
				edgeL=y;
				bestMatchLR=i;
			}
			 y=CompareEdges(edgesa[RIGHT],edgesb[LEFT],Method)[MeasureBy];
			 if(y<edgeR)//checking match on the right side
			 {
					edgeR=y;//set edgeR to the current best
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
				free(notused);
				notused=new unsigned int[newSize];

	for (int i = 0; i < newSize; i++)notused[i]=UNUSED;//initialize array 
	unsigned int counter=0;
	while(imgarr[rows[0]].rows!=finalRows&&counter!=100){
		double bestU=THOLD;
		double bestD=THOLD;
		int idxD=-1,idxU=-1;
		counter++;//safety
	for (int i =1; i < newSize; i++)
	{//for each row 
		if(notused[i]==UNUSED){
		vector<Mat> edgeA=*ExtEdges(imgarr[rows[0]]);//always take the first image and build the full image up on it 
		vector<Mat> edgeB=*ExtEdges(imgarr[rows[i]]);//extract the i row in the 

		double x=CompareEdges(edgeA[UP],edgeB[DOWN],Method)[MeasureBy];
		if(bestU>x)
		{
			bestU=x;
			idxU=i;
		}
		x=CompareEdges(edgeA[DOWN],edgeB[UP],Method)[MeasureBy];
		if(bestD>x){
			bestD=x;
			idxD=i;
		}
			}
	}
	
	if(bestD>bestU&&idxU!=-1){
				vconcat(imgarr[rows[idxU]],imgarr[rows[0]],imgarr[rows[0]]);
				notused[idxU]=DONE;
		/*	cout<<"**********"<<bestD<<","<<bestU<<"**********"<<endl;
			cout<<"******U****"<<idxD<<","<<idxU<<"**********"<<endl;*/
			}
	else if(idxD!=-1){
			vconcat(imgarr[rows[0]],imgarr[rows[idxD]],imgarr[rows[0]]);
				notused[idxD]=DONE;
			//cout<<"**********"<<bestD<<","<<bestU<<"**********"<<endl;
			//cout<<"*****D*****"<<idxD<<","<<idxU<<"**********"<<endl;
			}

	
	}
	int idx=rows[0];
	free(rows);
	free(notused);
	return imgarr[idx];
}

//------------------------------------------------------------------------------------------------

Mat SolvePuzzle(Mat* imgarr,int size){//throws exception
	int newSize=(int)sqrt(size);
	if(newSize*newSize!=size){
		throw exception("The number pieces is not a squere number");//cant work;
	}	
	Mat* Rows=new Mat[newSize];
	unsigned int *notused=new unsigned int[size];//This array is used to indicate which image was already used
	for (int i = 0; i < size; i++)
		{notused[i]=UNUSED;}//nullify array

	int finalsize=imgarr[0].cols*newSize;
	double edgeL=THOLD;
	double edgeR=THOLD;
	unsigned short counter=0;
	for(int j=0;j<((newSize-1)*newSize);j++)//for all of the rows 
	{
		pair<int,pair<int,int>> nextBest=getBestMatch(imgarr,notused,size,HOZ);
		if (nextBest.second.second==LEFT&&imgarr[nextBest.second.first].cols<finalsize){
			hconcat(imgarr[nextBest.first],imgarr[nextBest.second.first],imgarr[nextBest.second.first]);//concat to the left side store it
			notused[nextBest.first]=DURING;
			if(imgarr[nextBest.second.first].cols==finalsize){
			notused[nextBest.second.first]=ROW_DONE;
			Rows[counter]=imgarr[nextBest.second.first];
			counter++;}
		}
		else if (nextBest.second.second==RIGHT&&imgarr[nextBest.first].cols<finalsize){
		
				hconcat(imgarr[nextBest.first],imgarr[nextBest.second.first],imgarr[nextBest.first]);//concat to the left side store it  
				notused[nextBest.second.first]=DURING;
					if(imgarr[nextBest.first].cols==finalsize)
					{
						notused[nextBest.first]=ROW_DONE;
						Rows[counter]=imgarr[nextBest.first];
						counter++;
					}
		}
	
	}//end for
	//***********************************************************
	//pair<Mat*,int> ROWS=SlicesCalc(imgarr,size);
	if(counter!=newSize){
		throw exception("Cant solve\n");
			finalsize=Rows[0].rows*newSize;
	free(notused); 
}
	notused=new unsigned int[newSize];
	counter=0;
	for (int i = 0; i < newSize; i++){notused[i]=UNUSED;}//init
	
	for(int j=0;j<(newSize-1);j++)//for all of the rows 
	{
		pair<int,pair<int,int>> nextBest=getBestMatch(Rows,notused,newSize,VERT);
		if (nextBest.second.second==UP&&Rows[nextBest.first].rows!=finalsize){
		vconcat(Rows[nextBest.second.first],Rows[nextBest.first],Rows[nextBest.first]);//concat to the left side store it in the spot 
		notused[nextBest.second.first]=DURING;
		}
		else if (nextBest.second.second==DOWN&&Rows[nextBest.second.first].rows!=finalsize){
				vconcat(Rows[nextBest.first],Rows[nextBest.second.first],Rows[nextBest.first]);//concat to the left side store it in the spot 
				notused[nextBest.second.first]=DURING;
				//cout<<"down"<<endl;
					
		}

}

	free(notused);
	for (int i = 0; i < newSize; i++)
	{if(Rows[i].rows==finalsize)
	return Rows[i];

	}
	return Rows[0];

}
//-----------------------------------------------------------------------------------------------
//the function checks the best match in the unsued images and return the data neccessary to concat the vectors

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
			double x=CompareEdges(edgesi[LEFT],edgesz[RIGHT],Method)[MeasureBy];
			if(x<left_right)
			{
					left_right=x;
					BestMatchLR[0]=i;
					BestMatchLR[1]=z;
			}
			double y=CompareEdges(edgesi[RIGHT],edgesz[LEFT],Method)[MeasureBy];
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
					//phase one complete updating best score
				if(left_right<right_left)//the pair 
				{
				bestMatch.first=BestMatchLR[0];
				bestMatch.second.first=BestMatchLR[1];
				bestMatch.second.second=RIGHT;//connect the 
				}

				else if(left_right!=right_left)
				{
				bestMatch.first=BestMatchRL[0];
				bestMatch.second.first=BestMatchRL[1];
				bestMatch.second.second=LEFT;//connect the sec
				
				
				}
				else{//if the values are equal then compare by avg value
					//cout<<"equal testing for the smallest avg difference value"<<endl;
					vector<Mat> edgesA=*ExtEdges(images[BestMatchRL[0]]);
					vector<Mat> edgesB=*ExtEdges(images[BestMatchRL[1]]);
					double x=CompareEdges(edgesA[RIGHT],edgesB[LEFT],Method)[DIFF];
					double y=CompareEdges(edgesA[LEFT],edgesB[RIGHT],Method)[DIFF];
					if(x>y){
					bestMatch.first=BestMatchRL[0];
					bestMatch.second.first=BestMatchRL[1];
					bestMatch.second.second=LEFT;//connect the sec
							}
					else{
					bestMatch.first=BestMatchLR[0];
					bestMatch.second.first=BestMatchLR[1];
					bestMatch.second.second=RIGHT;
					
					}
				
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
				//if(z==2)cout<<images[z];
			vector<Mat> edgesz=*ExtEdges(images[z]);
			double x=CompareEdges(edgesi[UP],edgesz[DOWN],Method)[MeasureBy];
			if(x<up_down)
			{
					up_down=x;
					BestMatchUD[0]=i;
					BestMatchUD[1]=z;
			}
			double y=CompareEdges(edgesi[DOWN],edgesz[UP],Method)[MeasureBy];
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

vector<unsigned int> FindMatchingEdges(Mat *A,int size){

	vector<unsigned int > Match;
		double left_rightD=THOLD;
		double down_upD=THOLD;
		double up_downD=THOLD;
		double right_leftD=THOLD;
		int BestMatchLR[2]={-1,-1};
		int BestMatchRL[2]={-1,-1};
		int BestMatchUD[2]={-1,-1};
		int BestMatchDU[2]={-1,-1};

	for(int i=0;i<size;i++){
		vector<Mat> edgesi=*ExtEdges(A[i]);
		
		for(int z=i+1;z<size;z++){
			vector<Mat> edgesz=*ExtEdges(A[z]);

			//cout<<"the Sides being tested are "<<i<<","<<z<<endl;
			//cout<<"left"<<"right"<<endl;
			if(CompareEdges(edgesi[LEFT],edgesz[RIGHT],Method)[MeasureBy]<left_rightD)
			{
					left_rightD=CompareEdges(edgesi[LEFT],edgesz[RIGHT],Method)[MeasureBy];
					BestMatchLR[0]=i;
					BestMatchLR[1]=z;
			}
			//cout<<"up"<<"down"<<endl;
			if(CompareEdges(edgesi[UP],edgesz[DOWN],Method)[MeasureBy]<up_downD)
			{
					up_downD=CompareEdges(edgesi[UP],edgesz[DOWN],Method)[MeasureBy];
					BestMatchUD[0]=i;
					BestMatchUD[1]=z;
			}
			//cout<<"right"<<"left"<<endl;
			if(CompareEdges(edgesi[RIGHT],edgesz[LEFT],Method)[MeasureBy]<right_leftD)
			{

					right_leftD=CompareEdges(edgesi[RIGHT],edgesz[LEFT],Method)[MeasureBy];
					BestMatchRL[0]=i;
					BestMatchRL[1]=z;

			}
			//cout<<"down"<<"up"<<endl;
			if(CompareEdges(edgesi[DOWN],edgesz[UP],Method)[MeasureBy]<down_upD)
			{
					down_upD=CompareEdges(edgesi[DOWN],edgesz[UP],Method)[MeasureBy];
					BestMatchDU[0]=i;
					BestMatchDU[1]=z;
			}}
			
	}
				if(down_upD>up_downD)
					if(up_downD<left_rightD&&up_downD<right_leftD)
					{
					Match.push_back(BestMatchUD[0]);
					Match.push_back(BestMatchUD[1]);
					Match.push_back(UP);
					
					}
					else if(right_leftD<left_rightD)
					{
					Match.push_back(BestMatchRL[0]);
					Match.push_back(BestMatchRL[1]);
					Match.push_back(RIGHT);
					}
					else
					{
					Match.push_back(BestMatchLR[0]);
					Match.push_back(BestMatchLR[1]);
					Match.push_back(LEFT);
					}
				else
				{if(down_upD<left_rightD&&down_upD<right_leftD)
					{
					Match.push_back(BestMatchDU[0]);
					Match.push_back(BestMatchDU[1]);
					Match.push_back(DOWN);
					
					}
					else if(right_leftD<left_rightD)
					{
					Match.push_back(BestMatchRL[0]);
					Match.push_back(BestMatchRL[1]);
					Match.push_back(RIGHT);
					}
					else
					{
					Match.push_back(BestMatchLR[0]);
					Match.push_back(BestMatchLR[1]);
					Match.push_back(LEFT);
					}
				
				
				
				
				}

	return Match;
}
//------------------------------------------------------------------------
//return images from a directory
Mat* GetImages(string path,unsigned int size){
	vector<string> FileNames=GetFileNames(path);
	if(FileNames.size()!=size)
		throw exception("GetImages::The size parameter doesn't match\n");

Mat* Images=new Mat[size];
int s=FileNames.size();
for (int i = 0; i < s; i++)
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
if ((dir = opendir (path.data())) != NULL) {
  while ((ent = readdir (dir)) != NULL) {
	  if(!strcmp(ent->d_name+strlen(ent->d_name)-4,".jpg")||!strcmp(ent->d_name+strlen(ent->d_name)-4,".JPG")){
		  Files->push_back(ent->d_name);
	  }
  }
  closedir (dir);
} else {
  /* could not open directory */
  perror ("GetFileNames::Error reading Directory");
	return *Files; //will be empty on error
}
return *Files;

}

//----------------------------------------------------------------------------------------
//returns an array of images of size size^0.5 
//size is the number of peices for the puzzle

Mat* CreatePuzzle(Mat img,int size){
	if(size<=0){
	throw exception("CreatePuzzle::the number of peices has to be a positive integer");
	}
	int peiceSize=(int)pow(size,0.5);
	if(peiceSize*peiceSize!=size)//the input number have to be a squere number
		throw exception("CreatePuzzle::The number of peices has to be a squere number");
	if(!img.cols/peiceSize||!img.rows/peiceSize)
		throw exception("CreatePuzzle::The image cannot be devided into that many squeres");
	FixImgSize(img,peiceSize);
	Mat* imgArr=new Mat[size]();
	short counter=0;
	for (int i = 0; i < peiceSize; i++)
	{
		for (int j = 0; j < peiceSize; j++){
		
			imgArr[counter++]=img(Rect(i*img.cols/peiceSize,j*img.rows/peiceSize,img.cols/peiceSize,img.rows/peiceSize));
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
 //-----------------------------------------------------------------------------------------------
 pair<Mat,Mat> * CalcSobelDerrivetives(Mat src){
	Mat temp(src);
	Mat src_gray;
  Mat grad;
  int scale = 1;
  int delta = 0;
  int ddepth = CV_16S;
  pair<Mat,Mat> *Gradients=new pair<Mat,Mat>;
  if( !src.data )
  { return  Gradients; }

  //GaussianBlur(temp, temp, Size(3,3), 0, 0, BORDER_DEFAULT );
  /// Convert it to gray
  if(temp.channels()>1)
  cvtColor( temp, src_gray, CV_RGB2GRAY );
  else if(temp.channels()==1)
	  src_gray=temp;
  
  /// Generate grad_x and grad_y
  Mat grad_x, grad_y;
  Mat abs_grad_x, abs_grad_y;

  /// Gradient X
  //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
  Sobel( src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
  convertScaleAbs( grad_x, abs_grad_x );

  /// Gradient Y
  //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
  Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
  convertScaleAbs( grad_y, abs_grad_y );
  
  (*Gradients).first=abs_grad_x;
  (*Gradients).second=abs_grad_y;
  return Gradients;
  }
 //-------------------------------------------------------------------------------------------
 void SharpenImages(Mat* imarr,const double alpha,int size){

 for (int i = 0; i < size; i++)
		{

		Mat temp;
		if(!imarr[i].data)
			throw exception("SharpenImages::Bad Image Data");
		SharpenImage(imarr[i],alpha);
		}	
 
 }
//--------------------------------------------------------------------------------------------
  void SharpenImage(Mat &img,double alpha){

		Mat temp;
		if(!img.data)
			throw exception("ShrapenImage::Bad Image Data");
		GaussianBlur(img,temp,Size(5,5),1,0);
		temp=img-temp;
		img=img+alpha*temp;	
 }

//--------------------------------------------------------------------------------------------
  Mat FindMatchingCols(Mat* imgarr,int size){//throws exception on error
	int newSize=(int)sqrt(size);

	if(newSize*newSize!=size){
		throw exception("FindMatchingCols::The number pieces is not a squere number");//cant work;
	}	

	unsigned int *notused=new unsigned int[size];
	for (int i = 0; i < size; i++){notused[i]=UNUSED;}//nullify array
	int finalsize=imgarr[0].rows*newSize;
	double edgeU=THOLD;
	double edgeD=THOLD;
	
	unsigned short *cols=new unsigned short[newSize];
	for(int j=0;j<newSize;j++)//for all of the rows 
	{
		int x=-1; 
		pair<int,pair<int,int>> currentBest=getBestMatch(imgarr,notused,size,VERT);
		x=currentBest.first;//setting the current index as x
		notused[x]=DURING;//Image being worked on right now
	int counter=0;
	while(imgarr[x].rows!=finalsize&&counter!=1000){
		counter++;//safety feature
	vector<Mat> edgesa=*ExtEdges(imgarr[x]);
	edgeU=THOLD;
	edgeD=THOLD;
	short bestMatchUD=-1;
	short bestMatchDU=-1;
	for(int i=0;i<size;i++){//looking for the image 
		vector<Mat> edgesb=*ExtEdges(imgarr[i]);//looking for a match
		if(notused[i]==UNUSED&&notused[x]==DURING){
			double y=CompareEdges(edgesa[UP],edgesb[DOWN],Method)[MeasureBy];
			if(y<edgeU)//checking match on the left side
			{
				edgeU=y;
				bestMatchUD=i;
			}
			y=CompareEdges(edgesa[DOWN],edgesb[UP],Method)[MeasureBy];
			 if(y<edgeD)//checking match on the right side
			 {
					edgeD=y;//set edgeD to the current best
					bestMatchDU=i;//keep the index of the best match on the Down side
				}

	}//end if(notused[i]==0&&notused[x]==2)
	}//end for(int i=0;i<9;i++)
	//connect the images
	//ShowImg(imgarr[x]);
		if(edgeD<=edgeU){//down is better then up for x 
			if(bestMatchDU==bestMatchUD&&bestMatchDU!=-1){//if the peices are the same
					vconcat(imgarr[x],imgarr[bestMatchDU],imgarr[x]);//connect the x right side to the matches left side
					notused[bestMatchDU]=DONE;
				}
			else if(bestMatchDU>=0)
		 {//found a match on the right side
			vconcat(imgarr[x],imgarr[bestMatchDU],imgarr[x]);
			notused[bestMatchDU]=DONE;//setting the used images as -1 in the notused array
		}
			}//if
		else{
			if(bestMatchUD==bestMatchDU&&bestMatchUD!=-1){
					vconcat(imgarr[bestMatchUD],imgarr[x],imgarr[x]);//concat to the left side store it in the spot 
					notused[bestMatchUD]=DONE;//setting the used images as -1 in the notused array
					
			}
			else if(bestMatchUD>=0){//found a match on the left side
			vconcat(imgarr[bestMatchUD],imgarr[x],imgarr[x]);//concat to the left side store it in the spot 
			notused[bestMatchUD]=DONE;//setting the used images as -1 in the notused array
		}
		
		}
		//ShowImg(imgarr[x]);

	}//end while(imgarr[x].cols!=finalsize)
	notused[x]=ROW_DONE;
	cols[j]=x;
	
	}//end for(int j=0;j<3;j++)
	//***********************************************************
	//phase one complete //rows are constructed now and all ready for phase two
	//connect the rows together
	
	int finalCols=imgarr[cols[0]].cols*newSize;
				free(notused);
				notused=new unsigned int[newSize];

	for (int i = 0; i < newSize; i++)notused[i]=UNUSED;//initialize array 
	unsigned int counter=0;
	while(imgarr[cols[0]].cols!=finalCols&&counter!=100){
		double bestL=THOLD;
		double bestR=THOLD;
		int idxR=-1,idxL=-1;
		counter++;//safety
	for (int i =1; i < newSize; i++)
	{//for each row 
		if(notused[i]==UNUSED){
		vector<Mat> edgeA=*ExtEdges(imgarr[cols[0]]);//always take the first image and build the full image up on it 
		vector<Mat> edgeB=*ExtEdges(imgarr[cols[i]]);//extract the i row in the 

		double x=CompareEdges(edgeA[LEFT],edgeB[RIGHT],Method)[MeasureBy];
		if(bestL>x)
		{
			bestL=x;
			idxL=i;
		}
		x=CompareEdges(edgeA[RIGHT],edgeB[LEFT],Method)[MeasureBy];
		if(bestR>x){
			bestR=x;
			idxR=i;
		}
			}
	}//end for 
	
	if(bestR>bestL&&idxL!=-1){
				hconcat(imgarr[cols[idxL]],imgarr[cols[0]],imgarr[cols[0]]);
				notused[idxL]=DONE;
			}
	else if(idxR!=-1){
			hconcat(imgarr[cols[0]],imgarr[cols[idxR]],imgarr[cols[0]]);
				notused[idxR]=DONE;
			}

	
	}
	int idx=cols[0];
	free(cols);
	free(notused);
	return imgarr[idx];
}

//----------------------------------------------------------------------------------


  void FixImgSize(Mat &img,short Num){

	int newWidth=img.cols-img.cols%Num;
	int newHeight=img.rows-img.rows%Num;
	resize(img,img,Size(newWidth,newHeight));

  }
  //-----------------------------------------------------------------------------------------
  bool compareImgaes(const Mat &im1  ,const Mat &im2){
	  	if(im1.size()!=im2.size())
				throw exception("Compare_images: Size doesn't agree");
		Mat diff = im1!=im2;
		 ///Equal if no elements disagree
		bool eq= cv::countNonZero(diff) == 0;
		return eq;
  }

  //-----------------------------------------------------------------------------------------
  void ShowImg(const Mat &Img){
		imshow("Display",Img);
		waitKey(0);
  }

  //-----------------------------------------------------------------------------------------
  bool checkSquereNum(int Num){
  
  int newSize=(int)sqrt(Num);

	if(newSize*newSize!=Num){
		return false;
	}	
	return true;
  }

  //-------------------------------------------------------------------------------------------

  Mat ReadAndSolve(string path,Mat (*Solver)(Mat *,int)){
	  Mat * imgarr;
	  Mat sol;
	  int x=GetFileNames(path).size();//number of images 
	  if(checkSquereNum(x)){
			imgarr=GetImages(path,x);
			sol=Solver(imgarr,x);
	  }
	  return sol;
  }