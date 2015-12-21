#ifndef BOARD_H
#define BOARD_H

#include "tools.h"
#include "AdvancedMat.h"
#include "ControlPoints.h"

struct Board{
	int NUM_CHANGES_LEN = 7;
	vector < int > numChanges;
	Mat prevFrame, curFrame, nextFrame;
	Mat originalFrame;
	int FIND_BOARD_INITIAL_STATE = 0; // waiting for hand cover
	int FIND_BOARD_WAITING_FOR_CHANGES = 1; // took the board matrix and waiting for the next hand cover
	int FIND_BOARD_FOUND_BOARD = 2; // Board points are found
	int MAX_DEVIATION = 100; // the more the value the more motion is allowed
	int MOTION_CHANGES = 10; // the percentage of big motion
	int HAND_COVER_PERCENT = 70; // the 70% of image should be changed to mean a hand cover
	Mat kernel_ero = getStructuringElement(MORPH_RECT, Size(4, 4)); // Erode kernel filtering for making little bit bigger
	bool initialImageSet = false, controlImageSet = false, controlPointsSet = false, advancedMatrixSet = false;
	Mat initialImage, controlImage;
	ControlPoints controlPoints;
	AdvancedMat advMat;


	int state = 0;


	Board(){

	}

	Board(Mat frame){
		originalFrame = frame;
		cvtColor(frame, frame, CV_RGB2GRAY);
		prevFrame = curFrame = nextFrame = frame;
		state = FIND_BOARD_INITIAL_STATE;
		for (int i = 0; i < NUM_CHANGES_LEN; i++) numChanges.push_back(-1);
	}

	void updatePrevCurNext(Mat frame){
		originalFrame = frame.clone();
		cvtColor(frame, frame, CV_RGB2GRAY);
		prevFrame = curFrame;
		curFrame = nextFrame;
		nextFrame = frame;
	}

	void process(Mat frame){
		updatePrevCurNext(frame);
		if (state == FIND_BOARD_INITIAL_STATE){
			initialState();
		}
		else if (state == FIND_BOARD_WAITING_FOR_CHANGES){
			waitingChanges();
		}
		else if (state == FIND_BOARD_FOUND_BOARD){
			processFrame();
		}
	}

	inline int detectMotion(const Mat &motion)
	{
		int number_of_changes = 0;
		for (int i = 0; i < motion.rows; i++)
			for (int j = 0; j < motion.cols; j++)
				if (static_cast<int>(motion.at<uchar>(i, j)) == 255)
					number_of_changes++;
		return number_of_changes;
	}


	void updateNumberOfChanges(int x){
		int len = numChanges.size();
		for (int i = 0; i < len - 1; i++){
			numChanges[i] = numChanges[i + 1];
		}
		numChanges[len - 1] = x;
	}

	void updateMotionStatistics(){
		Mat d1, d2, motion;
		absdiff(prevFrame, nextFrame, d1);
		absdiff(nextFrame, curFrame, d2);
		bitwise_and(d1, d2, motion);
		threshold(motion, motion, 35, 255, CV_THRESH_BINARY);
		erode(motion, motion, kernel_ero);
		int number_of_changes = detectMotion(motion);
		int area = motion.cols * motion.rows;
		updateNumberOfChanges(number_of_changes);
	}

	bool absoluteMotionless(){
		for (int i = 0; i < numChanges.size(); i++)
			if (numChanges[i] != 0) return false;
		return true;
	}

	bool isMotionDetected(){
		return numChanges.back() > 0;
	}

	bool isHandCoverDetected(Mat cmpMat){
		if (!isMotionDetected()) return false;
		Mat motion;
		absdiff(initialImage, nextFrame, motion);
		if (VIDEO_NAME == "20150604_172836"){
			threshold(motion, motion, 25, 255, CV_THRESH_BINARY);
		}
		else
			threshold(motion, motion, 35, 255, CV_THRESH_BINARY);
		erode(motion, motion, kernel_ero);
		int number_of_changes = detectMotion(motion);
		immshow("motion", motion);
		int area = motion.cols * motion.rows;
		cout << "The number of changes " << number_of_changes << endl;
		if (VIDEO_NAME == "20150604_172836")
			HAND_COVER_PERCENT = 50;
		if (number_of_changes > HAND_COVER_PERCENT*area / 100) return true;
		return false;
	}

	void initialState(){
		cout << "InitialState" << endl;
		updateMotionStatistics();
		if (absoluteMotionless() && !initialImageSet){
			cout << "AbsoluteMotionLess" << endl;
			initialImage = curFrame;
			initialImageSet = true;
		}

		if (initialImageSet && isHandCoverDetected(initialImage)){
			cout << "handCover identified" << endl;
			state = FIND_BOARD_WAITING_FOR_CHANGES;
		}
	}

