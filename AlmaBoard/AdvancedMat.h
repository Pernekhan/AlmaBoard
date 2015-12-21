#ifndef ADVANCEDMATH_H
#define ADVANCEDMATH_H

#include "MatCompressor.h"
#include "tools.h"

struct AdvancedMat
{
	MatCompressor compressor = MatCompressor();
	Mat matrix;
	vector < Point2f > corners;
	Mat controlMatrix, prevMatrix;
	cv::Mat quad;
	std::vector<cv::Point2f> quad_pts;
	Mat kernel_ero = getStructuringElement(MORPH_RECT, Size(2, 2)); // Erode kernel filtering for making little bit bigger
	int body_eps = 5;
	set < pair < int, int > > addedPoints;

	void sortCorners(std::vector<cv::Point2f>& corners,
		cv::Point2f center)
	{
		std::vector<cv::Point2f> top, bot;

		for (int i = 0; i < corners.size(); i++)
		{
			if (corners[i].y < center.y)
				top.push_back(corners[i]);
			else
				bot.push_back(corners[i]);
		}
		corners.clear();

		if (top.size() == 2 && bot.size() == 2){
			cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
			cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
			cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
			cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

			corners.push_back(tl);
			corners.push_back(tr);
			corners.push_back(br);
			corners.push_back(bl);
		}
	}

	Point getMassCenter(vector < Point2f > corners){
		Point2f center = Point2f(0, 0);
		for (int i = 0; i < corners.size(); i++)
			center += corners[i];
		center *= (1. / corners.size());
		return center;
	}

	inline int maximalChange(const Mat &motion)
	{
		int number_of_changes = 0;
		int ans = 0;
		for (int i = 0; i < motion.rows; i++)
			for (int j = 0; j < motion.cols; j++){
				int val = static_cast<int>(motion.at<uchar>(i, j));
				if (val > 0){
					ans += val;
					number_of_changes++;
				}
			}
		return number_of_changes;
	}


	vector <Point2f> getPoint2f(vector < Point > &corners){
		vector < Point2f > res;
		for (int i = 0; i < corners.size(); i++)
			res.push_back(Point2f(corners[i].y, corners[i].x));
		return res;
	}
	AdvancedMat(){}
	AdvancedMat(Mat arr, vector < Point > _corners){
		corners = getPoint2f(_corners);
		matrix = arr;
		sortCorners(corners, getMassCenter(corners));
		int height = max(abs(corners[0].y - corners[3].y), abs(corners[1].y - corners[2].y));
		int width = max(abs(corners[0].x - corners[1].x), abs(corners[2].x - corners[3].x));
		width = 606; height = 381;
		quad = cv::Mat::zeros(height, width, CV_8UC3);
		quad_pts.push_back(cv::Point2f(0, 0));
		quad_pts.push_back(cv::Point2f(quad.cols, 0));
		quad_pts.push_back(cv::Point2f(quad.cols, quad.rows));
		quad_pts.push_back(cv::Point2f(0, quad.rows));
		MatCompressor::sendWidthHeight(width, height);

		immshow("quad", quad);
		cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
		cv::warpPerspective(matrix, quad, transmtx, quad.size());
		matrix = quad.clone();
		controlMatrix = quad.clone();
		prevMatrix = matrix.clone();
		cvtColor(controlMatrix, controlMatrix, CV_RGB2GRAY);
		//GaussianBlur(controlMatrix, controlMatrix, Size(5, 5), 0, 0);

		//imwrite(DIR + "transformedMatrix.png", matrix);
	}

	bool isBound(int x, int y, int n, int m){
		return x >= 0 && x < n && y >= 0 && y < m;
	}


	void dfs(int vi, int vj, vector < vector < bool > > &g, vector < vector < bool > > &used){
		cout << "DFS" << vi << " " << vj << endl;
		used[vi][vj] = true;

		for (int i = vi - body_eps; i <= vi + body_eps; i++){
			for (int j = vj - body_eps; j <= vj + body_eps; j++){
				int toi = i;
				int toj = j;
				if (!isBound(toi, toj, g.size(), g[0].size()))continue;
				if (used[toi][toj]) continue;
				if (!g[toi][toj]) continue;
				dfs(toi, toj, g, used);
			}
		}

	}

	void increaseBorders(vector < vector < bool > > &used, int n, int m){
		for (int i = 0; i < n; i++){
			for (int j = 0; j < m; j++){
				if (used[i][j]){
					for (int k = max(0, j - 5); k <= j; k++) used[i][k] = true;
				}
			}

			for (int j = m - 1; j >= 0; j--){
				if (used[i][j]){
					for (int k = j; k < min(m, j + 5); k++) used[i][k] = true;
				}
			}
		}
	}

