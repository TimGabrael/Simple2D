#include "Renderer.h"
#include "../Util/Math.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Renderables.h"
#include <algorithm>

static const char* quadFilterVertexShader = "#version 330\n\
out vec2 UV;\n\
out vec2 pos;\n\
void main()\n\
{\n\
	UV = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);\n\
	pos = UV * 2.0f - 1.0f;\n\
	gl_Position = vec4(pos, 0.0f, 1.0f);\n\
}";
static const char* blurFragmentShader = "#version 330\n\
in vec2 UV;\
in vec2 pos;\
uniform sampler2D tex;\
uniform float blurRadius;\
uniform int axis;\
uniform vec2 texUV;\
out vec4 outCol;\
void main()\
{\
    vec2 textureSize = vec2(textureSize(tex, 0));\
    float x,y,rr=blurRadius*blurRadius,d,w,w0;\
    vec2 p = 0.5 * (vec2(1.0, 1.0) + pos) * texUV;\
    vec4 col = vec4(0.0, 0.0, 0.0, 0.0);\
    w0 = 0.5135 / pow(blurRadius, 0.96);\n\
    if (axis == 0) for (d = 1.0 / textureSize.x, x = -blurRadius, p.x += x * d; x <= blurRadius; x++, p.x += d) { \n\
    w = w0 * exp((-x * x) / (2.0 * rr)); col += texture(tex, p) * w;\n\
 }\n\
    if (axis == 1) for (d = 1.0 / textureSize.y, y = -blurRadius, p.y += y * d; y <= blurRadius; y++, p.y += d) { \n\
    w = w0 * exp((-y * y) / (2.0 * rr)); col += texture(tex, p) * w; \n\
}\n\
    outCol = col;\
}";
static const char* bloomFragmentShader = "#version 330\n\
in vec2 UV;\
in vec2 pos;\
uniform sampler2D tex;\
uniform float blurRadius;\
uniform int axis;\
uniform float intensity;\
uniform float mipLevel;\
out vec4 outCol;\
void main()\
{\
    vec2 textureSize = vec2(textureSize(tex, int(mipLevel)));\
    float x,y,rr=blurRadius*blurRadius,d,w,w0;\
    vec2 p = 0.5 * (vec2(1.0, 1.0) + pos);\
    vec4 col = vec4(0.0, 0.0, 0.0, 0.0);\
    w0 = 0.5135 / pow(blurRadius, 0.96);\n\
    if (axis == 0) for (d = 1.0 / textureSize.x, x = -blurRadius, p.x += x * d; x <= blurRadius; x++, p.x += d) { w = w0 * exp((-x * x) / (2.0 * rr));\
            vec3 addCol = textureLod(tex, p, mipLevel).rgb;\
            vec3 remCol = vec3(1.0f, 1.0f, 1.0f) * intensity;\n\
            addCol = max(addCol - remCol, vec3(0.0f));\
            col += vec4(addCol, 0.0f) * w;\
        }\n\
    if (axis == 1) for (d = 1.0 / textureSize.y, y = -blurRadius, p.y += y * d; y <= blurRadius; y++, p.y += d) { w = w0 * exp((-y * y) / (2.0 * rr));\
            vec3 addCol = textureLod(tex, p, mipLevel).rgb;\
            vec3 remCol = vec3(1.0f, 1.0f, 1.0f) * intensity;\n\
            addCol = max(addCol - remCol, vec3(0.0f));\
            col += vec4(addCol, 0.0f) * w;\
        }\n\
    outCol = col;\
}";
static const char* copyFragmentShader = "#version 330\n\
in vec2 UV;\
in vec2 pos;\
uniform sampler2D tex;\
uniform float mipLevel;\
out vec4 outCol;\
void main()\
{\
    outCol = textureLod(tex, UV, mipLevel);\
}";
static const char* upsamplingFragmentShader = "#version 330\n\
in vec2 UV;\
in vec2 pos;\
uniform sampler2D tex;\
uniform float mipLevel;\
out vec4 outCol;\
void main()\
{\
    vec2 ts = vec2(1.0f) / vec2(textureSize(tex, int(mipLevel)));\
    vec3 c1 = textureLod(tex, UV + vec2(-ts.x, -ts.y), mipLevel).rgb;\
    vec3 c2 = 2.0f * textureLod(tex, UV + vec2(0.0f, -ts.y), mipLevel).rgb;\
    vec3 c3 = textureLod(tex, UV + vec2(ts.x, -ts.y), mipLevel).rgb;\
    vec3 c4 = 2.0f * textureLod(tex, UV + vec2(-ts.x, 0.0f), mipLevel).rgb;\
    vec3 c5 = 4.0f * textureLod(tex, UV + vec2(0.0f, 0.0f), mipLevel).rgb;\
    vec3 c6 = 2.0f * textureLod(tex, UV + vec2(ts.x, 0.0f), mipLevel).rgb;\
    vec3 c7 = textureLod(tex, UV + vec2(-ts.x, ts.y), mipLevel).rgb;\
    vec3 c8 = 2.0f * textureLod(tex, UV + vec2(0.0f, ts.y), mipLevel).rgb;\
    vec3 c9 = textureLod(tex, UV + vec2(ts.x, ts.y), mipLevel).rgb;\
    vec3 accum = 1.0f / 16.0f * (c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9);\
    outCol = vec4(accum, 1.0f);\
}";
static const char* copyDualFragmentShader = "#version 330\n\
in vec2 UV;\
in vec2 pos;\
uniform sampler2D tex1;\
uniform sampler2D tex2;\
uniform float mipLevel1;\
uniform float mipLevel2;\
out vec4 outCol;\
void main()\
{\
    vec4 c = textureLod(tex1, UV, mipLevel1) + textureLod(tex2, UV, mipLevel2);\n\
    outCol = c;\
}";


