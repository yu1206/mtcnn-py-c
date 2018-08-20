#include "stdio.h"
#include "string"
#include <sstream>
#include <direct.h>
#include "vector"
#include <iostream>
#include <fstream>
#include "opencv\cv.h"
#include "opencv\highgui.h"
using namespace std;
using namespace cv;
extern float IoU(Rect box, Rect boxes);
void readTxt(const char* anno_file, vector<string>& v_img_)
{
	ifstream ReadFile;
	ReadFile.open(anno_file,ios::in);
	if(ReadFile.fail())//文件打开失败:返回0  
	{  
		printf("文件打开失败:\n");
		return ;  
	}  
	else//文件存在  
	{  
		string tmp; 
		while(getline(ReadFile,tmp,'\n'))  
		{  
			 v_img_.push_back(tmp);
		}  
		ReadFile.close();  
		
	}  
}
int test1(int argc, char *argv[])
{
	/*int stdsizeW=14;
	int stdsizeH=4;
	string anno_file = "car1_14-15-train_cla-all.txt";*/
	int stdsizeW=atoi(argv[1]);
	int stdsizeH=atoi(argv[2]);
	string anno_file = argv[3];
	string sampleClass=argv[4];
	int IsampleClass=atoi(argv[4]);
	stringstream ssW;
	ssW<<stdsizeW;
	stringstream ssH;
	ssH<<stdsizeH;
	string path=ssW.str()+"-"+ssH.str()+"-"+sampleClass;
	string pos_save_dir = path + "/positive";
	string part_save_dir = path + "/part" ;
	string neg_save_dir = path+ "/negative" ;
	string save_dir = "./" + path ;

	mkdir(save_dir.c_str());
	mkdir(pos_save_dir.c_str());
	mkdir(part_save_dir.c_str());
	mkdir(neg_save_dir.c_str());

	string posPath=	save_dir+"/"+ "pos_"+path+".txt";
	string negPath=	save_dir+"/"+ "neg_"+path+".txt";
	string partPath=	save_dir+"/"+ "part_"+path+".txt";
	FILE* f1 = fopen(posPath.c_str(), "w");
	FILE* f2 = fopen(negPath.c_str(), "w");
	FILE* f3 = fopen(partPath.c_str(), "w");
	vector<string> v_img_;
	readTxt(anno_file.c_str(), v_img_);
	int num=v_img_.size();
	cout<<num<<",pics in total"<<endl;
	int p_idx = 0;
	int n_idx = 0 ;
	int d_idx = 0 ;
	int idx = 0;
	int box_idx = 0;

	for(int i=0;i<num;i++)
	{
		//printf("个数=%d\n",i);
		string 	annotation=	v_img_[i];
		string im_path,x1str,y1str,x2str,y2str;
		int x1,x2,y1,y2;
		istringstream is(annotation);
		is>>im_path>>x1str>>y1str>>x2str>>y2str;
		x1=atoi(x1str.c_str());
		y1=atoi(y1str.c_str());
		x2=atoi(x2str.c_str());
		y2=atoi(y2str.c_str());
		Rect bbox;bbox.x=x1;bbox.y=y1;bbox.width =x2-x1+1;bbox.height =y2-y1+1;
		Mat img = imread(im_path.c_str());
		if (img.data == NULL)
		{
			printf("读入图像失败");
			system("pause");
			continue;
		}
		idx=idx+1;
		if(idx%100==0)
			cout<<idx<<",images done"<<endl;
		int height=img.rows;
		int width=img.cols;
		int channel=img.channels();
		int neg_num = 0;
		srand((unsigned)time(NULL)); 
		while(neg_num<100)
		{
			//printf("负样本=%d\n",neg_num);

			int divisorW = width / 2 - stdsizeW + 1;
			int divisorH = height / 2 - stdsizeH + 1;
			if(divisorW==0)
			{
				printf("divisor 为0");
				system("pause");
			}
			int sizeW = rand() % divisorW + stdsizeW;
			//printf("sizeW=%d\n",sizeW);
			int sizeH = rand() % divisorH + stdsizeH;
			//printf("sizeH=%d\n",sizeH);
			int  nx= rand()%(width - sizeW);
			// printf("nx=%d\n",nx);
			int ny=rand()%(height - sizeH);
			//printf("ny=%d\n",ny);
			Rect crop_box;crop_box.x=nx;crop_box.y=ny;crop_box.width =sizeW;crop_box.height =sizeH;
			float Iou=IoU(crop_box,bbox);
          // printf("x=%d,y=%d,w=%d,h=%d\n",crop_box.x,crop_box.y,crop_box.width,crop_box.height);
			//printf("x2=%d,y2=%d,width=%d,height=%d\n",crop_box.x+crop_box.width-1,crop_box.y+crop_box.height-1,img.cols,img.rows);
			Mat cropped_im(img,crop_box);
			Mat resized_im;
			resize(cropped_im,resized_im,Size(stdsizeW, stdsizeH));
			if(Iou<0.3)
			{
				char save_file[128];
				sprintf(save_file,"%s/%d.jpg",neg_save_dir.c_str(),n_idx);
				fprintf(f2,"%s/%d.jpg 0 -1 -1 -1 -1\n",neg_save_dir.c_str(),n_idx);    
				imwrite(save_file, resized_im);
				n_idx += 1;
				neg_num += 1;
			}
		}
		int  w = x2 - x1 + 1;										  
		int h = y2 - y1 + 1;
		if(w<stdsizeW||h<stdsizeH)
		{			
			printf("随机切割的样本小于标准样本大小!\n");
			continue;
		}

		if(x1<0||y1<0||x2>width||y2>height)
		{
			printf("x1=%d,y1=%d,x2=%d,y2=%d\n");
			printf("路径:%s\n",annotation.c_str());
			printf("一个标记样本坐标不对!\n");
			system("pause");
			continue;
		}
		for(int j=0;j<50;j++)
		{
			//printf("pos:j=%d\n",j);

			int leftW=w*0.8;	int rightW=w*1.25;
			int leftH=h*0.8;	int rightH=h*1.25;
			int sizeW=rand()%(rightW-leftW+1)+leftW;
			int sizeH=rand()%(rightH-leftH+1)+leftH;
			int xw=0.4*w+1;
			int delta_x=rand()%(xw)- (0.2*w);
			int yh=0.4*h+1;
			int delta_y=rand()%(yh)-(0.2*h);
			int nx1=max(x1+w/2+delta_x-sizeW/2,0);
			int ny1=max(y1 + h / 2 + delta_y - sizeH / 2, 0);
			int nx2=nx1+sizeW-1;
			int ny2=ny1+sizeH-1;
			if(nx2 >=width || ny2 >= height)
				continue;
			Rect crop_box;crop_box.x=nx1;crop_box.y=ny1;crop_box.width=sizeW;crop_box.height=sizeH;

			float offset_x1 = (x1 - nx1) / float(sizeW);
			float offset_y1 = (y1 - ny1) / float(sizeH);
			float offset_x2 = (x2 - nx1) / float(sizeW);
			float offset_y2 = (y2 - ny1) / float(sizeH);

			//printf("x=%d,y=%d,w=%d,h=%d\n",nx1,ny1,sizeW,sizeH);
			//printf("x2=%d,y2=%d,width=%d,height=%d\n",nx1+sizeW-1,ny1+sizeH-1,img.cols,img.rows);
			Mat cropped_im (img,Rect(nx1,ny1,sizeW,sizeH));
			Mat   resized_im;
			resize(cropped_im,resized_im, Size(stdsizeW, stdsizeH));
			float Iou=IoU(crop_box,bbox);
			if(Iou>=0.65)
			{
				char save_file[128]; 
				sprintf(save_file,"%s/%d.jpg",pos_save_dir.c_str(), p_idx);
				fprintf(f1,"%s/%d.jpg %d %f %f %f %f\n",pos_save_dir.c_str(),p_idx,IsampleClass,offset_x1, offset_y1, offset_x2, offset_y2);               
				imwrite(save_file, resized_im);
				p_idx += 1;
			}
			else if(Iou>=0.4)
			{
				char save_file[128];
				sprintf(save_file,"%s/%d.jpg",part_save_dir.c_str(), d_idx);
				fprintf(f3,"%s/%d.jpg -1 %f %f %f %f\n",part_save_dir.c_str(),d_idx,offset_x1, offset_y1, offset_x2, offset_y2);       
				
				
				
				 imwrite(save_file, resized_im);
				d_idx += 1;
			}

		}
		box_idx += 1;
		printf("%d images done, pos: %d part: %d neg: %d\n",idx, p_idx, d_idx, n_idx);		
	}

	fclose(f1);
	fclose(f2);
	fclose(f3);
	return 0;
}