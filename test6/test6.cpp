//opencv version 3.4.6


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>

using namespace cv;
using namespace std;


// Read points stored in the text files
vector<Point2f> readPoints(string pointsFileName)
{
    vector<Point2f> points;
    ifstream ifs(pointsFileName);
    float x, y;
    while(ifs >> x >> y)
    {
        points.push_back(Point2f(x,y));
    }
    
    return points;
}

// Apply affine transform calculated using srcTri and dstTri to src
void applyAffineTransform(Mat &warpImage, Mat &src, vector<Point2f> &srcTri, vector<Point2f> &dstTri)
{
    
    // Given a pair of triangles, find the affine transform.
    Mat warpMat = getAffineTransform( srcTri, dstTri );
    
    // Apply the Affine Transform just found to the src image
    warpAffine( src, warpImage, warpMat, warpImage.size(), INTER_LINEAR, BORDER_REFLECT_101);
}

// Warps and alpha blends triangular regions from img1 and img2 to img
void morphTriangle(Mat &img1, Mat &img2, Mat &img, vector<Point2f> &t1, vector<Point2f> &t2, vector<Point2f> &t, double alpha)
{
    
    // Find bounding rectangle for each triangle
    Rect r = boundingRect(t);
    Rect r1 = boundingRect(t1);
    Rect r2 = boundingRect(t2);
    
    // Offset points by left top corner of the respective rectangles
    vector<Point2f> t1Rect, t2Rect, tRect;
    vector<Point> tRectInt;
    for(int i = 0; i < 3; i++)
    {
        tRect.push_back( Point2f( t[i].x - r.x, t[i].y -  r.y) );
        tRectInt.push_back( Point(t[i].x - r.x, t[i].y - r.y) ); // for fillConvexPoly
        
        t1Rect.push_back( Point2f( t1[i].x - r1.x, t1[i].y -  r1.y) );
        t2Rect.push_back( Point2f( t2[i].x - r2.x, t2[i].y - r2.y) );
    }
    
    // Get mask by filling triangle
    Mat mask = Mat::zeros(r.height, r.width, CV_32FC3);
    fillConvexPoly(mask, tRectInt, Scalar(1.0, 1.0, 1.0), 16, 0);
    
    // Apply warpImage to small rectangular patches
    Mat img1Rect, img2Rect;
    img1(r1).copyTo(img1Rect);
    img2(r2).copyTo(img2Rect);
    
    Mat warpImage1 = Mat::zeros(r.height, r.width, img1Rect.type());
    Mat warpImage2 = Mat::zeros(r.height, r.width, img2Rect.type());
    
    applyAffineTransform(warpImage1, img1Rect, t1Rect, tRect);
    applyAffineTransform(warpImage2, img2Rect, t2Rect, tRect);
    
    // Alpha blend rectangular patches
    Mat imgRect = (1.0 - alpha) * warpImage1 + alpha * warpImage2;
    
    // Copy triangular region of the rectangular patch to the output image
    multiply(imgRect,mask, imgRect);
    multiply(img(r), Scalar(1.0,1.0,1.0) - mask, img(r));
    img(r) = img(r) + imgRect;
    
    
}

int main( int argc, char** argv)
{
	
    string filename1("E:\\Solutions\\Experments_of_Computer_Animation_Technology_Foundation\\test6\\donald_trump.jpg");
    string filename2("E:\\Solutions\\Experments_of_Computer_Animation_Technology_Foundation\\test6\\BARACK-OBAMA.jpg"	);
    

	string videoName = "E:\\Solutions\\Experments_of_Computer_Animation_Technology_Foundation\\test6\\video\\test6.avi";
	VideoWriter writer(videoName, CV_FOURCC('M', 'J', 'P', 'G'), 24, Size(600, 800), 1);

    //alpha controls the degree of morph
    double alpha = 0.0;
    //Read input images
    Mat img1 = imread(filename1);
    Mat img2 = imread(filename2);


	Mat imgW;

    
    //convert Mat to float data type
    img1.convertTo(img1, CV_32F);
    img2.convertTo(img2, CV_32F);
    
	//cout << img1.channels() << endl; //打印查看图片的通道
	//cout << img2.size() << endl;     //打印查看图片的尺寸
    
    //empty average image
    Mat imgMorph = Mat::zeros(img1.size(), CV_32FC3);
    
    
    //Read points
    vector<Point2f> points1 = readPoints( filename1 + ".txt");
    vector<Point2f> points2 = readPoints( filename2 + ".txt");
    vector<Point2f> points;
	

	for (int k = 0; k <= 120; k++) {
		alpha = (double)k / 120;

		//compute weighted average point coordinates
		for (int i = 0; i < points1.size(); i++)
		{
			float x, y;
			x = (1 - alpha) * points1[i].x + alpha * points2[i].x;
			y = (1 - alpha) * points1[i].y + alpha * points2[i].y;

			points.push_back(Point2f(x, y));

		}


		//Read triangle indices
		ifstream ifs("E:\\Solutions\\Experments_of_Computer_Animation_Technology_Foundation\\test6\\tri.txt");
		int x, y, z;

		while (ifs >> x >> y >> z)
		{
			// Triangles
			vector<Point2f> t1, t2, t;

			// Triangle corners for image 1.
			t1.push_back(points1[x]);
			t1.push_back(points1[y]);
			t1.push_back(points1[z]);

			// Triangle corners for image 2.
			t2.push_back(points2[x]);
			t2.push_back(points2[y]);
			t2.push_back(points2[z]);

			// Triangle corners for morphed image.
			t.push_back(points[x]);
			t.push_back(points[y]);
			t.push_back(points[z]);

			morphTriangle(img1, img2, imgMorph, t1, t2, t, alpha);

		}

		imgW = imgMorph;
		imgW.convertTo(imgW, CV_8U);//输出视频需要把图片转成支持的格式
		cout << "Writing NO." << k + 1 << " frame..." << endl;
		writer.write(imgW);
		

	}
   /* imshow("Morphed Face", imgMorph / 255.0);
	waitKey(0);*/
    // Display Result
	cout << endl;
	cout << "Done." << endl;
    
    return 0;
}
