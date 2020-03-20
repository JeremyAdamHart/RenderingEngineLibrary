#include "Bitmask.h"

Bitmask::Bitmask(){
	bits.push_back(0);
}

void Bitmask::set(int bitNumber)
{
	while (bits.size() * 32 <= bitNumber)
		bits.push_back(0);

	bits[bitNumber / 32] |= 1 << (bitNumber % 32);
}

void Bitmask::clear(int bitNumber)
{
	while (bits.size() * 32 <= bitNumber)
		bits.push_back(0);

	bits[bitNumber / 32] &= !(1 << (bitNumber % 32));
}

void Bitmask::toggle(int bitNumber)
{
	while (bits.size() * 32 <= bitNumber)
		bits.push_back(0);

	bits[bitNumber / 32] ^= 1 << (bitNumber % 32);

}

bool Bitmask::test(int bitNumber) const {
	if (bitNumber / 32 < bits.size())
		return bits[bitNumber / 32] & (1 << (bitNumber % 32));
	else
		return false;
}

uint32_t Bitmask::getWord(int wordNumber) const
{
	return (wordNumber < bits.size()) ? bits[wordNumber] : 0;
}

const uint32_t* Bitmask::getWords() const
{
	return bits.data();
}