	void waitingChanges(){
		cout << "Waiting changes state" << endl;
		updateMotionStatistics();
		if (absoluteMotionless() && !controlImageSet){
			cout << "Absolute Motionless : Control Image is Set" << endl;
			controlImageSet = true;
			controlImage = curFrame;
			immshow("control image", curFrame);
		}

		if (controlImageSet && isHandCoverDetected(controlImage)){
			cout << "handCover identified" << endl;
			state = FIND_BOARD_FOUND_BOARD;
		}

	}

	void dfs(int vi, int vj, vector < vector <bool> > &g, vector < vector<bool> > &used, vector < Point > &temp){
		used[vi][vj] = true;
		temp.push_back(Point(vi, vj));
		int x = vi;
		int y = vj;
		for (int i = -1; i <= 1; i++){
			for (int j = -1; j <= 1; j++){
				int toi = x + i;
				int toj = y + j;
				if (toi >= 0 && toj >= 0 && toi < sz(used) && toj < sz(used[0]) && !used[toi][toj] && g[toi][toj])
					dfs(toi, toj, g, used, temp);
			}
		}
	}

	bool compsComparator(vector < Point > a, vector < Point > b){
		return a.size() > b.size();
	}

	void pointsNotFound(){
		cout << "Points not found" << endl;
		return;
	}

	void findPoints(Mat &arr){
		int number_of_changes = 0;

		vector < vector < bool > > g(arr.rows, vector < bool >(arr.cols, 0));
		vector < vector < bool > > used(arr.rows, vector < bool >(arr.cols, 0));

		for (int i = 0; i < arr.rows; i++){
			for (int j = 0; j < arr.cols; j++){
				int val = static_cast<int>(arr.at<uchar>(i, j));
				g[i][j] = (val == 255);
			}
		}

		vector <vector<Point>> comps;
		for (int i = 0; i < arr.rows; i++){
			for (int j = 0; j < arr.cols; j++){
				if (!used[i][j] && g[i][j]){
					vector <Point> temp;
					dfs(i, j, g, used, temp);
					comps.push_back(temp);
				}
			}
		}

		// this works in O(n*n) time, should be optimized soon
		for (int i = 0; i < comps.size(); i++){
			for (int j = i + 1; j < comps.size(); j++){
				int a = comps[i].size();
				int b = comps[j].size();
				if (b > a){
					swap(comps[j], comps[i]);
				}
			}
		}

		vector < Point > res;
		for (int i = 0; i < comps.size(); i++){
			if (ControlPoints().canBeValidRectangle(comps[i])) res.push_back(ControlPoints().getMassCenter(comps[i]));
			if (ControlPoints().canBeValidRectangle(comps[i])){
				cout << "The mass center coordinates: " << res.back().x << " " << res.back().y << endl;
				//Sleep(1000);
			}
			if (sz(res) == 4) break;
		}
		controlPoints = ControlPoints(res);
		drawCircles();

	}

	void drawCircles(){
		if ((int)controlPoints.points.size() == 4){
			cout << "FOUNDED" << endl;
			for (int i = 0; i < controlPoints.points.size(); i++){
				Point cur = controlPoints.points[i];
				cout << "The points are " << cur.x << " " << cur.y << endl;
				circle(originalFrame, Point(cur.y, cur.x), 3, CV_RGB(255, 0, 0), 2);
			}
			immshow("found points", originalFrame);
			imwrite(DIR + "found_points3.png", originalFrame);
		}
	}

	void findPoints(){
		Mat tmpFrame;
		absdiff(controlImage, nextFrame, tmpFrame);
		GaussianBlur(tmpFrame, tmpFrame, Size(3, 3), 0, 0);
		threshold(tmpFrame, tmpFrame, 35, 255, CV_THRESH_BINARY);
		erode(tmpFrame, tmpFrame, kernel_ero);
		imwrite(DIR + "poitns.png", tmpFrame);
		findPoints(tmpFrame);
	}

	void processFrame(){
		cout << "Found image state" << endl;
		updateMotionStatistics();
		if (absoluteMotionless() && !controlPointsSet){
			controlPointsSet = true;
			findPoints();
		}

		if (controlPointsSet){
			if (!advancedMatrixSet){
				POINTS_FOUND_FRAME = frameCount;
				advMat = AdvancedMat(originalFrame, controlPoints.points);
				advancedMatrixSet = true;
			}
			else {
				advMat.process(originalFrame);
				immshow("transformedMat", advMat.matrix);
				//imwrite(DIRHTDOCS, advMat.matrix);
			}
			//FRAME_SKIP = 25;
		}

	}


};


#endif