static const char* stdVertexShader = "#version 330\n\
in vec2 pos;\
in vec2 uv;\
in vec4 baseColor;\
uniform mat4 viewProj;\
out vec2 fragUV;\
out vec4 fragCol;\
void main()\
{\
	fragUV = uv;\
	fragCol = baseColor;\
	gl_Position = viewProj * vec4(pos, 0.0f, 1.0f);\
}";
static const char* stdFragmentShader = "#version 330\n\
in vec2 fragUV;\
in vec4 fragCol;\
out vec4 outCol;\
uniform sampler2D baseTexture;\
void main()\
{\
	vec4 c = texture(baseTexture, fragUV) * fragCol;\
	if(c.a == 0.0f) discard;\
	outCol = c;\
}";


struct RenderCommand
{
	uint32_t firstInd = 0;
	uint32_t indCount = 0;
	GLuint texture = 0;
};


struct InternalRenderContext2D
{
	void Draw(Vertex2D* vtx, uint32_t numVerts, uint32_t* inds, uint32_t numInds, GLuint texture)
	{
		RenderCommand* command = nullptr;
		{
			uint32_t cmdCount = (uint32_t)cmds.size();
			for(uint32_t i = 0; i < cmdCount; i++)
			{
				if (cmds.at(i).texture == texture)
				{
					command = &cmds.at(i);
				}
			}
			if (!command)
			{
				uint32_t prevEnd = 0;
				if (cmdCount > 0)
				{
					prevEnd = cmds.at(cmdCount - 1).firstInd + cmds.at(cmdCount - 1).indCount;
				}
				cmds.push_back({});
				command = &cmds.at(cmdCount);
				command->firstInd = prevEnd;
				command->indCount = 0;
				command->texture = texture;
			}
		}

		uint32_t oldSz = (uint32_t)vertices.size();
		vertices.resize(oldSz + numVerts);
		memcpy(vertices.data() + oldSz, vtx, numVerts * sizeof(Vertex2D));

		for (uint32_t i = 0; i < numInds; i++)
		{
			indices.push_back(inds[i] + oldSz);
		}

		command->indCount += numInds;
	}
	void DrawQuad(const glm::vec2& tl, const glm::vec2& br, const glm::vec2& uvTL, const glm::vec2& uvBR, const glm::vec4& col, GLuint texture, float angle)
	{
		const float s = sinf(angle);
		const float c = cosf(angle);

		glm::vec2 halfSize = (br - tl) / 2.0f;

		const glm::vec2 pos = tl + halfSize;
		halfSize = glm::abs(halfSize);

		const float x1 = -halfSize.x * c + halfSize.y * s;
		const float x2 = halfSize.x * c + halfSize.y * s;
		const float x3 = halfSize.x * c - halfSize.y * s;
		const float x4 = -halfSize.x * c - halfSize.y * s;

		const float y1 = -halfSize.x * s - halfSize.y * c;
		const float y2 = halfSize.x * s - halfSize.y * c;
		const float y3 = halfSize.x * s + halfSize.y * c;
		const float y4 = -halfSize.x * s + halfSize.y * c;

		
		Vertex2D verts[4] = {
			{ {pos.x + x1, (pos.y + y1) }, {uvTL.x, uvBR.y}, col },
			{ {pos.x + x2, (pos.y + y2) }, {uvBR.x, uvBR.y}, col },
			{ {pos.x + x3, (pos.y + y3) }, {uvBR.x, uvTL.y}, col },
			{ {pos.x + x4, (pos.y + y4) }, {uvTL.x, uvTL.y}, col }
		};

		uint32_t inds[6] = {
			0,1,2,2,3,0
		};

		Draw(verts, 4, inds, 6, texture);
	}

