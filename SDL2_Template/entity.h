#pragma once
#include <vector>

constexpr double MAX_RANGE = 8192;

class Entity {
private:
    vec3 origin{};
    vec3 angles{};           // Pitch, Yaw, Roll

    // Bounding box
    vec3 mins =  vec3{ MAX_RANGE, MAX_RANGE, MAX_RANGE };
    vec3 maxs = -vec3{ MAX_RANGE, MAX_RANGE, MAX_RANGE };

    int entindex;
    int modelIndex;

    // Флаги состояния
    bool isStatic  : 1;
    bool isEnabled : 1;
    bool isVisible : 1;

public:

    Entity(
        const vec3& position,
        const vec3& angles,
        const vec3& mins,
        const vec3& maxs,
        const std::string& classname,
        const bool& isStatic,
        const bool& isEnabled,
        const bool& isVisible
    );

    Entity() = default;
    virtual ~Entity() = default;

    virtual void spawn() = 0;
    virtual void think() = 0;
    virtual void update(float deltaTime) = 0;

    virtual void onDamage(float damage, Entity* attacker) = 0;
    virtual void onUse(Entity* activator) = 0;
    virtual void onTouch(Entity* other) = 0;


    // Геттеры
    const vec3& getOrigin() const { return origin; }
    const vec3& getAngles() const { return angles; }
    const vec3& getMins() const { return mins; }
    const vec3& getMaxs() const { return maxs; }
    virtual std::string getClassname() const = 0;

    int getEntIndex() const { return entindex; }
    virtual int getModelIndex() = 0;

    bool getIsStatic() const { return isStatic; }

    // Сеттеры
    void setOrigin(const vec3& newOrigin);
    void setAngles(const vec3& newAngles);

    void setEnabled(bool enabled);
    void setVisible(bool visible);
};

class BrushModelEntity : public Entity {

};

class SpriteModelEntity : public Entity {

};

class AliasModelEntity : public Entity {

};