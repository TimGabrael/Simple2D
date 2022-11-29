#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "Physics/Physics.h"
#include "GameState.h"
#include "Renderer.h"


enum RENDERABLE_FLAGS
{
	INACTIVE = 1,
};

struct Renderable
{
	Renderable();
	virtual ~Renderable();
	virtual void Draw(struct RenderContext2D* ctx) = 0;
	virtual int GetLayer() const = 0;
	virtual GLuint GetTexture() const = 0;
	virtual uint32_t GetFlags() const = 0;
};

struct TextureQuad : public Renderable
{
	TextureQuad(const glm::vec2& pos, const glm::vec2& halfSz, const glm::vec2& uvStart, const glm::vec2& uvEnd, GLuint tex = 0, const glm::vec4& col = glm::vec4(1.0f), int layer = 0);
	virtual ~TextureQuad() = default;
	virtual void Draw(RenderContext2D* ctx) override;
	virtual int GetLayer() const;
	virtual GLuint GetTexture() const;
	virtual uint32_t GetFlags() const;

	void UpdateFromBody(b2Body* body);

	glm::vec2 pos;
	glm::vec2 halfSize;
	glm::vec2 uvStart;
	glm::vec2 uvEnd;
	glm::vec4 col;
	float angle;
	int layer = 0;
	GLuint texture;
	uint32_t flags;
};

struct AnimatedQuad : public Renderable
{
	AnimatedQuad(const glm::vec2& pos, const glm::vec2& halfSz, struct AtlasTexture* tex, const glm::vec4& col = glm::vec4(1.0f), int layer = 0);
	virtual ~AnimatedQuad() = default;

	virtual void Draw(RenderContext2D* ctx) override;
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
	glm::vec4 col;
	struct AtlasTexture* atlas;
	float angle;
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
	glm::vec4 colBegin;
	glm::vec4 colEnd;
	uint32_t textureIdx;
	float rotationBegin;
	float rotationEnd;
	float lifeTime = 1.0f;
	float lifeRemaining = 0.0f;
	bool active = false;
	void Draw(const AtlasTexture& atlas, RenderContext2D* ctx) const;
};

struct ParticlesBase : public Renderable
{
	ParticlesBase(struct AtlasTexture* atlas, uint32_t numInPool, int layer = INT_MAX);
	~ParticlesBase() = default;

	virtual void Draw(RenderContext2D* ctx) override;

	void Update(float dt);


	void AddParticle(const glm::vec2& pos, const glm::vec2& vel, const glm::vec2& sizeBegin, const glm::vec2& sizeEnd, const glm::vec4& colBegin, const glm::vec4& colEnd, uint32_t textureIdx, float rotationBegin, float rotationEnd, float lifeTime);


	virtual int GetLayer() const;
	virtual GLuint GetTexture() const;
	virtual uint32_t GetFlags() const;

	std::vector<Particle> particles;
	uint32_t curIdx;
	AtlasTexture* tex;
	int layer;
	uint32_t flags;
};