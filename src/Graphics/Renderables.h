#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "Physics/Physics.h"
#include "GameState.h"

struct Vertex2D
{
	glm::vec2 pos;
	glm::vec2 uv;
	uint32_t col;
};

struct Renderable
{
	virtual ~Renderable() = default;
	GLuint texture;
	int layer = 0;
	virtual void AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds) = 0;
	virtual void UpdateFromBody(b2Body* body) = 0;

};

struct TextureQuad : public Renderable
{
	TextureQuad(const glm::vec2& pos, const glm::vec2& halfSz, const glm::vec2& uvStart, const glm::vec2& uvEnd, GLuint tex = 0, uint32_t col = 0xFFFFFFFF, int layer = 0);
	virtual void AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds) override;
	virtual void UpdateFromBody(b2Body* body) override;
	virtual ~TextureQuad() = default;

	glm::vec2 pos;
	glm::vec2 halfSize;
	glm::vec2 uvStart;
	glm::vec2 uvEnd;
	float angle;
	uint32_t col;
};


