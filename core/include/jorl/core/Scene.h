#pragma once

#include <vector>
#include <iterator>
#include "Drawable.h"

namespace renderlib {

typedef std::vector<Drawable*>::iterator DrawableIterator;

const int SCENE_ALL = 1;

class Scene{

	std::vector<Drawable*> objects;
	std::vector<int> flags;

public:
	Scene();

	void addObject(Drawable *object, int flag=SCENE_ALL);
	bool removeObject(Drawable *object);

	void modifyFlag(Drawable *object);

	void clearScene();

	DrawableIterator objectBegin();
	DrawableIterator objectEnd();
};

}