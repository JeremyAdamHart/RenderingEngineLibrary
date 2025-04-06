#pragma once

#include <vector>
#include <algorithm>

//Permits only one reading or writing operation at a time
class MultiBufferSwitch {
	std::vector<int> timestamp;
	int reading;		//-1 if nothing is being read from
	int writing;		//-1 if nothing is being written to

	int getOldestNotReading();
	int getNewestNotWriting();
public:
	static const int BUFFER_COPIES = 3;

	MultiBufferSwitch();

	int getRead();
	int getWrite();

	void endRead();
	void endWrite();
};