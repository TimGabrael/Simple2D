#pragma once


struct SceneObject
{
	struct Entity* entity;
	glm::mat4 transform;
	uint32_t flags;
};
