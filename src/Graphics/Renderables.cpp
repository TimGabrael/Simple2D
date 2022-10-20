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

void TextureQuad::AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds)
{
	uint32_t cur = verts.size();

	const float s = sinf(angle);
	const float c = cosf(angle);
	
	const float x1 = -halfSize.x * c + halfSize.y * s;
	const float x2 = halfSize.x * c + halfSize.y * s;
	const float x3 = halfSize.x * c - halfSize.y * s;
	const float x4 = -halfSize.x * c - halfSize.y * s;
					 
	const float y1 = -halfSize.x * s - halfSize.y * c;
	const float y2 = halfSize.x * s - halfSize.y * c;
	const float y3 = halfSize.x * s + halfSize.y * c;
	const float y4 = -halfSize.x * s + halfSize.y * c;


	verts.push_back({ {pos.x + x1, (pos.y + y1) }, {uvStart.x, uvEnd.y}, col });
	verts.push_back({ {pos.x + x2, (pos.y + y2) }, {uvEnd.x, uvEnd.y}, col });
	verts.push_back({ {pos.x + x3, (pos.y + y3) }, {uvEnd.x, uvStart.y}, col });
	verts.push_back({ {pos.x + x4, (pos.y + y4) }, {uvStart.x, uvStart.y}, col });


	inds.push_back(cur);
	inds.push_back(cur+1);
	inds.push_back(cur+2);
	inds.push_back(cur+2);
	inds.push_back(cur+3);
	inds.push_back(cur);
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
void AnimatedQuad::AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds)
{
	if (animIdx < range.size())
	{
		uint32_t idx = range.at(animIdx).startIdx + animStepIdx;
		if (idx < range.at(animIdx).endIdx && idx < atlas->numBounds)
		{
			uint32_t cur = verts.size();

			const float s = sinf(angle);
			const float c = cosf(angle);

			const float x1 = -halfSize.x * c + halfSize.y * s;
			const float x2 = halfSize.x * c + halfSize.y * s;
			const float x3 = halfSize.x * c - halfSize.y * s;
			const float x4 = -halfSize.x * c - halfSize.y * s;

			const float y1 = -halfSize.x * s - halfSize.y * c;
			const float y2 = halfSize.x * s - halfSize.y * c;
			const float y3 = halfSize.x * s + halfSize.y * c;
			const float y4 = -halfSize.x * s + halfSize.y * c;

			AtlasTexture::UVBound& bound = atlas->bounds[idx];

			verts.push_back({ {pos.x + x1, (pos.y + y1) }, {bound.start.x, bound.end.y}, col });
			verts.push_back({ {pos.x + x2, (pos.y + y2) }, {bound.end.x, bound.end.y}, col });
			verts.push_back({ {pos.x + x3, (pos.y + y3) }, {bound.end.x, bound.start.y}, col });
			verts.push_back({ {pos.x + x4, (pos.y + y4) }, {bound.start.x, bound.start.y}, col });


			inds.push_back(cur);
			inds.push_back(cur + 1);
			inds.push_back(cur + 2);
			inds.push_back(cur + 2);
			inds.push_back(cur + 3);
			inds.push_back(cur);
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



void Particle::AddToVertices(const AtlasTexture& atlas, std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds) const
{
	uint32_t cur = verts.size();

	const float rem = lifeRemaining / lifeTime;
	const float angle = rotationEnd * (1.0f - rem) + rotationBegin * rem;

	const glm::vec2 halfSize = { sizeEnd.x * (1.0f - rem) + sizeBegin.x * rem, sizeEnd.y * (1.0f - rem) + sizeBegin.y * rem };


	const glm::vec4 col = { (colEnd.x * (1.0f - rem) + colBegin.x * rem),
							(colEnd.y * (1.0f - rem) + colBegin.y * rem),
							(colEnd.z * (1.0f - rem) + colBegin.z * rem),
							(colEnd.w * (1.0f - rem) + colBegin.w * rem)
	};
		

	const float s = sinf(angle);
	const float c = cosf(angle);
	
	
	const float x1 = -halfSize.x* c + halfSize.y * s;
	const float x2 = halfSize.x * c + halfSize.y * s;
	const float x3 = halfSize.x * c - halfSize.y * s;
	const float x4 = -halfSize.x * c - halfSize.y * s;

	const float y1 = -halfSize.x * s - halfSize.y * c;
	const float y2 = halfSize.x * s - halfSize.y * c;
	const float y3 = halfSize.x * s + halfSize.y * c;
	const float y4 = -halfSize.x * s + halfSize.y * c;



	AtlasTexture::UVBound& bound = atlas.bounds[textureIdx];

	verts.push_back({ {pos.x + x1, (pos.y + y1) }, {bound.start.x, bound.end.y}, col });
	verts.push_back({ {pos.x + x2, (pos.y + y2) }, {bound.end.x, bound.end.y}, col });
	verts.push_back({ {pos.x + x3, (pos.y + y3) }, {bound.end.x, bound.start.y}, col });
	verts.push_back({ {pos.x + x4, (pos.y + y4) }, {bound.start.x, bound.start.y}, col });


	inds.push_back(cur);
	inds.push_back(cur + 1);
	inds.push_back(cur + 2);
	inds.push_back(cur + 2);
	inds.push_back(cur + 3);
	inds.push_back(cur);
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
void ParticlesBase::AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds)
{
	for (auto& p : particles)
	{
		if (p.active)
		{
			p.AddToVertices(*tex, verts, inds);
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