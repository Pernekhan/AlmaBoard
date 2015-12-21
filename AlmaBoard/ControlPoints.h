#ifndef CONTROLPOINTS_H
#define CONTROLPOINTS_H

#include "tools.h"

struct ControlPoints{
	vector < Point > points;
	int RECTANGLE_PERCENTAGE = 30; // the percent of area of points
	ControlPoints(){}
	ControlPoints(Point a, Point b, Point c, Point d){
		points.pb(a); points.pb(b); points.pb(d); points.pb(d);
	}
	ControlPoints(vector < Point > points){
		this->points = points;
	}

	Point getMassCenter(vector < Point > &points){
		int sumx = 0, sumy = 0;
		for (int i = 0; i < points.size(); i++){
			sumx += points[i].x;
			sumy += points[i].y;
		}
		int len = points.size();
		return Point(sumx / len, sumy / len);
	}

	bool canBeValidRectangleCheckSides(int lena, int lenb){
		return lena * 3 > lenb && lenb * 3 > lena;
	}

	bool canBeValidRectangle(vector < Point > &points){
		int minx = inf, miny = inf, maxx = -inf, maxy = -inf;
		for (int i = 0; i < points.size(); i++){
			minx = min(minx, points[i].x);
			miny = min(miny, points[i].y);
			maxx = max(maxx, points[i].x);
			maxy = max(maxy, points[i].y);
		}
		int len = points.size();
		int lena = abs(maxx - minx);
		int lenb = abs(maxy - miny);
		return lena*lenb*RECTANGLE_PERCENTAGE / 100 <= len && canBeValidRectangleCheckSides(lena, lenb);
	}

};



#endif