#include "Assets.h"

#include "zlib.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_truetype.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"

#include "finders_interface.h"


using namespace rectpack2D;
constexpr bool allow_flip = false;
const auto runtime_flipping_mode = rectpack2D::flipping_option::DISABLED;
using spaces_type = rectpack2D::empty_spaces<allow_flip, rectpack2D::default_empty_spaces>;
using rect_type = output_rect_t<spaces_type>;
struct AtlasBuildData
{
	static constexpr size_t max = 0x8000;
	uint32_t* copyData = nullptr;
	uint32_t* data = nullptr;
	uint32_t copyWidth = 0;
	uint32_t copyHeight = 0;
	uint32_t curWidth = 0;
	uint32_t curHeight = 0;
	std::vector<rect_type> rects;
	void CopyRectToRect(const rect_type& cpyRec, uint32_t dstStartX, uint32_t dstStartY)
	{
		for (uint32_t y = 0; y < cpyRec.h; y++)
		{
			for (uint32_t x = 0; x < cpyRec.w; x++)
			{
				data[x + dstStartX + (y + dstStartY) * curWidth] = copyData[x + cpyRec.x + (y + cpyRec.y) * copyWidth];
			}
		}
	}
};
struct AtlasBuildData* AM_BeginAtlasTexture()
{
	AtlasBuildData* data = new AtlasBuildData;
	return data;
}