	std::vector<RenderCommand> cmds;
	std::vector<Vertex2D> vertices;
	std::vector<uint32_t> indices;
};




static GLuint CreateProgram(const char* vtxShader, const char* frgShader)
{
	GLuint out = glCreateProgram();

	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertShader, 1, &vtxShader, NULL);
	glShaderSource(fragShader, 1, &frgShader, NULL);

	glCompileShader(vertShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* errorLog = new char[maxLength];
		glGetShaderInfoLog(vertShader, maxLength, &maxLength, &errorLog[0]);

		LOG("FAILED TO COMPILE VERTEXSHADER: %s\n", errorLog);
		delete[] errorLog;

		glDeleteShader(vertShader);
	}

	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* errorLog = new char[maxLength];
		glGetShaderInfoLog(fragShader, maxLength, &maxLength, &errorLog[0]);

		LOG("FAILED TO COMPILE FRAGMENTSHADER: %s\n", errorLog);
		delete[] errorLog;

		glDeleteShader(fragShader);
	}


	glAttachShader(out, vertShader);
	glAttachShader(out, fragShader);

	glLinkProgram(out);
	glDetachShader(out, vertShader);
	glDetachShader(out, fragShader);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return out;
}


struct PostProcessingRenderInfo
{
	enum class BLUR_AXIS
	{
		X_AXIS,
		Y_AXIS,
	};
	struct Blur
	{
		GLuint program;
		GLuint radiusLoc;
		GLuint axisLoc;
		GLuint textureUVLoc;
	}blur;
	struct Bloom
	{
		GLuint program;
		GLuint blurRadiusLoc;
		GLuint blurAxisLoc;
		GLuint intensityLoc;
		GLuint mipLevelLoc;
	}bloom;
	struct Copy
	{
		GLuint program;
		GLuint mipLevelLoc;
	}copy;
	struct DualCopy
	{
		GLuint program;
		GLuint mipLevel1Loc;
		GLuint mipLevel2Loc;
		GLuint exposureLoc;
		GLuint gammaLoc;
	}dualCopy;
	struct Upsampling
	{
		GLuint program;
		GLuint mipLevelLoc;
	}upsampling;
};
static void LoadPostProcessingRenderInfo(PostProcessingRenderInfo* info)
{
	info->blur.program = CreateProgram(quadFilterVertexShader, blurFragmentShader);
	info->bloom.program = CreateProgram(quadFilterVertexShader, bloomFragmentShader);
	info->copy.program = CreateProgram(quadFilterVertexShader, copyFragmentShader);
	info->dualCopy.program = CreateProgram(quadFilterVertexShader, copyDualFragmentShader);
	info->upsampling.program = CreateProgram(quadFilterVertexShader, upsamplingFragmentShader);

	info->blur.radiusLoc = glGetUniformLocation(info->blur.program, "blurRadius");
	info->blur.axisLoc = glGetUniformLocation(info->blur.program, "axis");
	info->blur.textureUVLoc = glGetUniformLocation(info->blur.program, "texUV");

	info->bloom.blurRadiusLoc = glGetUniformLocation(info->bloom.program, "blurRadius");
	info->bloom.blurAxisLoc = glGetUniformLocation(info->bloom.program, "axis");
	info->bloom.intensityLoc = glGetUniformLocation(info->bloom.program, "intensity");
	info->bloom.mipLevelLoc = glGetUniformLocation(info->bloom.program, "mipLevel");

	info->copy.mipLevelLoc = glGetUniformLocation(info->copy.program, "mipLevel");

	info->dualCopy.mipLevel1Loc = glGetUniformLocation(info->dualCopy.program, "mipLevel1");
	info->dualCopy.mipLevel2Loc = glGetUniformLocation(info->dualCopy.program, "mipLevel2");
	info->dualCopy.exposureLoc = glGetUniformLocation(info->dualCopy.program, "exposure");
	info->dualCopy.gammaLoc = glGetUniformLocation(info->dualCopy.program, "gamma");

	glUseProgram(info->dualCopy.program);
	GLuint idx = glGetUniformLocation(info->dualCopy.program, "tex1");
	glUniform1i(idx, 0);
	idx = glGetUniformLocation(info->dualCopy.program, "tex2");
	glUniform1i(idx, 1);

	info->upsampling.mipLevelLoc = glGetUniformLocation(info->upsampling.program, "mipLevel");



}
static void CleanUpPostProcessingRenderInfo(PostProcessingRenderInfo* info)
{
	glDeleteProgram(info->blur.program);
	glDeleteProgram(info->bloom.program);
	glDeleteProgram(info->copy.program);
	glDeleteProgram(info->dualCopy.program);
	glDeleteProgram(info->upsampling.program);
}


