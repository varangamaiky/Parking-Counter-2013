#include<opencv2/opencv.hpp>
#include<iostream>
#include<vector>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

int parking_count = 0;
bool traceParking = false;

vector<Point> parking; // polygon that enclose a parking space;


/*
 * Drawing ang getting the rect (polygon) that
 * encircle a parking space.
 */
void onMouse( int event, int x, int y, int flags, void* param )
{
    switch( event )
    {
        case CV_EVENT_LBUTTONDOWN:
            if (traceParking)
            {
                parking.push_back(Point(x,y));
            }
            break;
        case CV_EVENT_LBUTTONUP:
            break;
    } 
}


/*
 * Draw area that is enclosing the parking space.
 *
 */
void drawPaking(Mat &img)
{
    // Draw parking space
    if (!traceParking)
    {
        //drawContours(frame, parking, -1, Scalar(0,0,255), 2);
        const Point *pts = (const Point*) Mat(parking).data;
        int npts = Mat(parking).rows;
        //cout << "Number of polygon vertices: " << npts << endl;
        // draw the polygon 
        polylines(img, &pts,&npts, 1,
                true,           // draw closed contour (i.e. joint end to start) 
                Scalar(0,255,0),// colour RGB ordering (here = green) 
                3,              // line thickness
                CV_AA, 0);
    }
}


int main(int argc, char *argv[])
{
    RNG rng(12345);

    Mat frame;
    Mat back;
    Mat fore;
    VideoCapture cap(0);

    const int nmixtures =3;
    const bool bShadowDetection = false;
    const int history = 4;
    double dist2Center;
    BackgroundSubtractorMOG2 bg (history,nmixtures,bShadowDetection);
    //BackgroundSubtractorMOG2 bg; 
    //bg.nmixtures = 3;
    //bg.bShadowDetection = false;

    vector<vector<Point> > contours;

    namedWindow("Frame");
    namedWindow("Background");
    setMouseCallback( "Frame", onMouse, 0 );
    //createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );

    for(;;)
    {
        cap >> frame;
        bg.operator ()(frame,fore);
        bg.getBackgroundImage(back);
        erode(fore,fore,Mat());
        dilate(fore,fore,Mat());
        findContours(fore,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE); // CV_RETR_EXTERNAL retrieves only the extreme outer contours
        //drawContours(frame,contours,-1,Scalar(0,0,255),2); // This draws all the contours
        // Only draw important contours

	vector<vector<Point> > contours_poly( contours.size() );
  	vector<Rect> boundRect( contours.size() );
  	vector<Point2f>center( contours.size() );
  	vector<float>radius( contours.size() );

        for( unsigned int i = 0; i< contours.size(); i++ )
        {
            Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
            //drawContours(frame, contours, i, color, 2, 8, hierarchy, 0, Point() );
            approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true);
			boundRect[i] = boundingRect ( Mat(contours_poly[i]) );
				
            double area = boundRect[i].area();
            if (area > 5000)
            {
                //printf("Area[%d]: %.2f\n", i, contourArea(contours[i]));
                //drawContours(frame,contours,i,color,2); // This draws all the contours

                // Track if an object of interest has crossed a roi
               
				//minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i]);
               
				// double pointPolygonTest(const Mat& contour, Point2f pt, bool measureDist)

                //Rect box = boundingRect(contours[i]); // Get bounding box around a countour
                // cout << "[x=" << box.x << " , y=" << box.y << ",w=" << box.width << ",h=" << box.height << "]\n"; 
                Point2f center(boundRect[i].x + boundRect[i].width/2.0, boundRect[i].y + boundRect[i].height/2.0);

                // Test if the center of a contour has crossed ROI (direction: going in or out)
                if (parking.size() > 3)
                {
                	dist2Center = pointPolygonTest(parking, center, true);
                }
                cout << center << "is " << dist2Center << " distance from the contour. \n"; 
                putText(frame, "I", center, FONT_HERSHEY_COMPLEX_SMALL, 1.5, color, 1);
                rectangle(frame, boundRect[i].tl(), boundRect[i].br(), Scalar(100, 100, 200), 2, CV_AA);
            }
        }


        /*
         * Draw parking zone
         */
        for (unsigned int j = 0; j < parking.size(); j++)
        {
            circle(frame, parking[j], 5, Scalar(0,0,255), -1);
        }
        drawPaking(frame);

        imshow("Frame",frame);
        imshow("Background",back);

        char c = (char)waitKey(30);
        if( c == 27 || c == 'q')
            break;
        switch(c)
        {
            case 's':
                traceParking = !traceParking; 
                break;
            case 'c':
                parking.clear();
                break;
            default:
                ;
        }
    }       

    return 0;
}
