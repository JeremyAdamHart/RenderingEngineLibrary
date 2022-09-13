#pragma once

#include <GLGeometry.h>
#include <glm/glm.hpp>
#include <vector>
#include <tuple>
#include <glSupport.h>
#include "VertexBufferTemplates.h"
#include "TemplateParameterParsing.h"
#include "MultiBuffer.h"
#include "BufferQueue.h"

#include<MultiThreadedResource.h>
#include <VertexAttribLayout.h>

namespace renderlib {

using namespace glm;

template<class... Ts>
class PinnedGeometry : public GLGeometryContainer {
public:
	class AttributePointers {
	friend PinnedGeometry;
	private:
		std::vector<void*> pointers;
		size_t offset;

	public:
		AttributePointers(std::vector<void*> pointers = {}, size_t offset=0) 
			: pointers(pointers), offset(offset) {}
		template<typename T>
		typename T::Type* get() { return static_cast<typename T::Type*>(pointers[indexOf<T, Ts...>()])+offset; }
		template<typename T>
		const typename T::Type* get() const { return static_cast<typename T::Type*>(pointers[indexOf<T, Ts...>()])+offset; }
	};

protected:

	struct SyncPair {
		GLsync sync;
		typename Resource<AttributePointers, 3>::Read ptrs;
		SyncPair(GLsync sync, typename Resource<AttributePointers, 3>::Read&& ptrs) :
			sync(sync), ptrs(std::move(ptrs)) {}
		void update(GLsync newSync) {
			glDeleteSync(sync);
			sync = newSync;
		}
		bool triggered() {
			GLenum result = glClientWaitSync(sync, 0, 0);
			return result == GL_CONDITION_SATISFIED || result == GL_ALREADY_SIGNALED;
		}
	};

	VertexBindingMapping vaoMap;
	GLenum mode;
	std::vector<GLBuffer> vbo;
	size_t bufferSize;
	size_t indexSize;

	//Synchronization
	std::map<int, SyncPair> drawFences;
public:
	Resource<AttributePointers, 3> pinnedData;
	PinnedGeometry(size_t size, const unsigned int* indices, size_t indexSize, GLenum mode = GL_TRIANGLES)
		: bufferSize(size), indexSize(indexSize), mode(mode)
	{
		for (int i = 0; i < sizeof...(Ts); i++) {
			vbo.push_back(createBufferID());
			glBindBuffer(GL_ARRAY_BUFFER, vbo.back());
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vbo.push_back(createBufferID());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indexSize, indices, GL_STATIC_DRAW);
		
		std::vector<void*> voidPointers(sizeof...(Ts));
		allocateBufferStorage<Ts...>(vbo.data(), bufferSize*3, voidPointers.data());

		*pinnedData.getWriteSpecific(0, std::chrono::milliseconds(1)) =
			AttributePointers(voidPointers, 0);
		*pinnedData.getWriteSpecific(1, std::chrono::milliseconds(1)) =
			AttributePointers(voidPointers, bufferSize);
		*pinnedData.getWriteSpecific(2, std::chrono::milliseconds(1)) =
			AttributePointers(voidPointers, 2*bufferSize);
	}

	PinnedGeometry(size_t size, GLenum mode = GL_TRIANGLES)
		: bufferSize(size), indexSize(0), mode(mode)
	{
		for (int i = 0; i < sizeof...(Ts); i++) {
			vbo.push_back(createBufferID());
			glBindBuffer(GL_ARRAY_BUFFER, vbo.back());
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vbo.push_back(createBufferID());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());

		std::vector<void*> voidPointers(sizeof...(Ts));
		allocateBufferStorage<Ts...>(vbo.data(), bufferSize * 3, voidPointers.data());

		*pinnedData.getWriteSpecific(0, std::chrono::milliseconds(1)) =
			AttributePointers(voidPointers, 0);
		*pinnedData.getWriteSpecific(1, std::chrono::milliseconds(1)) =
			AttributePointers(voidPointers, bufferSize);
		*pinnedData.getWriteSpecific(2, std::chrono::milliseconds(1)) =
			AttributePointers(voidPointers, 2 * bufferSize);
	}

