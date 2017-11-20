#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <iostream>
using namespace cv;
using namespace std;
typedef uchar* PBYTE;
enum {
	YUV_NV21 = 0,
	YUV_NV12,
	YUV_I420,
	YUV_YV12,
	YUV_YUYV,
	YUV_UYVY,
	YUV_444,
	YUV_NUMBER,
};
#define ALIGN_DOWN(x, align) ((x) & ~((align)-1))

void _BGRtoYUV(const Vec3b& rgb, Vec3b& yuv)
{
	int B = rgb[0];
	int G = rgb[1];
	int R = rgb[2];
	int Y = (int)(0.299f * R + 0.587f * G + 0.114f * B);
	int U = (int)((B-Y) * 0.565f + 128);
	int V = (int)((R-Y) * 0.713f + 128);

	yuv[0]= saturate_cast<uchar>(Y);
	yuv[1]= saturate_cast<uchar>(U);
	yuv[2]= saturate_cast<uchar>(V);
}
int GetYUVSize(int w, int h, int type)
{
	int height;
	switch(type)
	{
	case YUV_NV12:
	case YUV_NV21:
	case YUV_I420:
	case YUV_YV12:
		height = h*3/2;
		break;
	case YUV_YUYV:
	case YUV_UYVY:
		height = h*2;
		break;
	case YUV_444:
		height = h*3;
		break;
	}
	return w*height;
}
void _YUVtoBGR(Vec3b& rgb, const Vec3b& yuv)
{
	int Y = yuv[0];
	int U = yuv[1] - 128;
	int V = yuv[2] - 128;

	int R = (int)( Y + 1.403f * V);
	int G = (int)( Y - 0.344f * U - 0.714f * V);
	int B = (int)( Y + 1.770f * U);

	rgb[0]= saturate_cast<uchar>(B);
	rgb[1]= saturate_cast<uchar>(G);
	rgb[2]= saturate_cast<uchar>(R);
}

void ImageYUV2BGR(Mat& img, PBYTE pYUV, int type)
{
	int w = img.cols;
	int h = img.rows;
	w = ALIGN_DOWN(w, 4);
	h = ALIGN_DOWN(h, 4);
	Vec3b yuv;
	switch(type)
	{
	case YUV_NV21:
	case YUV_NV12:
		{
			PBYTE pUV = pYUV+w*h;
			PBYTE pY = pYUV;
			int vidx = type == YUV_NV12 ? 1 : 2;
			int uidx = type == YUV_NV12 ? 2 : 1;
			for (int y = 0;y < h;y++)
			{
				for (int x = 0;x < w;x++)
				{
					yuv[0] = *pY++;
					yuv[uidx] = pUV[y/2*w+x/2*2];
					yuv[vidx] = pUV[y/2*w+x/2*2+1];
					_YUVtoBGR(img.at<Vec3b>(y, x), yuv);
				}
			}
		}
		break;
	case YUV_I420:
	case YUV_YV12:
		{
			PBYTE pY = pYUV;
			PBYTE pV = pYUV+w*h*5/4;
			PBYTE pU = pYUV+w*h;
			int uidx = type == YUV_I420 ? 1 : 2;
			int vidx = type == YUV_I420 ? 2 : 1;
			for (int y = 0;y < h;y++)
			{
				for (int x = 0;x < w;x++)
				{
					yuv[0] = *pY++;

					yuv[uidx] = pU[y/4*w+x/2+w*y/4%2];
					yuv[vidx] = pV[y/4*w+x/2+w*y/4%2];

					_YUVtoBGR(img.at<Vec3b>(y, x), yuv);

				}
			}
		}
		break;
	case YUV_YUYV:
	case YUV_UYVY:
		{
			int yidx = type == YUV_YUYV ? 0 : 1;
			int uidx = type == YUV_YUYV ? 1 : 0;
			int vidx = type == YUV_YUYV ? 3 : 2;
			PBYTE pY = pYUV + yidx;
			PBYTE pV = pYUV + vidx;
			PBYTE pU = pYUV + uidx;


			for (int y = 0;y < h;y++)
			{
				for (int x = 0;x < w;x++)
				{
					yuv[0] = *pY;
					pY+=2;
					if(x%2==0) {
						yuv[1] = *pU;
						yuv[2] = *pV;
						pU+=4;
						pV+=4;
					}
					_YUVtoBGR(img.at<Vec3b>(y, x), yuv);
				}
			}
		}
		break;
	case YUV_444:
		{
			PBYTE pY = pYUV;
			PBYTE pU = pYUV + w*h;
			PBYTE pV = pYUV + w*h*2;
			for (int y = 0;y < h;y++)
			{
				for (int x = 0;x < w;x++)
				{
					yuv[0] = *pY++;
					yuv[1] = *pU++;
					yuv[2] = *pV++;
					_YUVtoBGR(img.at<Vec3b>(y, x), yuv);
				}
			}
		}
		break;
	}

}