struct BaseRenderInfo
{
	GLuint program;
	GLuint viewProjLoc;
};

struct Renderer
{
	BaseRenderInfo info;
	PostProcessingRenderInfo ppInfo;

	GLuint whiteTexture;
	GLuint blackTexture;

	GLuint vao;
	GLuint vertexBuffer;
	GLuint indexBuffer;

	InternalRenderContext2D renderContext;
};

struct Renderer* RE_CreateRenderer()
{
	Renderer* out = new Renderer;
	memset(out, 0, sizeof(Renderer) - sizeof(InternalRenderContext2D));


	out->info.program = CreateProgram(stdVertexShader, stdFragmentShader);
	out->info.viewProjLoc = glGetUniformLocation(out->info.program, "viewProj");
	{
		glGenVertexArrays(1, &out->vao);
		glBindVertexArray(out->vao);
		glGenBuffers(1, &out->vertexBuffer);
		glGenBuffers(1, &out->indexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, out->vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->indexBuffer);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), nullptr);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, uv));
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, col));

		glEnableVertexArrayAttrib(out->vao, 0);
		glEnableVertexArrayAttrib(out->vao, 1);
		glEnableVertexArrayAttrib(out->vao, 2);



	}
	LoadPostProcessingRenderInfo(&out->ppInfo);
	{
		uint32_t col = 0xFFFFFFFF;
		glGenTextures(1, &out->whiteTexture);
		glBindTexture(GL_TEXTURE_2D, out->whiteTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &col);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		col = 0;
		glGenTextures(1, &out->blackTexture);
		glBindTexture(GL_TEXTURE_2D, out->blackTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &col);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	return out;
}
void RE_CleanUpRenderer(struct Renderer* r)
{
	glDeleteProgram(r->info.program);
	r->info.program = 0;
	r->info.viewProjLoc = 0;
	CleanUpPostProcessingRenderInfo(&r->ppInfo);
	glDeleteVertexArrays(1, &r->vao);
	glDeleteBuffers(1, &r->vertexBuffer);
	glDeleteBuffers(1, &r->indexBuffer);
	glDeleteTextures(1, &r->blackTexture);
	glDeleteTextures(1, &r->whiteTexture);
	r->vao = 0;
	r->vertexBuffer = 0;
	r->indexBuffer = 0;
	r->blackTexture = 0;
	r->whiteTexture = 0;
	delete r;
}

