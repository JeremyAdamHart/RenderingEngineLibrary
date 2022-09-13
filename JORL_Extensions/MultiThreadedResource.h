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
	std::vector<T> resource;
	std::shared_mutex locks [N];
	std::atomic_int lastWritten;

	Resource():resource(N), lastWritten(0){}

	Resource(T value) :resource(N, value), lastWritten(0) {
	}

	class Read {
		friend Resource;
		Read(T& data, std::shared_lock<std::shared_mutex> lock, int id)
			:lock(std::move(lock)), data(data), id(id) {}
		std::shared_lock<std::shared_mutex> lock;
	public:
		const T& data;
		const int id;

		const T& operator*() const { return data; }

		const T* operator->() const { return &data; }

		bool hasLock() { return bool(lock); }
	};

	class WriteBase {
	public:
		std::atomic_int* lastWritten;
		const int id;
		WriteBase(std::atomic_int* lastWritten, int id) : lastWritten(lastWritten), id(id) {}
		WriteBase(WriteBase&& w) :lastWritten(w.lastWritten), id(w.id) {
			w.lastWritten = nullptr;
		}

		WriteBase(const WriteBase& w) = delete;
		WriteBase& operator=(const WriteBase& w) = delete;
		WriteBase& operator=(WriteBase&& w) = delete;

		~WriteBase(){
			if (lastWritten != nullptr)
				lastWritten->store(id);
		}
	};

	class Write : public WriteBase{
		friend Resource;
		Write() = delete;
		//Write(T& data, std::unique_lock<std::shared_mutex> lock, int id, std::atomic_int* lastWritten) 
		//	:lock(std::move(lock)), lastWritten(lastWritten), data(data), id(id) { }
		Write(T& data, std::unique_lock<std::shared_mutex> lock, int id, std::atomic_int* lastWritten)
			:WriteBase(lastWritten, id), lock(std::move(lock)), data(data) { }
		std::unique_lock<std::shared_mutex> lock;
		//std::atomic_int* lastWritten;
	public:
		T& data;
		//const int id;

		T& operator*() { return data; }
		const T& operator*() const { return data; }

		T* operator->() {
			return &data;
		}

		const T* operator->() const{
			return &data;
		}

		bool hasLock() { return bool(lock); }

		Write(const Write& w) = delete;
		Write& operator=(const Write& w) = delete;
		Write& operator=(Write&& w) = delete;
		Write(Write&& w) : WriteBase(std::move(w)), lock(std::move(w.lock)), data(w.data){}

		void invalidate() {
			lock.release();
			lastWritten = nullptr;
		}
		/*Write(Write&& w)
			:WriteBase(w.lastWritten, w.id), lock(std::move(w.lock)), data(w.data)
			//:lock(std::move(w.lock)), lastWritten(w.lastWritten), 
			//data(w.data), id(w.id)
		{
			w.lastWritten = nullptr;
		}*/

		/*~Write() {
			if (lastWritten != nullptr && hasLock()) {
				//printf("\tlastWritten = %d\n", id);
				std::cout << "<" << typeid(*this).name() << ">: destructor lastWritten=" << id << std::endl;
				lock.unlock();
				lastWritten->store(id);
			}
		}*/
	};

	Resource::Read getReadSpecific(unsigned int buffer, std::chrono::microseconds pollrate = std::chrono::microseconds(0)) {
		do {
			int index = buffer; 
			{
				Resource::Read readView(resource[index], std::shared_lock<std::shared_mutex>(locks[index], std::try_to_lock), index);
				if (readView.hasLock())
					return readView;
			}
			std::this_thread::sleep_for(pollrate);
		} while (true);
	}

	Resource::Read getRead(std::chrono::milliseconds pollrate = std::chrono::milliseconds(0)) {
		do {                         
			int index = lastWritten;
			Resource::Read readView(resource[index], 
				std::shared_lock<std::shared_mutex>(
					locks[index], std::try_to_lock), index);
			if (readView.hasLock())
				return readView;
			//else
				//printf("Read couldn't get lock %d\n", index);
			std::this_thread::sleep_for(pollrate);
		} while (true);		
	}
	Resource::Write getWrite(std::chrono::milliseconds pollrate = std::chrono::milliseconds(0)) {
		int baseIndex = (lastWritten + 1) % N;
		//std::cout << "<" << typeid(*this).name() << ">::getWrite()" << std::endl;
		do {
			for (int i = 0; i < N - 1; i++) {
				//std::cout << "\tTesting " << (baseIndex + i) % N << std::endl;
				int index = (baseIndex + i) % N;	//std::max(int(N - 1), int(0));
				Resource::Write writeView(resource[index], std::unique_lock(locks[index], std::try_to_lock), index, &lastWritten);
				if (writeView.hasLock())
					return std::move(writeView);
				else
					writeView.invalidate();
			}

			std::this_thread::sleep_for(pollrate);
		} while (true);
	}

	Resource::Write getWriteSpecific(unsigned int buffer, std::chrono::microseconds pollrate = std::chrono::microseconds(0)) {
		do {
			int index = buffer;
			Resource::Write writeView(resource[index], std::unique_lock(locks[index], std::try_to_lock), index, &lastWritten);
			if (writeView.hasLock())
				return std::move(writeView);
			else
				writeView.invalidate();

			std::this_thread::sleep_for(pollrate);
		} while (true);
	}

	class ReadOnly {
		friend Resource;
		Resource* manager;
		ReadOnly(Resource* manager) :manager(manager) {}
	public:
		Resource::Read getRead(std::chrono::milliseconds pollrate = std::chrono::milliseconds(0)) { return manager->getRead(pollrate); }
		Resource::Read getReadSpecific(unsigned int buffer, std::chrono::microseconds pollrate = std::chrono::microseconds(0)) { return manager->getReadSpecific(buffer, pollrate); }
	};

	ReadOnly createReader() { return ReadOnly(this); }
};

/*
template<typename T>
class LLNode{
public:
	std::unique_ptr<LLNode> prev;
};

template<typename T>
class MTQueue {
	std::unique_ptr<LLNode> readerHead;
};

*/