void ImageBGR2YUV(Mat& img, PBYTE pYUV, int type)
{
	int w = img.cols;
	int h = img.rows;
	w = ALIGN_DOWN(w, 4);
	h = ALIGN_DOWN(h, 4);
	Vec3b yuv;
	switch(type)
	{

	case YUV_NV21:
	case YUV_NV12:
		{
			PBYTE pUV = pYUV+w*h;
			PBYTE pY = pYUV;
			int vidx = type == YUV_NV12 ? 1 : 2;
			int uidx = type == YUV_NV12 ? 2 : 1;
			for (int y = 0;y < h;y++)
			{
				for (int x = 0;x < w;x++)
				{
					Vec3b c = img.at<Vec3b>(y, x);
					_BGRtoYUV(c, yuv);
					*pY++ = yuv[0];
					if(y%2==0&&x%2==0) {
						pUV[y/2*w+x/2*2] = yuv[uidx];
						pUV[y/2*w+x/2*2+1] = yuv[vidx];
					}
				}
			}
		}
		break;
	case YUV_I420:
	case YUV_YV12:
		{
			PBYTE pY = pYUV;
			PBYTE pV = pYUV+w*h*5/4;
			PBYTE pU = pYUV+w*h;
			int uidx = type == YUV_I420 ? 1 : 2;
			int vidx = type == YUV_I420 ? 2 : 1;
			for (int y = 0;y < h;y++)
			{
				for (int x = 0;x < w;x++)
				{
					Vec3b c = img.at<Vec3b>(y, x);
					_BGRtoYUV(c, yuv);
					*pY++ = yuv[0];
					if(y%2==0&&x%2==0) {
						*pU++ = yuv[uidx];
						*pV++ = yuv[vidx];
					}
				}
			}
		}
		break;
	case YUV_YUYV:
	case YUV_UYVY:
		{
			int yidx = type == YUV_YUYV ? 0 : 1;
			int uidx = type == YUV_YUYV ? 1 : 0;
			int vidx = type == YUV_YUYV ? 3 : 2;
			PBYTE pY = pYUV + yidx;
			PBYTE pV = pYUV + vidx;
			PBYTE pU = pYUV + uidx;

			for (int y = 0;y < h;y++)
			{
				for (int x = 0;x < w;x++)
				{
					Vec3b c = img.at<Vec3b>(y, x);
					_BGRtoYUV(c, yuv);
					*pY = yuv[0];
					pY+=2;
					if(x%2==0) {
						*pU = yuv[1];
						*pV = yuv[2];           
						pU+=4;
						pV+=4;
					}

				}
			}
		}
		break;
	case YUV_444:
		{
			PBYTE pY = pYUV;
			PBYTE pU = pYUV + w*h;
			PBYTE pV = pYUV + w*h*2;

			for (int y = 0;y < h;y++)
			{
				for (int x = 0;x < w;x++)
				{
					Vec3b c = img.at<Vec3b>(y, x);
					_BGRtoYUV(c, yuv);
					*pY++ = yuv[0];
					*pU++ = yuv[1];
					*pV++ = yuv[2];
				}
			}
		}
		break;  
	}
}
int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

    Mat image;
#if 1 
    image = imread( argv[1], CV_LOAD_IMAGE_UNCHANGED );

    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", image);
    string path = argv[1];
    string str = "/home/zyyin/yuv/";
    str += path.substr(path.find_last_of("/") , path.find_last_of(".")- path.find_last_of("/")) + ".yuv";
    cout<<str <<endl;
    int size = image.cols*image.rows*3/2;
    uchar *pByte = new uchar[size];
    ImageBGR2YUV(image, pByte, YUV_YV12);
    
    FILE* p = fopen(str.c_str(), "wb");
    fwrite(pByte, size, 1, p);
    fclose(p);
    waitKey(33);
#else   
    int w = 540;int h = 400;
    FILE* p = fopen(argv[1], "rb");
    int size = w*h*3/2;
    uchar *pByte = new uchar[w*h*3/2];
    fread(pByte, size, 1, p);
    fclose(p);
    image.create(h, w, CV_8UC3);
    ImageYUV2BGR(image, pByte, YUV_YV12);
    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", image);
    waitKey(0);
#endif
    
    return 0;
}