void RE_CreateIntermediateRenderData(IntermediateRenderData* data, uint32_t width, uint32_t height)
{
	data->width = width;
	data->height = height;
	glGenFramebuffers(1, &data->intermediateFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, data->intermediateFbo);

	glGenTextures(1, &data->intermediateTexture);
	glBindTexture(GL_TEXTURE_2D, data->intermediateTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, data->width, data->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->intermediateTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG("FAILED TO CREATE INTERMEDIATE FRAMEBUFFER\n");
	}
}
void RE_CleanUpIntermediateRenderData(IntermediateRenderData* data)
{
	glDeleteTextures(1, &data->intermediateTexture);
	glDeleteFramebuffers(1, &data->intermediateFbo);
}
void RE_CreatePostProcessingRenderData(PostProcessingRenderData* data, uint32_t width, uint32_t height)
{
	data->width = width;
	data->height = height;

	data->blurRadius = 4.0f;		// set standard value for blur
	data->bloomIntensity = 1.0f;	// set standard value for bloom


	data->numPPFbos = (int)(1 + floor(log2(glm::max(data->width, data->height))));
	int curX = width; int curY = height;
	for (int i = 1; i < data->numPPFbos; i++)
	{
		curX = glm::max((curX >> 1), 1);
		curY = glm::max((curY >> 1), 1);
		if (curX < 16 && curY < 16 && false)
		{
			data->numPPFbos = i;
			break;
		}
	}
	data->numPPFbos = glm::min(data->numPPFbos, MAX_BLOOM_MIPMAPS);
	{
		data->ppFBOs1 = new GLuint[data->numPPFbos];
		glGenFramebuffers(data->numPPFbos, data->ppFBOs1);

		glGenTextures(1, &data->ppTexture1);
		glBindTexture(GL_TEXTURE_2D, data->ppTexture1);
		glTexStorage2D(GL_TEXTURE_2D, data->numPPFbos, GL_RGBA16F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		for (int i = 0; i < data->numPPFbos; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, data->ppFBOs1[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->ppTexture1, i);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				LOG("FAILED  TO CREATE FRAMEBUFFER\n");
			}
		}
	}
	{
		data->ppFBOs2 = new GLuint[data->numPPFbos];
		glGenFramebuffers(data->numPPFbos, data->ppFBOs2);

		glGenTextures(1, &data->ppTexture2);
		glBindTexture(GL_TEXTURE_2D, data->ppTexture2);
		glTexStorage2D(GL_TEXTURE_2D, data->numPPFbos, GL_RGBA16F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		for (int i = 0; i < data->numPPFbos; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, data->ppFBOs2[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->ppTexture2, i);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				LOG("FAILED  TO CREATE FRAMEBUFFER\n");
			}
		}

	}

	glGenFramebuffers(1, &data->intermediateFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, data->intermediateFbo);
	glGenTextures(1, &data->intermediateTexture);
	glBindTexture(GL_TEXTURE_2D, data->intermediateTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->intermediateTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG("FAILED  TO CREATE FRAMEBUFFER\n");
	}
}
void RE_CleanUpPostProcessingRenderData(PostProcessingRenderData* data)
{
	if (data->intermediateTexture) glDeleteTextures(1, &data->intermediateTexture);
	if (data->intermediateFbo) glDeleteFramebuffers(1, &data->intermediateFbo);
	if (data->ppTexture1) glDeleteTextures(1, &data->ppTexture1);
	if (data->ppTexture2) glDeleteTextures(1, &data->ppTexture2);
	if (data->ppFBOs1)
	{
		glDeleteFramebuffers(data->numPPFbos, data->ppFBOs1);
		delete[] data->ppFBOs1;
	}
	if (data->ppFBOs2)
	{
		glDeleteFramebuffers(data->numPPFbos, data->ppFBOs2);
		delete[] data->ppFBOs2;
	}
	data->numPPFbos = 0;
	data->ppFBOs1 = nullptr;
	data->ppFBOs2 = nullptr;
	data->ppTexture1 = 0;
	data->ppTexture2 = 0;
	data->intermediateTexture = 0;
}