bool AM_AtlasAddFromFile(struct AtlasBuildData* data, const char* file)
{
	int x, y, comp;
	stbi_uc* fileData = stbi_load(file, &x, &y, &comp, 4);
	bool res = false;
	if (fileData)
	{
		res = AM_AtlasAddRawData(data, (uint32_t*)fileData, x, y);
		stbi_image_free(fileData);
	}
	return res;
}
FontMetrics* AM_AtlasAddGlyphRangeFromFile(struct AtlasBuildData* data, const char* fontFile, uint32_t first, uint32_t last, float fontSize)
{
	uint8_t* fontData = nullptr;
	{
		FILE* fileHandle = NULL;
		errno_t err = fopen_s(&fileHandle, fontFile, "rb");
		if (err != 0) {
			return nullptr;
		}
		fseek(fileHandle, 0, SEEK_END);
		int size = ftell(fileHandle);
		fseek(fileHandle, 0, SEEK_SET);

		fontData = new uint8_t[size];
		fread(fontData, size, 1, fileHandle);
		fclose(fileHandle);
	}

	stbtt_fontinfo info;
	if (!stbtt_InitFont(&info, fontData, 0))
	{
		delete[] fontData;
		return nullptr;
	}
	const float scale = stbtt_ScaleForPixelHeight(&info, fontSize);

	uint32_t tempWidthHeight = (int)(fontSize + 100);
	uint32_t* tempColor = new uint32_t[tempWidthHeight * tempWidthHeight];
	uint8_t* tempBitmap = new uint8_t[tempWidthHeight * tempWidthHeight];

	FontMetrics* metrics = new FontMetrics;
	memset(metrics, 0, sizeof(FontMetrics));
	metrics->size = fontSize;
	metrics->atlasIdx = data->rects.size();

	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

	metrics->ascent = roundf(ascent * scale);
	metrics->descent = roundf(descent * scale);
	metrics->lineGap = roundf(lineGap * scale);
	metrics->firstCharacter = first;
	metrics->numGlyphs = last - first;
	metrics->glyphs = new Glyph[metrics->numGlyphs];
	memset(metrics->glyphs, 0, sizeof(Glyph) * metrics->numGlyphs);

	for (uint32_t i = first; i < last; i++)
	{
		Glyph& g = metrics->glyphs[i - first];
		int advance = 0, leftSideBearing = 0;
		stbtt_GetGlyphHMetrics(&info, i, &advance, &leftSideBearing);
		g.advance = advance * scale;
		g.leftSideBearing = leftSideBearing * scale;

		memset(tempBitmap, 0, sizeof(uint8_t) * tempWidthHeight * tempWidthHeight);

		int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
		stbtt_GetCodepointBitmapBox(&info, i, scale, scale, &x1, &y1, &x2, &y2);

		g.width = (float)(x2 - x1);
		g.height = (float)(y2 - y1);
		g.leftSideBearing = x1;
		g.yStart = (float)y1;

		stbtt_MakeCodepointBitmap(&info, tempBitmap, g.width, g.height, tempWidthHeight, scale, scale, i);

		for (uint32_t j = 0; j < tempWidthHeight * tempWidthHeight; j++)
		{
			tempColor[j] = 0xFFFFFF | (tempBitmap[j] << 24);
		}

		if (!AM_AtlasAddSubRawData(data, tempColor, 0, 0, g.width, g.height, tempWidthHeight))
		{
			delete[] fontData;
			delete[] metrics->glyphs;
			delete[] tempBitmap;
			delete[] tempColor;
			delete metrics;
			return nullptr;
		}

	}

	delete[] fontData;
	delete[] tempBitmap;
	delete[] tempColor;


	return metrics;
}
bool AM_AtlasAddRawData(struct AtlasBuildData* data, uint32_t* rawData, uint32_t width, uint32_t height)
{
	return AM_AtlasAddSubRawData(data, rawData, 0, 0, width, height, width);
}
bool AM_AtlasAddSubRawData(struct AtlasBuildData* data, uint32_t* rawData, uint32_t startX, uint32_t startY, uint32_t endX, uint32_t endY, uint32_t rawWidth)
{
	const uint32_t width = endX - startX;
	const uint32_t height = endY - startY;
	std::vector<rect_type> old = data->rects;
	data->rects.emplace_back(0, 0, width, height);
	const int discard_step = -4;
	bool failed = false;
	auto report_successful = [&](rect_type&) { failed = false;  return callback_result::CONTINUE_PACKING; };
	auto report_unsuccessful = [&](rect_type&) { failed = true; return callback_result::ABORT_PACKING; };


	const rect_wh result_size = find_best_packing<spaces_type>(data->rects, make_finder_input(AtlasBuildData::max, discard_step, report_successful, report_unsuccessful, runtime_flipping_mode));
	if (!failed)
	{
		bool sizeChanged = false;
		if (!data->data)
		{
			data->curWidth = result_size.w + 100;
			data->curHeight = result_size.h + 100;
			data->copyWidth = data->curWidth;
			data->copyHeight = data->copyHeight;
			const uint32_t dataSize = data->curWidth * data->curHeight;

			data->data = new uint32_t[dataSize];
			data->copyData = new uint32_t[dataSize];
			memset(data->data, 0, sizeof(uint32_t) * dataSize);
			memset(data->copyData, 0, sizeof(uint32_t) * dataSize);
		}
		memcpy(data->copyData, data->data, sizeof(uint32_t) * data->curWidth * data->curHeight);
		if (data->curWidth <= result_size.w || data->curHeight <= result_size.h)
		{
			sizeChanged = true;
			delete[] data->data;
			data->curWidth = result_size.w + 100;
			data->curHeight = result_size.h + 100;
			data->data = new uint32_t[data->curWidth * data->curHeight];
		}
		memset(data->data, 0, sizeof(uint32_t) * data->curWidth * data->curHeight);
		for (uint32_t i = 0; i < old.size(); i++)
		{
			data->CopyRectToRect(old.at(i), data->rects.at(i).x, data->rects.at(i).y);
		}

		const rect_type& last = data->rects.at(data->rects.size() - 1);
		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				data->data[last.x + x + (y + last.y) * data->curWidth] = rawData[startX + x + (startY + y) * rawWidth];
			}
		}

		if (sizeChanged)
		{
			delete[] data->copyData;
			data->copyWidth = data->curWidth;
			data->copyHeight = data->curHeight;
			data->copyData = new uint32_t[data->curWidth * data->curHeight];
			memset(data->copyData, 0, sizeof(uint32_t) * data->curWidth * data->curHeight);
		}
		return true;
	}
	else
	{
		data->rects = old;
		return false;
	}
}

struct TextureAtlasSerializedHeader
{
	uint32_t width;
	uint32_t height;
	uint32_t numRects;
	uint32_t numMetrics;
};
struct TextureAltasSerializedFontMetrics
{
	uint32_t numGlyphs;
	uint32_t firstCharacter;
	uint32_t atlasIdx;
	float ascent;
	float descent;
	float lineGap;
	float size;
};

