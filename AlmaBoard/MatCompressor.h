#ifndef MATCOMPRESSOR_H
#define MATCOMPRESSOR_H

#include "tools.h"

struct MatCompressor{
	vector < pair < int, int > > pixels;
	vector < pair < int, int > > prevPixels;
	int N, M;
	vector < pair < pii, pii > > prevRects;

	MatCompressor(){}

	void process(Mat arr){
		N = arr.rows; M = arr.cols;
		prevPixels.clear();
		for (int i = 0; i < pixels.size(); i++) prevPixels.push_back(pixels[i]);
		pixels.clear();
		for (int i = 0; i < arr.rows; i++){
			for (int j = 0; j < arr.cols; j++){
				int val = static_cast<int>(arr.at<uchar>(i, j));
				if (val == 0)
					pixels.push_back(make_pair(i, j));
			}
		}

	}

	void static sendWidthHeight(int width, int height){
		ofstream out(DIRHTDOCS_TXT_WIDTH_HEIGHT);
		out << width << " " << height << endl;
		out.close();
	}

	vector < pair < int, int > > getWhitePixels(){
		vector < pii > res;
		for (int i = 0; i < prevPixels.size(); i++){
			if (binary_search(pixels.begin(), pixels.end(), prevPixels[i]))
				res.push_back(prevPixels[i]);
		}
		return res;
	}

	vector < pair < int, int > > getBlackPixels(){
		vector < pair < int, int > > res;
		for (int i = 0; i < pixels.size(); i++){
			if (binary_search(prevPixels.begin(), prevPixels.end(), pixels[i]))
				res.push_back(pixels[i]);
		}
		return res;
	}

	vector < pair < pii, pii > > getLineSegments(vector <pii> pixels){
		set < pii > S;
		for (int i = 0; i < pixels.size(); i++){
			int x = pixels[i].first;
			int y = pixels[i].second;

		}
	}

	void sendDataAxisCompressed(vector < pair < pii, pii > > res){
		ofstream out(DIRHTDOCS_TXT);
		string req = "";
		vector < string > v;
		int t = frameCount - POINTS_FOUND_FRAME;
		for (int i = 0; i < res.size(); i++){
			int x = res[i].first.first;
			int y = res[i].first.second;
			int width = res[i].second.first;
			int height = res[i].second.second;
			//req += toString(x) + "_" + toString(y) + "_" + toString(width) + "_" + toString(height) + "_" + toString(t) + "!";
			v.push_back(toString(x) + "_" + toString(y) + "_" + toString(width) + "_" + toString(height) + "_" + toString(t));
			out << x << " " << y << " " << width << " " << height << endl;
		}
		out.close();

		if (sz(res) < 100) return;

		if (sz(res) * 100 > sz(prevRects) * 110 || sz(res) * 100 < sz(prevRects) * 90){
			//tempSendingData(v);
			prevRects = res;
		}



	}

	void tempSendingData(vector <string> v){
		for (int i = 0; i < v.size(); i++){
			sendDataToServer(v[i]);
		}
	}

	vector < pair < pii, pii > > getDataAxisCompressed(){
		vector <bool> g(N*M, 0);
		vector < pii > v = pixels;
		for (int i = 0; i < v.size(); i++){
			g[v[i].first*M + v[i].second] = true;
		}

		vector < pair < pii, pii > > res;
		for (int j = 0; j < v.size(); j++){
			int pos = v[j].first*M + v[j].second;
			if (!g[pos]) continue;
			int col = v[j].second;
			int row = v[j].first;
			while (col < M && g[row*M + col]) col++;
			for (; row < N; row++){
				bool ok = true;
				for (int k = v[j].second; k < col && ok; k++){
					if (!g[row*M + k]) ok = false;
				}
				if (!ok)break;
			}
			int width = col - v[j].second;
			int height = row - v[j].first;

			for (int i = v[j].first; i < row; i++)
				for (int k = v[j].second; k < col; k++)
					g[i*M + k] = false;
			res.push_back(mp(v[j], mp(width, height)));
		}
		return res;
	}

	void sendSimpleData(){
		ofstream out(DIRHTDOCS_TXT2);
		for (int i = 0; i < pixels.size(); i++){
			out << pixels[i].first << " " << pixels[i].second << endl;
		}
		out.close();
	}

	string static toString(int x){
		if (x == 0) return "0";
		string res = "";
		while (x){
			res = char('0' + x % 10) + res;
			x /= 10;
		}
		return res;
	}
	int cnt = 0;
	void sendDataToServer(string &req){
		/*
		cout << req << endl;
		WinHttpClient client(L"http://localhost/almaboard/receive_data.php");
		// Set post data.
		string data = "session_id=2&request=" + req;
		client.SetAdditionalDataToSend((BYTE *)data.c_str(), data.size());

		// Set request headers.
		wchar_t szSize[50] = L"";
		swprintf_s(szSize, L"%d", data.size());
		wstring headers = L"Content-Length: ";
		headers += szSize;
		headers += L"\r\nContent-Type: application/x-www-form-urlencoded\r\n";
		client.SetAdditionalRequestHeaders(headers);

		// Send HTTP post request.
		client.SendHttpRequest(L"POST");
		cout << "DONE" << endl;
		*/
	}

	void sendData(){
		//Sleep(1000);
		sendDataAxisCompressed(getDataAxisCompressed()); return;
		sendSimpleData(); return;
		vector < pair < int, int > > whitePixels = getWhitePixels();
		vector < pair < int, int > > blackPixels = getBlackPixels();
		ofstream out(DIRHTDOCS_TXT, ios::app);
		string request = "";
		for (int i = 0; i < whitePixels.size(); i++){
			//request += toString(whitePixels[i].first) + "_" + toString(whitePixels[i].second) + "_" + "0_" + toString(clock()) + "!";
			out << toString(whitePixels[i].first) + "_" + toString(whitePixels[i].second) + "_" + "0_" + toString(clock()) << endl;
		}

		for (int i = 0; i < blackPixels.size(); i++){
			//request += toString(blackPixels[i].first) + "_" + toString(blackPixels[i].second) + "_" + "1_" + toString(clock()) + "!";
			out << toString(blackPixels[i].first) + "_" + toString(blackPixels[i].second) + "_" + "1_" + toString(clock()) << endl;
		}
		out.close();
		//sendDataToServer(request);
	}

};


#endif