void RE_RenderScene(struct Renderer* renderer, const glm::mat4& viewProj, Scene* scene)
{
	size_t num = 0;
	Renderable** renderables =SC_GetAllRenderables(scene, &num);
	if (num == 0) return;
	std::sort(renderables, renderables + num, [](Renderable* r1, Renderable* r2) {
		if (!(r1->GetFlags() & RENDERABLE_FLAGS::INACTIVE) && !(r2->GetFlags() & RENDERABLE_FLAGS::INACTIVE)) {
			return r1->GetLayer() < r2->GetLayer();
		}
		else
		{
			return (!(r1->GetFlags() & RENDERABLE_FLAGS::INACTIVE) && (r2->GetFlags() & RENDERABLE_FLAGS::INACTIVE));
		}
	});

	int curLayer = renderables[0]->GetLayer();
	
	renderer->renderContext.cmds.clear();
	renderer->renderContext.vertices.clear();
	renderer->renderContext.indices.clear();

	for (uint32_t i = 0; i < num; i++)
	{
		if (renderables[i]->GetFlags() & RENDERABLE_FLAGS::INACTIVE) break;

		renderables[i]->Draw((RenderContext2D*)&renderer->renderContext);
	}

	glBindVertexArray(renderer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->indexBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * renderer->renderContext.vertices.size(), renderer->renderContext.vertices.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * renderer->renderContext.indices.size(), renderer->renderContext.indices.data(), GL_DYNAMIC_DRAW);

	glUseProgram(renderer->info.program);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniformMatrix4fv(renderer->info.viewProjLoc, 1, GL_FALSE, (GLfloat*)&viewProj);
	glActiveTexture(GL_TEXTURE0);

	for (uint32_t i = 0; i < renderer->renderContext.cmds.size(); i++)
	{
		RenderCommand& cmd = renderer->renderContext.cmds.at(i);
		if(cmd.texture) glBindTexture(GL_TEXTURE_2D, cmd.texture);
		else glBindTexture(GL_TEXTURE_2D, renderer->whiteTexture);

		glDrawElements(GL_TRIANGLES, cmd.indCount, GL_UNSIGNED_INT, (const void*)(cmd.firstInd * sizeof(uint32_t)));
	}

}