	template<typename A>
	void loadBuffer(typename const A::Type* data) {
		if constexpr (!attrib::usingPinned<A>()) {
			glBindBuffer(GL_ARRAY_BUFFER, vbo[indexOf<A, Ts...>()]);
			size_t bufferSizeBytes = bufferSize * sizeof(typename A::Type);
			glBufferData(GL_ARRAY_BUFFER, bufferSizeBytes*3, nullptr, GL_DYNAMIC_DRAW);
			for (int i = 0; i < 3; i++)
				glBufferSubData(GL_ARRAY_BUFFER, i*bufferSizeBytes, bufferSizeBytes, data);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		else {
			for (int i = 0; i < 3; i++) {
				auto buffer = pinnedData.getWriteSpecific(i, std::chrono::microseconds(100));
				for (int d = 0; d < bufferSize; d++) {
					buffer->get<A>()[d] = data[d];
				}
			}
		}
	}
	
	void loadBuffers(typename const Ts::Type*... data) {
		loadBuffers_rec<PinnedGeometry<Ts...>, Ts...>(this, data...);
	}

	void loadIndices(const unsigned int* indices, size_t newIndexSize) {
		indexSize = newIndexSize;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	virtual void drawGeometry(GLProgram program) override {
		//Release any other locks held for rendering if finished
		//printf("Start draw------\n");
		for (auto& fence = drawFences.begin(); fence != drawFences.end();) {
			if (fence->second.triggered()) {
				//printf("\t\tunlocked %d\n", fence->first);
				fence = drawFences.erase(fence);
			}
			else
				fence++;
		}

		vaoMap.requestVAO<Ts...>(program, &vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());

		int drawIndex = pinnedData.lastWritten;
		if (drawFences.find(drawIndex) == drawFences.end())
			drawFences.emplace(
				std::make_pair(
					std::move(drawIndex), 
					std::move(SyncPair(
						0, 
						std::move(pinnedData.getReadSpecific(drawIndex)
						))
					)
				)
			);

		if (drawFences.find(drawIndex)->second.ptrs.hasLock()){
			//printf("\t[Draw]Drawing index %d\n", drawIndex);
			glDrawElementsBaseVertex(mode, indexSize, GL_UNSIGNED_INT, 0,  bufferSize*drawIndex);
			drawFences.find(drawIndex)->second.update(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
		}
		else {
			printf("////////FAILED TO OBTAIN LOCK %d/////////\n", drawIndex);
			for (auto& fence : drawFences) {
				printf("[Draw]draw fence %d\n", fence.first);
			}
		}

		glBindVertexArray(0);
	}
};

/**
* STREAM GEOMETRY CLASS
**/
template<class... Ts>
class StreamGeometry : public GLGeometryContainer {
public:
	MultiBufferSwitch buffManager;
protected:
	GLVAO vao;
	size_t bufferSize;
	size_t elementNum;
	GLenum mode;
	std::vector<GLBuffer> vbo;
	std::vector<void*> voidVboPointers; 
	GLsync drawSync;
	GLBuffer vboElement;
	std::vector<wrap_tuple<BufferQueue, Ts...>> queue;
	std::vector<char> streamed;

	std::vector<unsigned int> vboCopies;

	bool initVAO() {
		glBindVertexArray(vao);
		initVertexBuffers<Ts...>(&vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);

		glBindVertexArray(0);

		return checkGLErrors("StreamGeometry::initVAO()");
	}

	void createBufferStorage() {
		voidVboPointers.resize(vbo.size());
		allocateBufferStorage<Ts...>(vbo.data(), streamed.data(), bufferSize*MultiBufferSwitch::BUFFER_COPIES, voidVboPointers.data());
	}

public:
	StreamGeometry(GLenum mode = GL_TRIANGLES) :
		bufferSize(0), elementNum(0), mode(mode), queue(MultiBufferSwitch::BUFFER_COPIES),
		streamed({}), vao(createVAOID()), vboElement(createBufferID())
	{
		initVAO();
		streamed.resize(vbo.size(), false);
		createBufferStorage();
	}
	StreamGeometry(size_t bufferSize, std::vector<char> streamed = {}, size_t elementNum = 0, GLenum mode = GL_TRIANGLES) :
		bufferSize(bufferSize), elementNum(elementNum), mode(mode), vao(createVAOID()), vboElement(createBufferID()),
		queue(MultiBufferSwitch::BUFFER_COPIES), streamed(streamed)
	{
		initVAO();
		streamed.resize(vbo.size(), true);
		createBufferStorage();
	}

	template<size_t N>
	void modify(size_t index, const nth_type<N, Ts...>& val) {
		if (index > bufferSize)
			printf("Bad\n");
		for (int i = 0; i < queue.size(); i++) {
			get<N, BufferQueue, Ts...>(queue[i])->add(index, val);
		}
	}

	template<size_t N>
	void dump() {
		size_t bufferNum = buffManager.getWrite();
		nth_type<N, Ts...>* data = vboPointer<N>();
		get<N, BufferQueue, Ts...>(queue[bufferNum])->dump(data);
	}

	size_t getBufferSize() const { return bufferSize; }

	template<size_t N>
	void unmap() {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	virtual GLuint getVboID(int num) const { return (num < vbo.size()) ? vbo[num] : 0; }

	virtual void drawGeometry() {
		static bool firstDraw = true;
		
/*		if (!firstDraw) {
			GLenum syncStatus = GL_UNSIGNALED;
			int numMilliseconds = 0;
			while (syncStatus != GL_ALREADY_SIGNALED && syncStatus != GL_CONDITION_SATISFIED) {
				syncStatus = glClientWaitSync(drawSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1000);
				numMilliseconds++;
			}
		}*/
		buffManager.endRead();	//Free buffer to be written to
		int bufferNum = buffManager.getRead();

		glBindVertexArray(vao);
		glDrawElementsBaseVertex(mode, elementNum, GL_UNSIGNED_INT, 0, bufferSize*bufferNum);
		//Syncronize
//		glDeleteSync(drawSync);
	//	drawSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		firstDraw = false;
	}

	virtual void drawGeometry(GLProgram programID) {
		static bool firstDraw = true;

		/*		if (!firstDraw) {
					GLenum syncStatus = GL_UNSIGNALED;
					int numMilliseconds = 0;
					while (syncStatus != GL_ALREADY_SIGNALED && syncStatus != GL_CONDITION_SATISFIED) {
						syncStatus = glClientWaitSync(drawSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1000);
						numMilliseconds++;
					}
				}*/
		buffManager.endRead();	//Free buffer to be written to
		int bufferNum = buffManager.getRead();

		glBindVertexArray(vao);
		glDrawElementsBaseVertex(mode, elementNum, GL_UNSIGNED_INT, 0, bufferSize*bufferNum);
		//Syncronize
//		glDeleteSync(drawSync);
	//	drawSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		firstDraw = false;
	}

	template<size_t N> 
	nth_type<N, Ts...>* vboPointer() { 
		size_t offset = bufferSize*buffManager.getWrite();
		return reinterpret_cast<nth_type<N, Ts...>*>((char*)voidVboPointers[N]) + offset; 
	}

	template<size_t N>
	void loadBuffer(const nth_type<N, Ts...>* data) {
		if (streamed[N]) {
			size_t byteSize = bufferSize * sizeof(nth_type<N, Ts...>);
			for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
				std::memcpy((char*)voidVboPointers[N]+i*byteSize, data, byteSize);
			}
		}
		else {
			glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
			///*
			std::vector<nth_type<N, Ts...>> buffer;

			for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
				for (int j = 0; j < bufferSize; j++) {
					buffer.push_back(data[j]);
				}
			}
			glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(nth_type<N, Ts...>), buffer.data(), GL_DYNAMIC_DRAW);
			//*/
			//glBufferSubData(GL_ARRAY_BUFFER, bufferSize * sizeof(nth_type<N, Ts...>)*buffManager.getWrite(), bufferSize * sizeof(nth_type<N, Ts...>), data);
			//glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(nth_type<N, Ts...>), data, GL_DYNAMIC_DRAW);
		}

		checkGLErrors("StreamGeometry::loadBuffer()");
	}
	template<size_t N>
	void loadHalfBuffer(nth_type<N, Ts...>* data) {
		if (streamed[N]) {
			size_t byteSize = bufferSize/2 * sizeof(nth_type<N, Ts...>);
			for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
				std::memcpy((char*)voidVboPointers[N] + i * byteSize, data, byteSize);
			}
		}
		else {
			glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
			/*std::vector<nth_type<N, Ts...>> buffer;

			for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
				for (int j = 0; j < bufferSize/2; j++) {
					buffer.push_back(data[j]);
				}
			}
			glBufferData(GL_ARRAY_BUFFER, buffer.size()/2 * sizeof(nth_type<N, Ts...>), buffer.data(), GL_DYNAMIC_DRAW);
			*/
			glBufferData(GL_ARRAY_BUFFER, bufferSize*sizeof(nth_type<N, Ts...>), data, GL_DYNAMIC_DRAW)
		}

		checkGLErrors("StreamGeometry::loadBuffer()");
	}
/*
	template<>
	void loadBuffer<0>(glm::vec3* data) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		std::vector<glm::vec3> buffer;

		for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
			for (int j = 0; j < bufferSize; j++) {
				buffer.push_back(data[j]);
			}
		}
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(glm::vec3), buffer.data(), GL_DYNAMIC_DRAW);
	}

	template<>
	void loadBuffer<1>(glm::vec3* data) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		std::vector<glm::vec3> buffer;

		for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
			for (int j = 0; j < bufferSize; j++) {
				buffer.push_back(data[j]);
			}
		}
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(glm::vec3), buffer.data(), GL_DYNAMIC_DRAW);
	}
	*/
/*	//DON'T USE
	template<unsigned int N, class D> 
	void loadBuffer(D* data, GLenum usage) {
//		BufferCompatability<N - 1, D, Ts...> compat;	
		VERIFY_TEMPLATE_TYPE_AT_INDEX(N, D, Ts...);		//Checks to see if Nth type in Ts is D

		glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(D)*bufferSize, data, usage);
	}

	//DON'T USE
	void loadGeometry(size_t newBufferSize, GLenum usage, Ts*... args) {
		bufferSize = newBufferSize;
		loadVBOs(0, &vbo, bufferSize, usage, args...);
	}
*/

	void loadElementArray(size_t newElementNum, GLenum usage, unsigned int* elements) {
		elementNum = newElementNum;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementNum * sizeof(unsigned int), elements, usage);
	}
};

}