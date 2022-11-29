#include "Renderables.h"
#include "util/Math.h"

Renderable::Renderable()
{
	SC_AddRenderable(GetGameState()->scene, this);
}
Renderable::~Renderable()
{
	SC_RemoveRenderable(GetGameState()->scene, this);
}

TextureQuad::TextureQuad(const glm::vec2& pos, const glm::vec2& halfSz, const glm::vec2& uvStart, const glm::vec2& uvEnd, GLuint tex, const glm::vec4& col, int layer)
{
	this->pos = pos;
	this->halfSize = halfSz;
	this->uvStart = uvStart;
	this->uvEnd = uvEnd;
	this->layer = layer;
	this->angle = 0;
	this->col = col;
	this->texture = tex;
	this->flags = 0;
}

void TextureQuad::Draw(RenderContext2D* ctx)
{
	ctx->DrawQuad(pos - halfSize, pos + halfSize, uvStart, uvEnd, col, texture, angle);
}

void TextureQuad::UpdateFromBody(b2Body* body)
{
	auto p = body->GetPosition();
	this->pos = { p.x, p.y };
	this->angle = body->GetAngle();
}
int TextureQuad::GetLayer() const
{
	return layer;
}
GLuint TextureQuad::GetTexture() const
{
	return texture;
}
uint32_t TextureQuad::GetFlags() const
{
	return flags;
}


AnimatedQuad::AnimatedQuad(const glm::vec2& pos, const glm::vec2& halfSz, AtlasTexture* tex, const glm::vec4& col, int layer)
{
	this->pos = pos;
	this->halfSize = halfSz;
	this->layer = layer;
	this->angle = 0;
	this->col = col;
	this->atlas = tex;
	this->texture = tex->texture.uniform;
	animIdx = 0;
	animStepIdx = 0;
	this->flags = 0;
}
void AnimatedQuad::Draw(RenderContext2D* ctx)
{
	if (animIdx < range.size())
	{
		uint32_t idx = range.at(animIdx).startIdx + animStepIdx;
		if (idx < range.at(animIdx).endIdx && idx < atlas->numBounds)
		{
			AtlasTexture::UVBound& bound = atlas->bounds[idx];
			ctx->DrawQuad(pos - halfSize, pos + halfSize, bound.start, bound.end, col, texture, angle);
		}
	}
}
void AnimatedQuad::UpdateFromBody(b2Body* body)
{
	auto p = body->GetPosition();
	this->pos = { p.x, p.y };
	this->angle = body->GetAngle();
}
void AnimatedQuad::AddAnimRange(uint32_t start, uint32_t end)
{
	range.push_back({ start, end });
}
int AnimatedQuad::GetLayer() const
{
	return layer;
}
GLuint AnimatedQuad::GetTexture() const
{
	return texture;
}
uint32_t AnimatedQuad::GetFlags() const
{
	return flags;
}



void Particle::Draw(const AtlasTexture& atlas, RenderContext2D* ctx) const
{
	const float rem = lifeRemaining / lifeTime;
	const float angle = rotationEnd * (1.0f - rem) + rotationBegin * rem;

	const glm::vec2 halfSize = { sizeEnd.x * (1.0f - rem) + sizeBegin.x * rem, sizeEnd.y * (1.0f - rem) + sizeBegin.y * rem };


	const glm::vec4 col = { (colEnd.x * (1.0f - rem) + colBegin.x * rem),
							(colEnd.y * (1.0f - rem) + colBegin.y * rem),
							(colEnd.z * (1.0f - rem) + colBegin.z * rem),
							(colEnd.w * (1.0f - rem) + colBegin.w * rem)
	};
	AtlasTexture::UVBound& bound = atlas.bounds[textureIdx];
	
	ctx->DrawQuad(pos - halfSize, pos + halfSize, bound.start, bound.end, col, atlas.texture.uniform, angle);
}

ParticlesBase::ParticlesBase(AtlasTexture* atlas, uint32_t numInPool, int layer)
{
	particles.resize(numInPool);
	curIdx = 0;
	memset(particles.data(), 0, sizeof(Particle));
	this->layer = layer;
	this->tex = atlas;
	this->flags = 0;
}
void ParticlesBase::Draw(RenderContext2D* ctx)
{
	for (auto& p : particles)
	{
		if (p.active)
		{
			p.Draw(*tex, ctx);
		}
	}
}
void ParticlesBase::Update(float dt)
{
	for (auto& p : particles)
	{
		if (p.active)
		{
			p.lifeRemaining -= dt;
			if (p.lifeRemaining < 0.0f)
			{
				p.active = false;
			}
			p.pos += p.vel * dt;
		}
	}
}
void ParticlesBase::AddParticle(const glm::vec2& pos, const glm::vec2& vel, const glm::vec2& sizeBegin, const glm::vec2& sizeEnd, const glm::vec4& colBegin, const glm::vec4& colEnd, uint32_t textureIdx, float rotationBegin, float rotationEnd, float lifeTime)
{
	if (textureIdx < tex->numBounds)
	{
		Particle& p = particles.at(curIdx);

		p.pos = pos;
		p.vel = vel;
		p.colBegin = colBegin;
		p.colEnd = colEnd;
		p.textureIdx = textureIdx;
		p.lifeRemaining = lifeTime;
		p.lifeTime = lifeTime;
		p.rotationBegin = rotationBegin;
		p.rotationEnd = rotationEnd;
		p.sizeBegin = sizeBegin;
		p.sizeEnd = sizeEnd;
		p.active = true;

		curIdx = (curIdx + 1) % particles.size();
	}
}
int ParticlesBase::GetLayer() const
{
	return layer;
}
GLuint ParticlesBase::GetTexture() const
{
	return tex->texture.uniform;
}
uint32_t ParticlesBase::GetFlags() const
{
	return flags;
}