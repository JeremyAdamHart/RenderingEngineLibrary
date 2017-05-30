#include "Scene.h"

using namespace std;

Scene::Scene(){}

void Scene::clearScene(){ objects.clear(); }

DrawableIterator Scene::objectBegin(){ return objects.begin(); }
DrawableIterator Scene::objectEnd(){ return objects.end(); }