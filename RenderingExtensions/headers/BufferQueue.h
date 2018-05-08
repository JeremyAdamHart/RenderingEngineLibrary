#pragma once

#include "TemplateParameterParsing.h"
#include <vector>
#include <utility>

template<class T>
class BufferQueue {
	std::vector<std::pair<size_t, T>> pending;
public:
	void add(size_t index, T val) { pending.push_back({ index, val }); }

	void dump(T* output) {
		for (int i = 0; i < pending.size(); i++) {
			output[pending[i].first] = pending[i].second;
		}
		pending.clear();
	}
};
