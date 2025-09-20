#pragma once

#include <list>
#include <string>
#include <memory>

#include "mathlib.h"
#include "model.h"
#include "entity.h"


class World {
protected:

    std::shared_ptr<Map> map;

    std::list<Entity*> entities{};

    double worldTime = 0.0f;
    double TotalTime = 0.0f;

public:

};