	void addPoint(int x, int y){
		addedPoints.insert(make_pair(x, y));
	}

	void flushAddedPoints(){
		ofstream out(DIRHTDOCS_TXT);
		for (set < pair < int, int > > ::iterator it = addedPoints.begin(); it != addedPoints.end(); it++){
			pair < int, int > p = *it;
			out << p.first << " " << p.second << endl;
		}
		out.close();
	}

	void getBodyErasedMatrix(Mat arr, Mat matrix, Mat prevMatrix){
		vector < vector < bool > > g(arr.rows, vector < bool >(arr.cols, 0));
		vector < vector < bool > > used(arr.rows, vector < bool >(arr.cols, 0));

		for (int i = 0; i < arr.rows; i++){
			for (int j = 0; j < arr.cols; j++){
				int val = static_cast<int>(arr.at<uchar>(i, j));
				g[i][j] = (val == 255);
			}
		}

		int n = arr.rows;
		int m = arr.cols;

		queue < int > Q;

		body_eps = 1;

		for (int i = 0; i < n; i++){
			for (int j = 0; j < m; j++){
				if (used[i][j]) continue;
				if (!g[i][j]) continue;
				if (i < body_eps || j < body_eps || i >= n - body_eps || j >= m - body_eps){
					Q.push(i*m + j);
					used[i][j] = true;
				}
			}
		}

		int di[] = { 1, 0, -1, 0 };
		int dj[] = { 0, 1, 0, -1 };

		while (!Q.empty()){
			int vi = Q.front() / m;
			int vj = Q.front() % m;
			Q.pop();
			for (int i = 0; i < 4; i++){
				int toi = vi + di[i];
				int toj = vj + dj[i];
				if (!isBound(toi, toj, g.size(), g[0].size()))continue;
				if (used[toi][toj]) continue;
				if (!g[toi][toj]) continue;
				Q.push(toi*m + toj);
				used[toi][toj] = true;
			}
		}

		//increaseBorders(used, n, m);
		cout << n << " " << m << endl;

		for (int i = 0; i < arr.rows; i++){
			for (int j = 0; j < arr.cols; j++){
				int val = static_cast<int>(matrix.at<uchar>(i, j));
				g[i][j] = (val == 255);
			}
		}

		for (int i = 0; i < n; i++){
			for (int j = 0; j < m; j++){
				if (!used[i][j] && g[i][j]){
					addPoint(i, j);
				}
			}
		}

		flushAddedPoints();
	}


	void getBodyErasedMatrixBfs(Mat &arr, Mat &matrix){
		vector < vector < bool > > g(arr.rows, vector < bool >(arr.cols, 0));
		vector < vector < bool > > used(arr.rows, vector < bool >(arr.cols, 0));

		for (int i = 0; i < arr.rows; i++){
			for (int j = 0; j < arr.cols; j++){
				int val = static_cast<int>(arr.at<uchar>(i, j));
				g[i][j] = (val == 255);
			}
		}

		int n = arr.rows;
		int m = arr.cols;

		queue < int > Q;

		body_eps = 1;

		for (int i = 0; i < n; i++){
			for (int j = 0; j < m; j++){
				if (used[i][j]) continue;
				if (!g[i][j]) continue;
				if (i < body_eps || j < body_eps || i >= n - body_eps || j >= m - body_eps){
					Q.push(i*m + j);
					used[i][j] = true;
				}
			}
		}

		int di[] = { 1, 0, -1, 0 };
		int dj[] = { 0, 1, 0, -1 };

		while (!Q.empty()){
			int vi = Q.front() / m;
			int vj = Q.front() % m;
			Q.pop();
			for (int i = 0; i < 4; i++){
				int toi = vi + di[i];
				int toj = vj + dj[i];
				if (!isBound(toi, toj, g.size(), g[0].size()))continue;
				if (used[toi][toj]) continue;
				if (!g[toi][toj]) continue;
				Q.push(toi*m + toj);
				used[toi][toj] = true;
			}
		}

		//increaseBorders(used, n, m);
		cout << n << " " << m << endl;

		for (int i = 0; i < arr.rows; i++){
			for (int j = 0; j < arr.cols; j++){
				int val = static_cast<int>(matrix.at<uchar>(i, j));
				g[i][j] = (val == 255);
			}
		}

		for (int i = 0; i < n; i++){
			for (int j = 0; j < m; j++){
				if (!used[i][j] && g[i][j]){
					addPoint(i, j);
				}
			}
		}

		flushAddedPoints();
	}

