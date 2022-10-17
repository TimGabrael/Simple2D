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

struct AnimatedQuad : public Renderable
{
	AnimatedQuad(const glm::vec2& pos, const glm::vec2& halfSz, struct AtlasTexture* tex, uint32_t col = 0xFFFFFFFF, int layer = 0);

	virtual void AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds) override;
	virtual void UpdateFromBody(b2Body* body) override;

	virtual ~AnimatedQuad() = default;

	void AddAnimRange(uint32_t start, uint32_t end);

	struct Range
	{
		uint32_t startIdx;
		uint32_t endIdx;
	};

	std::vector<Range> range;
	glm::vec2 pos;
	glm::vec2 halfSize;
	struct AtlasTexture* atlas;
	float angle;
	uint32_t col;
	uint32_t animIdx = 0;
	uint32_t animStepIdx = 0;
};
