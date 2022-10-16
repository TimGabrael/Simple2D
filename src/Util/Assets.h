#pragma once 
#include "Math.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

struct Texture
{
	GLuint uniform;
	uint32_t width;
	uint32_t height;
	GLenum type;
};

struct AtlasTexture
{
	Texture texture;
	struct UVBound
	{
		glm::vec2 start;
		glm::vec2 end;
	};
	UVBound* bounds;
	uint32_t numBounds;
};
struct Glyph
{
	float leftSideBearing;
	float yStart;
	float advance;
	float width;
	float height;
};
struct FontMetrics
{
	Glyph* glyphs;
	uint32_t numGlyphs;
	uint32_t firstCharacter;
	uint32_t atlasIdx;
	float ascent;
	float descent;
	float lineGap;
	float size;
};




struct AtlasBuildData* AM_BeginAtlasTexture();

bool AM_AtlasAddFromFile(struct AtlasBuildData* data, const char* file);
FontMetrics* AM_AtlasAddGlyphRangeFromFile(struct AtlasBuildData* data, const char* fontFile, uint32_t first, uint32_t last, float fontSize);
bool AM_AtlasAddRawData(struct AtlasBuildData* data, uint32_t* rawData, uint32_t width, uint32_t height);
bool AM_AtlasAddSubRawData(struct AtlasBuildData* data, uint32_t* rawData, uint32_t startX, uint32_t startY, uint32_t endX, uint32_t endY, uint32_t rawWidth);

void AM_StoreTextureAtlas(const char* file, struct AtlasBuildData* data, FontMetrics** metrics, uint32_t numFontMetrics);
AtlasTexture* AM_EndTextureAtlas(struct AtlasBuildData* data, bool linear);
AtlasTexture* AM_LoadTextureAtlas(const char* file, FontMetrics** metrics, uint32_t* numFontMetrics, bool linear);

Texture AM_LoadTexture(const char* file, bool linear);
void AM_CleanUpTexture(Texture* texture);