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

enum RENDERABLE_FLAGS
{
	INACTIVE = 1,
};

struct Renderable
{
	Renderable();
	virtual ~Renderable();
	virtual void AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds) = 0;
	virtual int GetLayer() const = 0;
	virtual GLuint GetTexture() const = 0;
	virtual uint32_t GetFlags() const = 0;
};

struct TextureQuad : public Renderable
{
	TextureQuad(const glm::vec2& pos, const glm::vec2& halfSz, const glm::vec2& uvStart, const glm::vec2& uvEnd, GLuint tex = 0, uint32_t col = 0xFFFFFFFF, int layer = 0);
	virtual ~TextureQuad() = default;
	virtual void AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds) override;
	virtual int GetLayer() const;
	virtual GLuint GetTexture() const;
	virtual uint32_t GetFlags() const;

	void UpdateFromBody(b2Body* body);

	glm::vec2 pos;
	glm::vec2 halfSize;
	glm::vec2 uvStart;
	glm::vec2 uvEnd;
	float angle;
	uint32_t col;
	int layer = 0;
	GLuint texture;
	uint32_t flags;
};

struct AnimatedQuad : public Renderable
{
	AnimatedQuad(const glm::vec2& pos, const glm::vec2& halfSz, struct AtlasTexture* tex, uint32_t col = 0xFFFFFFFF, int layer = 0);
	virtual ~AnimatedQuad() = default;

	virtual void AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds) override;
	virtual int GetLayer() const;
	virtual GLuint GetTexture() const;
	virtual uint32_t GetFlags() const;

	void UpdateFromBody(b2Body* body);
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
	int layer = 0;
	GLuint texture;
	uint32_t animIdx = 0;
	uint32_t animStepIdx = 0; 
	uint32_t flags;
};


struct Particle
{
	glm::vec2 pos;
	glm::vec2 vel;
	glm::vec2 sizeBegin; // these are half sizes
	glm::vec2 sizeEnd;	 // these are half sizes
	uint32_t colBegin;
	uint32_t colEnd;
	uint32_t textureIdx;
	float rotationBegin;
	float rotationEnd;
	float lifeTime = 1.0f;
	float lifeRemaining = 0.0f;
	bool active = false;
	void AddToVertices(const AtlasTexture& atlas, std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds) const;
};

struct ParticlesBase : public Renderable
{
	ParticlesBase(struct AtlasTexture* atlas, uint32_t numInPool, int layer = INT_MAX);
	~ParticlesBase() = default;

	virtual void AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds) override;

	void Update(float dt);


	void AddParticle(const glm::vec2& pos, const glm::vec2& vel, const glm::vec2& sizeBegin, const glm::vec2& sizeEnd, uint32_t colBegin, uint32_t colEnd, uint32_t textureIdx, float rotationBegin, float rotationEnd, float lifeTime);


	void AddParticleToVertices(Particle& p, std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds);

	virtual int GetLayer() const;
	virtual GLuint GetTexture() const;
	virtual uint32_t GetFlags() const;

	std::vector<Particle> particles;
	uint32_t curIdx;
	AtlasTexture* tex;
	int layer;
	uint32_t flags;
};