	int getDarkestPixel(Mat arr){
		int mini = inf;
		for (int i = 0; i < arr.rows; i++){
			for (int j = 0; j < arr.cols; j++){
				int val = static_cast<int>(arr.at<uchar>(i, j));
				mini = min(mini, val);
			}
		}
		return mini;
	}

	void contrast(Mat &image){
		double alpha = 2.2;
		int beta = 50;

		/// Do the operation new_image(i,j) = alpha*image(i,j) + beta
		for (int y = 0; y < image.cols; y++)
		{
			for (int x = 0; x < image.rows; x++)
			{
				for (int c = 0; c < 3; c++)
				{
					image.at<Vec3b>(y, x)[c] =
						saturate_cast<uchar>(alpha*(image.at<Vec3b>(y, x)[c]) + beta);
				}
			}
		}

	}

	Mat makeBright(int beta, Mat mat){
		Mat nmat = Mat(mat.size(), mat.type());
		for (int i = 0; i < mat.rows; i++){
			for (int j = 0; j < mat.cols; j++){
				int val = static_cast<int>(mat.at<uchar>(i, j));
				nmat.at<uchar>(i, j) = max(0, min(255, val + beta));
			}
		}
		return nmat;
	}

	void detectEdgesCanny(Mat src_gray){
		int thresh = 5;
		int max_thresh = 255;
		RNG rng(12345);

		Mat canny_output;
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		/// Detect edges using canny
		Canny(src_gray, canny_output, thresh, thresh * 2, 3);
		/// Find contours
		findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		imshow("Canny output", canny_output);

		/// Draw contours
		Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			//Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			drawContours(drawing, contours, i, 255, 2, 8, hierarchy, 0, Point());
		}

		/// Show in a window
		namedWindow("Contours", CV_WINDOW_AUTOSIZE);
		imshow("Contours", drawing);
	}

	void contourDetectionAttemptCanny(Mat matrix){
		Mat orig_img = matrix.clone();
		Mat _img;

		double otsu_thresh_val = cv::threshold(
			orig_img, _img, 0, 150, CV_THRESH_BINARY | CV_THRESH_OTSU
			);

		double high_thresh_val = otsu_thresh_val,
			lower_thresh_val = otsu_thresh_val * 0.5;

		Mat cannyOP;
		cv::Canny(orig_img, cannyOP, lower_thresh_val, high_thresh_val);


		immshow("cannyOP", cannyOP);

	}

	void contourDetectionAttemptColored(Mat matrix){
		Mat image = matrix.clone();
		Mat output;
		inRange(image, cv::Scalar(0, 125, 0), cv::Scalar(255, 200, 255), output);
		immshow("output", output);
	}

	void sendingMat(Mat matrix){
		Mat copyMatrix = matrix.clone();
		GaussianBlur(copyMatrix, copyMatrix, Size(3, 3), 0, 0);

		adaptiveThreshold(copyMatrix, copyMatrix, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 2);

		compressor.process(copyMatrix);
		compressor.sendData();

		immshow("copyMatrix", copyMatrix);

		//contourDetectionAttemptCanny(matrix);
	}

	bool hasANoise(Mat matrix){
		Mat noiseMatrix = matrix.clone();
		GaussianBlur(noiseMatrix, noiseMatrix, Size(5, 5), 0, 0);
		adaptiveThreshold(noiseMatrix, noiseMatrix, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 2);
		//dilate(cannyOP, cannyOP, kernel_ero);

		int n = noiseMatrix.rows;
		int m = noiseMatrix.cols;
		int epsn = n * 5 / 100;
		int epsm = m * 5 / 100;
		for (int i = epsn; i < n - epsn; i++){
			for (int k = 1; k <= 5; k++)
				if (static_cast<int>(noiseMatrix.at<uchar>(i, m - k)) == 0 ||
					static_cast<int>(noiseMatrix.at<uchar>(i, k)) == 0) return true;
		}
		for (int j = epsm; j < m - epsm; j++){
			for (int k = 1; k <= 5; k++)
				if (static_cast<int>(noiseMatrix.at<uchar>(k, j)) == 0 ||
					static_cast<int>(noiseMatrix.at<uchar>(n - k, j) == 0)) return true;
		}
		immshow("cannyOP", noiseMatrix);
		return false;
	}

	void process(Mat arr){
		cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
		cv::warpPerspective(arr, quad, transmtx, quad.size());
		matrix = quad;
		Mat original_matrix = matrix.clone();
		cvtColor(matrix, matrix, CV_RGB2GRAY);
		if (hasANoise(matrix)) return;
		sendingMat(matrix);
	}


};

class CVBot
{
public:
	CVBot(const cv::Point &init_point);
	~CVBot();

private:
	const cv::Point init_point;
};

#endif /* CVBOT_H */