void AM_StoreTextureAtlas(const char* file, struct AtlasBuildData* data, FontMetrics** metrics, uint32_t numFontMetrics)
{

	uint32_t fullDataSize = sizeof(TextureAtlasSerializedHeader) + data->curWidth * data->curHeight * sizeof(uint32_t) + sizeof(rect_type) * data->rects.size();
	for (uint32_t i = 0; i < numFontMetrics; i++)
	{
		fullDataSize += (sizeof(FontMetrics) - 8) + sizeof(Glyph) * metrics[i]->numGlyphs;
	}
	uint8_t* fullData = new uint8_t[fullDataSize];

	uint8_t* curFill = fullData;
	TextureAtlasSerializedHeader* header = (TextureAtlasSerializedHeader*)fullData; curFill += sizeof(TextureAtlasSerializedHeader);
	header->width = data->curWidth;
	header->height = data->curHeight;
	header->numMetrics = numFontMetrics;
	header->numRects = data->rects.size();

	memcpy(curFill, data->data, sizeof(uint32_t) * data->curWidth * data->curHeight);
	curFill += sizeof(uint32_t) * data->curWidth * data->curHeight;

	for (uint32_t i = 0; i < header->numRects; i++)
	{
		memcpy(curFill, &data->rects[i], sizeof(rect_type));
		curFill += sizeof(rect_type);
	}


	for (uint32_t i = 0; i < numFontMetrics; i++)
	{
		TextureAltasSerializedFontMetrics* f = (TextureAltasSerializedFontMetrics*)curFill;

		f->ascent = metrics[i]->ascent;
		f->atlasIdx = metrics[i]->atlasIdx;
		f->descent = metrics[i]->descent;
		f->firstCharacter = metrics[i]->firstCharacter;
		f->lineGap = metrics[i]->lineGap;
		f->numGlyphs = metrics[i]->numGlyphs;
		f->size = metrics[i]->size;

		curFill += sizeof(TextureAltasSerializedFontMetrics);
		for (uint32_t j = 0; j < metrics[i]->numGlyphs; j++)
		{
			Glyph& glyph = metrics[i]->glyphs[j];
			Glyph* g = (Glyph*)curFill;
			*g = glyph;
			curFill += sizeof(Glyph);
		}
	}




	uLongf dataLen = fullDataSize;
	uint8_t* compressData = new uint8_t[fullDataSize];
	if (compress(compressData, &dataLen, fullData, fullDataSize) != Z_OK)
	{
		delete[] compressData;
		delete[] fullData;
		return;
	}

	FILE* f = NULL;
	fopen_s(&f, file, "wb");
	if (!f)
	{
		delete[] compressData;
		delete[] fullData;
		return;
	}
	fwrite(&fullDataSize, sizeof(uint32_t), 1, f);
	fwrite(compressData, dataLen, 1, f);
	fclose(f);

	delete[] compressData;
	delete[] fullData;
}
AtlasTexture* AM_EndTextureAtlas(struct AtlasBuildData* data, bool linear)
{
	AtlasTexture* atlas = new AtlasTexture;
	memset(atlas, 0, sizeof(AtlasTexture));

	atlas->texture.width = data->curWidth;
	atlas->texture.height = data->curHeight;
	atlas->texture.type = GL_TEXTURE_2D;
	glGenTextures(1, &atlas->texture.uniform);
	glBindTexture(GL_TEXTURE_2D, atlas->texture.uniform);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data->curWidth, data->curHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data->data);
	if (linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



	atlas->numBounds = data->rects.size();
	atlas->bounds = new AtlasTexture::UVBound[atlas->numBounds];
	memset(atlas->bounds, 0, sizeof(AtlasTexture::UVBound) * atlas->numBounds);
	for (uint32_t i = 0; i < atlas->numBounds; i++)
	{
		rect_type& r = data->rects.at(i);
		atlas->bounds[i].start = { (float)r.x / (float)atlas->texture.width, (float)r.y / (float)atlas->texture.height };
		atlas->bounds[i].end = { (float)(r.x + r.w) / (float)atlas->texture.width, (float)(r.y + r.h) / (float)atlas->texture.height };
	}

	if (data->copyData) delete[] data->copyData;
	delete[] data->data;
	delete data;

	return atlas;
}
struct AtlasTexture* AM_LoadTextureAtlas(const char* file, FontMetrics** metrics, uint32_t* numFontMetrics, bool linear)
{
	uint8_t* fullData = nullptr;
	{
		FILE* f = NULL;
		fopen_s(&f, file, "rb");
		if (!f)
		{
			return nullptr;
		}

		fseek(f, 0, SEEK_END);
		int sz = ftell(f);
		fseek(f, 0, SEEK_SET);

		uint8_t* fileData = new uint8_t[sz];
		fread(fileData, sz, 1, f);
		fclose(f);

		const uint32_t uncompressedSize = *(uint32_t*)fileData;
		uint8_t* curFileData = fileData + sizeof(uint32_t);

		fullData = new uint8_t[uncompressedSize];

		uLongf outSize = uncompressedSize;
		int res = uncompress(fullData, &outSize, curFileData, sz - sizeof(uint32_t));
		delete[] fileData;
	}
	uint8_t* curRead = fullData;
	AtlasTexture* out = new AtlasTexture;

