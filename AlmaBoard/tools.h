#ifndef tools_h
#define tools_h


#include <opencv2/opencv.hpp>
#include <time.h>

#include <iostream>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/video/background_segm.hpp"
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/video/background_segm.hpp"
#include <stdio.h>

#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <cassert>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <stack>
#include <functional>
#include <numeric>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <cassert>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace cv;

#define f first
#define s second
#define mp make_pair
#define pb push_back
#define pii pair<int, int>
#define pll pair<long long, long long>
#define y1 stupid_y1
#define ll long long
#define forit(it, s) for(__typeof(s.begin()) it = s.begin(); it != s.end(); it++)
#define all(a) a.begin(), a.end()
#define sqr(x) ((x)*(x))
#define sz(a) (int)a.size()
#define file "a"


const int inf = (int)1e9;
const double eps = 1e-9;
const double pi = acos(-1.0);

const int DELAY = 500;
int FRAME_SKIP = 5;
const string DIR = "C:/Users/Pernekhan Utemuratov/Desktop/Computer Vision/Pattern Recognition/images/TEST/";
const string DIRHTDOCS = "C:/xampp/htdocs/almaboard/transformedMatrix.png";
const string DIRHTDOCS_TXT = "C:/xampp/htdocs/almaboard/output.txt";
const string DIRHTDOCS_TXT2 = "C:/xampp/htdocs/almaboard/output2.txt";
const string DIRHTDOCS_TXT_WIDTH_HEIGHT = "C:/xampp/htdocs/almaboard/width_height.txt";
const string DIRHTDOCS_TXT_LOCKING = "C:/xampp/htdocs/almaboard/locking.txt";
const string VIDEO_NAME = "with_white_board";
int POINTS_FOUND_FRAME = 0;
int frameCount = 0;

const bool isDebugMode = true;

void immshow(string name, Mat &mat){
	if (!isDebugMode)return;
	Mat showMat = mat.clone();
	if (showMat.rows > 800 || showMat.cols > 800) resize(showMat, showMat, Size(960 / 1.6, 540 / 1.6));
	imshow(name, showMat);
}



#endif


