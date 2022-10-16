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
	gl_Position = vec4(pos, 0.0f, 1.0f);\
}";
static const char* stdFragmentShader = "#version 330\n\
in vec2 fragUV;\
in vec4 fragCol;\
out vec4 outCol;\
uniform sampler2D baseTexture;\
void main()\
{\
	outCol = texture(baseTexture, fragUV) * fragCol;\
}";



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

	std::vector<Vertex2D> vertexList;
	std::vector<uint32_t> indList;

};

struct Renderer* RE_CreateRenderer()
{
	Renderer* out = new Renderer;
	memset(out, 0, sizeof(Renderer));


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
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, col));

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


	data->numPPFbos = 1 + floor(log2(glm::max(data->width, data->height)));
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




void RE_RenderScene(struct Renderer* renderer, const glm::mat4& viewProj, SceneObject** objs, uint32_t numObjs)
{
	std::sort(objs, objs + numObjs, [](SceneObject* o1, SceneObject* o2) {
		if (o1->flags & OBJECT_FLAG_VISIBLE && o1->renderable && o2->renderable) {
			return o1->renderable->layer < o2->renderable->layer;
		}
		else
		{
			return (o2->flags & OBJECT_FLAG_VISIBLE && o2->renderable);
		}
	});
	for (uint32_t i = 0; i < numObjs; i++)
	{
		if (!(objs[i]->flags & OBJECT_FLAG_VISIBLE) || !objs[i]->renderable) break;
		if (objs[i]->body) objs[i]->renderable->UpdateFromBody(objs[i]->body);
		objs[i]->renderable->AddVertices(renderer->vertexList, renderer->indList);
	}

	glBindVertexArray(renderer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->indexBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * renderer->vertexList.size(), renderer->vertexList.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * renderer->indList.size(), renderer->indList.data(), GL_DYNAMIC_DRAW);

	glUseProgram(renderer->info.program);
	glUniformMatrix4fv(renderer->info.viewProjLoc, 1, GL_FALSE, (GLfloat*)&viewProj);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->whiteTexture);

	glDrawElements(GL_TRIANGLES, renderer->indList.size(), GL_UNSIGNED_INT, nullptr);

	renderer->indList.clear();
	renderer->vertexList.clear();
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
			glUseProgram(renderer->ppInfo.bloom.program);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ppData->ppTexture1);

			glUniform1f(renderer->ppInfo.bloom.blurRadiusLoc, ppData->blurRadius);
			glUniform1i(renderer->ppInfo.bloom.blurAxisLoc, (int)PostProcessingRenderInfo::BLUR_AXIS::X_AXIS);
			glUniform1f(renderer->ppInfo.bloom.intensityLoc, 0.0f);
			glUniform1f(renderer->ppInfo.bloom.mipLevelLoc, i - 1);

			glDrawArrays(GL_TRIANGLES, 0, 3);

			glBindFramebuffer(GL_FRAMEBUFFER, ppData->ppFBOs1[i]);

			glBindTexture(GL_TEXTURE_2D, ppData->ppTexture2);

			glUniform1f(renderer->ppInfo.bloom.blurRadiusLoc, ppData->blurRadius / 2.0f);
			glUniform1f(renderer->ppInfo.bloom.intensityLoc, 0.0f);
			glUniform1f(renderer->ppInfo.bloom.mipLevelLoc, i);
			glUniform1i(renderer->ppInfo.bloom.blurAxisLoc, (int)PostProcessingRenderInfo::BLUR_AXIS::Y_AXIS);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
	{	// UPSAMPLE FROM LOWER MIPMAP TO HIGHER
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		for (int i = ppData->numPPFbos - 2; i >= 0; i--)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ppData->ppFBOs1[i]);
			glViewport(0, 0, fboSizes[i].x, fboSizes[i].y);
			glUseProgram(renderer->ppInfo.upsampling.program);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ppData->ppTexture1);
			glUniform1f(renderer->ppInfo.upsampling.mipLevelLoc, i + 1);
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