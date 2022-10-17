#include "Renderables.h"
#include "util/Math.h"



TextureQuad::TextureQuad(const glm::vec2& pos, const glm::vec2& halfSz, const glm::vec2& uvStart, const glm::vec2& uvEnd, GLuint tex, uint32_t col, int layer)
{
	this->pos = pos;
	this->halfSize = halfSz;
	this->uvStart = uvStart;
	this->uvEnd = uvEnd;
	this->layer = layer;
	this->angle = 0;
	this->col = col;
	this->texture = tex;
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


AnimatedQuad::AnimatedQuad(const glm::vec2& pos, const glm::vec2& halfSz, AtlasTexture* tex, uint32_t col, int layer)
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