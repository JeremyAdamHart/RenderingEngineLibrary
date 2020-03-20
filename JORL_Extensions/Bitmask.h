#pragma once
#include <vector>

struct Bitmask {
	std::vector<uint32_t> bits;
	Bitmask();

	void set(int bitNumber);
	void clear(int bitNumber);
	void toggle(int bitNumber);
	bool test(int bitNumber) const;

	uint32_t getWord(int wordNumber) const;
	const uint32_t* getWords() const;
};