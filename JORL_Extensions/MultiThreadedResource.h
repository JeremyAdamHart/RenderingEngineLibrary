#pragma once
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <iterator>
#include <algorithm>
#include <functional>
#include <assert.h>

//template<typename T> class Resource;

template<typename T, size_t N>
class MultiBufferStager {
	size_t startIndices[N];
	bool valid[N];
	std::vector<T> data;
	size_t lastElement;
public:
	MultiBufferStager(size_t set_capacity) :lastElement(0) {
		data.resize(set_capacity);
		std::for_each(std::begin(startIndices), std::end(startIndices), 
			[](size_t& v) { v = 0; }
		);
		std::for_each(std::begin(valid), std::end(valid),
			[](bool& v) { v = true; }
		);
	}

	void clear() {
		data.clear();
		std::for_each(std::begin(startIndices), std::end(startIndices),
			[](size_t& v) { v = 0; }
		);
		std::for_each(std::begin(valid), std::end(valid),
			[](bool& v) { v = true; }
		);
		lastElement = 0;

		data.resize(set_capacity*N);
	}

	size_t capacity() const { return data.size(); }

	size_t maxStart() const {
		return *std::max_element(std::begin(startIndices), std::end(startIndices));
	}

	size_t minStart() const {
		return *std::min_element(std::begin(startIndices), std::end(startIndices));
	}

	//Invalid if queued data has been stomped on
	bool isValid(unsigned int bufferIndex) const {
		return valid[bufferIndex];
	}

	template<typename Iter>
	void add(Iter begin, Iter end) {
		auto destBegin = data.begin() + lastElement % capacity();
		size_t startRange = std::min(size_t(end - begin), size_t(data.end() - destBegin));
		size_t endRange = size_t(end - begin) - startRange;
		std::copy(begin, begin + startRange, destBegin);
		std::copy(begin + startRange, end, data.begin());

		lastElement += size_t(end - begin);

		for (int i = 0; i < N; i++) {
			if (startIndices[i] + capacity() < lastElement)
				valid[i] = false;
			//assert(valid[i], "Data has been overwritten");
		}
	}

	template<typename... Args>
	using FuncType = void(*)(Args...);

	template<typename... Args>
	void processStagedElements(unsigned int bufferIndex, FuncType<const T&, Args...> f, Args... args) {
		auto destBegin = data.begin() + startIndices[bufferIndex]%capacity();
		size_t startRange = std::min(size_t(data.end() - destBegin), lastElement - startIndices[bufferIndex]);
		
		size_t endRange = (lastElement - startIndices[bufferIndex]) - startRange;

		for (auto it = destBegin; it != destBegin + startRange; it++)
			f(*it, args...);
		for (auto it = data.begin(); it != data.begin() + endRange; it++)
			f(*it, args...);

		startIndices[bufferIndex] = lastElement;
	}
};

template<typename T, size_t N>
class Resource{
public:
	T resource[N];
	std::shared_mutex locks [N];
	std::atomic_int lastWritten;

	class Read {
		friend Resource;
		Read(T* data, std::shared_lock<std::shared_mutex> lock, int id): lock(lock), data(data), id(id){}
		std::shared_lock<std::shared_mutex> lock;
	public:
		const T& data;
		const int id;

		const T& operator*() {
			return data;
		}

		bool hasLock() { return lock; }
	};

	class Write {
		friend Resource;
		Write(T& data, std::unique_lock<std::shared_mutex> lock, int id, std::atomic_int* lastWritten) :lock(lock), lastWritten(lastWritten), data(data), id(id) { }
		std::unique_lock<std::shared_mutex> lock;
		std::atomic_int* lastWritten;
	public:
		T& data;
		const int id;

		T& operator*() { return data; }
		const T& operator*() { return data; }

		bool hasLock() { return lock; }

		~Write() {
			lastWritten->store(id);
		}
	};

	Resource::Read getRead(std::chrono::milliseconds pollrate = 0) {
		do {
			int index = lastWritten;
			Resource::Read readView(resource[index], std::shared_lock(locks[index]), index, &lastWritten);
			if (readView.hasLock())
				return readView;
			std::this_thread::sleep_for(pollrate);
		} while (true);		
	}
	Resource::Write getWrite(std::chrono::milliseconds pollrate = 0) {
		int baseIndex = (lastWritten + 1) % N;
		do {
			for (int i = 0; i < N - 1; i++) {
				int index = baseIndex + i % max(N - 1, 0);
				Resource::Write writeView(resource[index], std::unique_lock(locks[index]), index, &lastWritten);
				if (writeView.hasLock())
					return writeView;
			}

			std::this_thread::sleep_for(pollrate);
		} while (true);
	}

	class ReadOnly {
		friend Resource;
		Resource* manager;
		ReadOnly(Resource* manager) :manager(manager) {}
	public:
		Resource::Read getRead(std::chrono::milliseconds pollrate = 0) { return manager->getRead(time); }
	};

	ReadOnly createReader() { return ReadOnly(this); }
};
