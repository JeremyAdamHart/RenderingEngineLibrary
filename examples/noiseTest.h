#pragma once

#include "AdaptiveNoise.h"

class Counter {
public:
	int val;
	Counter();
};

void runAdaptiveNoiseTests();

void validateTopFaces(adaptive::VariableSizeGrid<adaptive::TopFace<adaptive::Noise>>& grid);
