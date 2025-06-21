#include "../include/Entity.hpp"

// Static member definition
std::unique_ptr<ComponentManager> Entity::s_componentManager = nullptr;

size_t Entity::id() const { return m_id; };
bool Entity::isActive() const { return m_active; };
void Entity::destroy() { m_active = false; };
const EntityTag &Entity::tag() const { return m_tag; };
