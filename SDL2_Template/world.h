#pragma once

#include <list>
#include <string>


#include "mathlib.h"
#include "model.h"
#include "entity.h"


class IWorld {
protected:

    BSPMap map;

    std::list<Entity*> entities{};

    double worldTime = 0.0f;

public:

    IWorld();
    virtual ~IWorld() = 0 { }

    virtual bool init() = 0;

    virtual void update(float deltaTime) = 0;
    
    bool loadBSPMap(std::string path) { return map.loadMap(path); }
    
    const std::list<Entity*>& getEntities() const { return entities; }
    const BSPMap& getMap() const { return map; }

    virtual void addEntity(Entity* entity) { entities.push_back(entity); }
};