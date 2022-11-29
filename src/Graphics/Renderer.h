#pragma once
#define MAX_BLOOM_MIPMAPS 8
#include "../Util/Math.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Scene.h"
#include "Renderables.h"


struct IntermediateRenderData
{
	GLuint intermediateFbo;
	GLuint intermediateTexture;

	uint32_t width;
	uint32_t height;
};
struct PostProcessingRenderData
{
	GLuint* ppFBOs1;
	GLuint* ppFBOs2;
	GLuint ppTexture1;
	GLuint ppTexture2;

	GLuint intermediateFbo;
	GLuint intermediateTexture;

	uint32_t width;
	uint32_t height;
	float blurRadius;
	float bloomIntensity;
	int numPPFbos;
};

struct Vertex2D
{
	glm::vec2 pos;
	glm::vec2 uv;
	glm::vec4 col;
};


struct Renderer* RE_CreateRenderer();
void RE_CleanUpRenderer(struct Renderer* r);

void RE_CreateIntermediateRenderData(IntermediateRenderData* data, uint32_t width, uint32_t height);
void RE_CleanUpIntermediateRenderData(IntermediateRenderData* data);
void RE_CreatePostProcessingRenderData(PostProcessingRenderData* data, uint32_t width, uint32_t height);
void RE_CleanUpPostProcessingRenderData(PostProcessingRenderData* data);


void RE_RenderScene(struct Renderer* renderer, const glm::mat4& viewProj, Scene* scene);


void RE_RenderPostProcessingBloom(struct Renderer* renderer, const PostProcessingRenderData* ppData, GLuint srcTexture, uint32_t srcWidth, uint32_t srcHeight, GLuint targetFBO, uint32_t targetWidth, uint32_t targetHeight);


struct RenderContext2D
{
	void Draw(Vertex2D* vtx, uint32_t numVerts, uint32_t* inds, uint32_t numInds, GLuint texture);
	void DrawQuad(const glm::vec2& tl, const glm::vec2& br, const glm::vec2& uvTL, const glm::vec2& uvBR, const glm::vec4& col, GLuint texture, float angle);



};