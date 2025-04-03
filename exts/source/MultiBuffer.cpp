#include "MultiBuffer.h"

#include <limits>
#include <algorithm>

MultiBufferSwitch::MultiBufferSwitch(): timestamp(BUFFER_COPIES, 0), reading(-1), writing(-1){}

int MultiBufferSwitch::getOldestNotReading() {
	int oldestTime = std::numeric_limits<int>::max();
	int oldestIndex = 0;
	for (int i = 0; i < timestamp.size(); i++) {
		if (timestamp[i] < oldestTime && i != reading) {
			oldestTime = timestamp[i];
			oldestIndex = i;
		}
	}

	return oldestIndex;
}

int MultiBufferSwitch::getNewestNotWriting() {
	int newestTime = -1;
	int newestIndex = 0;
	for (int i = 0; i < timestamp.size(); i++) {
		if (timestamp[i] > newestTime && i != writing) {
			newestTime = timestamp[i];
			newestIndex = i;
		}
	}

	return newestIndex;
}

int MultiBufferSwitch::getRead() {
	if (reading >= 0)
		return reading;

	int index = getNewestNotWriting();
	reading = index;
	return index;
}

int MultiBufferSwitch::getWrite() {
	if (writing >= 0)
		writing;

	int index = getOldestNotReading();
	writing = index;
	return index;
}

void MultiBufferSwitch::endRead() {
	reading = -1;
}

void MultiBufferSwitch::endWrite() {
	int maxTimestamp = *std::max_element(timestamp.begin(), timestamp.end());
	timestamp[writing] = maxTimestamp + 1;
	writing = -1;
}