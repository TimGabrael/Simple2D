#include "Renderables.h"
#include "util/Math.h"

TextureQuad::TextureQuad(const glm::vec2& pos, const glm::vec2& sz, const glm::vec2& uvStart, const glm::vec2& uvEnd, uint32_t col, int layer)
{
	this->pos = pos;
	this->size = sz;
	this->uvStart = uvStart;
	this->uvEnd = uvEnd;
	this->layer = layer;
	angle = 0;
	this->col = col;
}

void TextureQuad::AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds)
{
	uint32_t cur = verts.size();

	const float s = sinf(angle);
	const float c = cosf(angle);
	
	const float x1 = -size.x * c + size.y * s;
	const float x2 = size.x * c + size.y * s;
	const float x3 = size.x * c - size.y * s;
	const float x4 = -size.x * c - size.y * s;
					 
	const float y1 = -size.x * s - size.y * c;
	const float y2 = size.x * s - size.y * c;
	const float y3 = size.x * s + size.y * c;
	const float y4 = -size.x * s + size.y * c;


	verts.push_back({ {pos.x + x1 / 2.0f, pos.y + y1 / 2.0f}, uvStart, col });
	verts.push_back({ {pos.x + x2 / 2.0f, pos.y + y2 / 2.0f}, {uvEnd.x, uvStart.y}, col });
	verts.push_back({ {pos.x + x3 / 2.0f, pos.y + y3 / 2.0f}, uvEnd, col });
	verts.push_back({ {pos.x + x4 / 2.0f, pos.y + y4 / 2.0f}, {uvStart.x, uvEnd.y}, col });


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