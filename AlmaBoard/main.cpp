#include "tools.h"
#include "Board.h"

int main(int argc, char *argv[]){
	VideoCapture cap("C:/Users/Pernekhan Utemuratov/Desktop/Videos/20150604_172836.mp4");

	if (!cap.isOpened()){
		cout << "Can'at open the file" << endl;
		return -1;
	}

	Board board;
	Mat frame;
	for (frameCount = 0;; frameCount++){
		if (!cap.read(frame)){
			break;
		}
		if (frameCount%FRAME_SKIP){
			continue;
		}
		resize(frame, frame, Size(960, 540));
		if (frameCount == 0) board = Board(frame);
		immshow("window", frame);
		Mat nframe;
		resize(frame, nframe, Size(480, 270));
		imwrite(DIRHTDOCS, nframe);
		if (isDebugMode) 
			cvvWaitKey();
		board.process(frame);
		cout << "OK_PROCESSED"<<endl;
	}

	return 0;
}
