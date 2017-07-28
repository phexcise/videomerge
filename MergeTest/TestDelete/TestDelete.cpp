// TestDelete.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

static int n = 0;

#include "API.h"

int main()
{
	MEDIAIN mediaIn;
	VECMEDIAIN vecMediaIn;
	mediaIn.strFile = "E:\\1.mp4";
	mediaIn.time_begin = 10;
	mediaIn.time_end = 20;
	vecMediaIn.push_back(mediaIn);

	mediaIn.strFile = "E:\\2.mp4";
	mediaIn.time_begin = 30;
	mediaIn.time_end = 40;
	vecMediaIn.push_back(mediaIn);

	mediaIn.strFile = "E:\\3.mp4";
	mediaIn.time_begin = 50;
	mediaIn.time_end = 60;
	vecMediaIn.push_back(mediaIn);
	mergeVideo(vecMediaIn, "output.mp4");
	return 0;
}