void RE_RenderPostProcessingBloom(struct Renderer* renderer, const PostProcessingRenderData* ppData, GLuint srcTexture, uint32_t srcWidth, uint32_t srcHeight, GLuint targetFBO, uint32_t targetWidth, uint32_t targetHeight)
{
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glm::ivec2 fboSizes[MAX_BLOOM_MIPMAPS];
	{	// Bloom src image into the pptexture1
		glBindFramebuffer(GL_FRAMEBUFFER, ppData->ppFBOs2[0]);
		glViewport(0, 0, ppData->width, ppData->height);
		glUseProgram(renderer->ppInfo.bloom.program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, srcTexture);


		glUniform1f(renderer->ppInfo.bloom.blurRadiusLoc, ppData->blurRadius);
		glUniform1i(renderer->ppInfo.bloom.blurAxisLoc, (int)PostProcessingRenderInfo::BLUR_AXIS::X_AXIS);
		glUniform1f(renderer->ppInfo.bloom.intensityLoc, ppData->bloomIntensity);
		glUniform1f(renderer->ppInfo.bloom.mipLevelLoc, 0);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindFramebuffer(GL_FRAMEBUFFER, ppData->ppFBOs1[0]);
		glViewport(0, 0, ppData->width, ppData->height);

		glBindTexture(GL_TEXTURE_2D, ppData->ppTexture2);

		glUniform1f(renderer->ppInfo.bloom.intensityLoc, 0.0f);
		glUniform1i(renderer->ppInfo.bloom.blurAxisLoc, (int)PostProcessingRenderInfo::BLUR_AXIS::Y_AXIS);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	fboSizes[0] = { ppData->width, ppData->height };
	int curSizeX = ppData->width; int curSizeY = ppData->height;
	{	// BLUR AND DOWNSAMPLE EACH ITERATION
		for (int i = 1; i < ppData->numPPFbos; i++)
		{
			curSizeX = glm::max(curSizeX >> 1, 1); curSizeY = glm::max(curSizeY >> 1, 1);
			fboSizes[i] = { curSizeX, curSizeY };
			glBindFramebuffer(GL_FRAMEBUFFER, ppData->ppFBOs2[i]);
			glViewport(0, 0, curSizeX, curSizeY);

			glBindTexture(GL_TEXTURE_2D, ppData->ppTexture1);

			glUniform1f(renderer->ppInfo.bloom.blurRadiusLoc, ppData->blurRadius);
			glUniform1i(renderer->ppInfo.bloom.blurAxisLoc, (int)PostProcessingRenderInfo::BLUR_AXIS::X_AXIS);
			glUniform1f(renderer->ppInfo.bloom.intensityLoc, 0.0f);
			glUniform1f(renderer->ppInfo.bloom.mipLevelLoc, (GLfloat)(i - 1));

			glDrawArrays(GL_TRIANGLES, 0, 3);

			glBindFramebuffer(GL_FRAMEBUFFER, ppData->ppFBOs1[i]);

			glBindTexture(GL_TEXTURE_2D, ppData->ppTexture2);

			glUniform1f(renderer->ppInfo.bloom.blurRadiusLoc, ppData->blurRadius / 2.0f);
			glUniform1f(renderer->ppInfo.bloom.intensityLoc, 0.0f);
			glUniform1f(renderer->ppInfo.bloom.mipLevelLoc, (GLfloat)i);
			glUniform1i(renderer->ppInfo.bloom.blurAxisLoc, (int)PostProcessingRenderInfo::BLUR_AXIS::Y_AXIS);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
	{	// UPSAMPLE FROM LOWER MIPMAP TO HIGHER
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glUseProgram(renderer->ppInfo.upsampling.program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ppData->ppTexture1);
		for (int i = ppData->numPPFbos - 2; i >= 0; i--)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ppData->ppFBOs1[i]);
			glViewport(0, 0, fboSizes[i].x, fboSizes[i].y);
			glUniform1f(renderer->ppInfo.upsampling.mipLevelLoc, (GLfloat)(i + 1));
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}

	{	// COPY FINAL IMAGE
		glDisable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
		glViewport(0, 0, targetWidth, targetHeight);
		glUseProgram(renderer->ppInfo.dualCopy.program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ppData->ppTexture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, srcTexture);

		glUniform1f(renderer->ppInfo.dualCopy.mipLevel1Loc, 0);
		glUniform1f(renderer->ppInfo.dualCopy.mipLevel2Loc, 0);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}





void RenderContext2D::Draw(Vertex2D* vtx, uint32_t numVerts, uint32_t* inds, uint32_t numInds, GLuint texture)
{
	((InternalRenderContext2D*)this)->Draw(vtx, numVerts, inds, numInds, texture);
}

void RenderContext2D::DrawQuad(const glm::vec2& tl, const glm::vec2& br, const glm::vec2& uvTL, const glm::vec2& uvBR, const glm::vec4& col, GLuint texture, float angle)
{
	((InternalRenderContext2D*)this)->DrawQuad(tl, br, uvTL, uvBR, col, texture, angle);
}