	TextureAtlasSerializedHeader* header = (TextureAtlasSerializedHeader*)fullData; curRead += sizeof(TextureAtlasSerializedHeader);

	out->numBounds = header->numRects;
	out->texture.width = header->width;
	out->texture.height = header->height;
	out->texture.type = GL_TEXTURE_2D;


	glGenTextures(1, &out->texture.uniform);
	glBindTexture(GL_TEXTURE_2D, out->texture.uniform);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, header->width, header->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, curRead);
	if (linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	curRead += sizeof(uint32_t) * header->width * header->height;


	out->bounds = new AtlasTexture::UVBound[header->numRects];
	memset(out->bounds, 0, sizeof(AtlasTexture::UVBound) * header->numRects);
	for (uint32_t i = 0; i < header->numRects; i++)
	{
		rect_type* cur = (rect_type*)curRead;

		out->bounds[i].start = { (float)cur->x / (float)out->texture.width, (float)cur->y / (float)out->texture.height };
		out->bounds[i].end = { (float)(cur->x + cur->w) / (float)out->texture.width, (float)(cur->y + cur->h) / (float)out->texture.height };

		curRead += sizeof(rect_type);
	}


	if (numFontMetrics && metrics)
	{
		*numFontMetrics = header->numMetrics;
		*metrics = new FontMetrics[header->numMetrics];
		memset(*metrics, 0, sizeof(FontMetrics) * header->numMetrics);
		for (uint32_t i = 0; i < header->numMetrics; i++)
		{
			TextureAltasSerializedFontMetrics* f = (TextureAltasSerializedFontMetrics*)curRead;

			(*metrics)[i].ascent = f->ascent;
			(*metrics)[i].atlasIdx = f->atlasIdx;
			(*metrics)[i].descent = f->descent;
			(*metrics)[i].firstCharacter = f->firstCharacter;
			(*metrics)[i].lineGap = f->lineGap;
			(*metrics)[i].numGlyphs = f->numGlyphs;
			(*metrics)[i].size = f->size;


			(*metrics)[i].glyphs = new Glyph[(*metrics)[i].numGlyphs];
			memset((*metrics)[i].glyphs, 0, sizeof(Glyph) * (*metrics)[i].numGlyphs);
			curRead += sizeof(TextureAltasSerializedFontMetrics);
			for (uint32_t j = 0; j < (*metrics)[i].numGlyphs; j++)
			{
				Glyph& glyph = (*metrics)[i].glyphs[j];
				Glyph* g = (Glyph*)curRead;
				glyph = *g;
				curRead += sizeof(Glyph);
			}
		}
	}


	delete[] fullData;
	return out;
}



Texture AM_LoadTexture(const char* file, bool linear)
{
	Texture res;
	memset(&res, 0, sizeof(Texture));
	int x, y, comp;
	stbi_uc* fileData = stbi_load(file, &x, &y, &comp, 4);
	if (fileData)
	{
		res.width = x;
		res.height = y;
		res.type = GL_TEXTURE_2D;
		glGenTextures(1, &res.uniform);
		glBindTexture(GL_TEXTURE_2D, res.uniform);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fileData);
		if (linear)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		stbi_image_free(fileData);
	}

	return res;
}
void AM_CleanUpTexture(Texture* texture)
{

}