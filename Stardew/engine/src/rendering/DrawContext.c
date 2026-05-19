#include "DrawContext.h"
#include "Platform.h"
#include "PlatformDefs.h"
#include <string.h>
#include <glad/glad.h>

#include <cglm/cglm.h>
#include "ObjectPool.h"
#include "Widget.h"
#include "DynArray.h"
#include "AssertLib.h"
#include "PlatformDefs.h"
#include "Game2DLayer.h"
#include "Log.h"


const char* lineVert =
#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec4 aColour;\n"
"out vec4 Colour;\n"
"uniform mat4 vp;\n"
"void main()\n"
"{\n"
	"gl_Position = vp * vec4(aPos, 0.0, 1.0);\n"
	"Colour = aColour;"
"}\n"
#elif GAME_GL_API_TYPE == GAME_GL_API_TYPE_ES
"#version 300 es\n"
"in vec2 aPos;\n"
"in vec4 aColour;\n"
"out vec4 Colour;\n"
"uniform mat4 vp;\n"
"void main()\n"
"{\n"
	"gl_Position = vp * vec4(aPos, 0.0, 1.0);\n"
	"Colour = aColour;"
"}\n"
#endif
;

const char* lineFrag =
#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec4 Colour;\n"

"void main()\n"
"{\n"
	"FragColor = Colour;\n"
"}\n"
#elif GAME_GL_API_TYPE == GAME_GL_API_TYPE_ES
"#version 300 es\n"
"precision highp float;\n"
"out vec4 FragColor;\n"
"in vec4 Colour;\n"

"void main()\n"
"{\n"
	"FragColor = Colour;\n"
"}\n"
#endif
;

const char* uiVert =
#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec2 aUv;\n"
"layout (location = 2) in vec4 aColour;\n"
"out vec2 UV;\n"
"out vec4 Colour;\n"
"uniform mat4 screenToClipMatrix;\n"
"void main()\n"
"{\n"
	"gl_Position = screenToClipMatrix * vec4(aPos, 0.0, 1.0);\n"
	"UV = aUv;\n"
	"Colour = aColour;"
"}\n"
#elif GAME_GL_API_TYPE == GAME_GL_API_TYPE_ES
"#version 300 es\n"
"in vec2 aPos;\n"
"in vec2 aUv;\n"
"in vec4 aColour;\n"
"out vec2 UV;\n"
"out vec4 Colour;\n"
"uniform mat4 screenToClipMatrix;\n"
"void main()\n"
"{\n"
	"gl_Position = screenToClipMatrix * vec4(aPos, 0.0, 1.0);\n"
	"UV = aUv;\n"
	"Colour = aColour;"
"}\n"
#endif
;

const char* uiFrag =
#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec4 Colour;\n"
"in vec2 UV;\n"

"uniform sampler2D ourTexture;\n"

"void main()\n"
"{\n"
	"FragColor = texture(ourTexture, UV) * Colour;\n"
"}\n"
#elif GAME_GL_API_TYPE == GAME_GL_API_TYPE_ES
"#version 300 es\n"
"precision highp float;\n"
"out vec4 FragColor;\n"
"in vec4 Colour;\n"
"in vec2 UV;\n"

"uniform sampler2D ourTexture;\n"

"void main()\n"
"{\n"
	"FragColor = texture(ourTexture, UV) * Colour;\n"
"}\n"
#endif
;

const char* worldspaceVert =
"#version 300 es\n"
"in vec2 aPos;\n"
"in vec2 aUv;\n"
"out vec2 UV;\n"
"uniform mat4 vp;\n"
"void main()\n"
"{\n"
	"gl_Position = vp * vec4(aPos, 0.0, 1.0);\n"
	"UV = aUv;\n"
"}\n"
;

const char* worldspaceFrag =
"#version 300 es\n"
"precision mediump float;\n"
"out vec4 FragColor;\n"
"in vec2 UV;\n"

"uniform sampler2D ourTexture;\n"

"void main()\n"
"{\n"
	"FragColor = texture(ourTexture, UV);\n"
"}\n"
;

//
//const char* tilemapVert = 
//"#version 330 core\n"

struct IndexedVertexBuffer
{
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	int capacity;
	int eboCapacity;

};


struct VertexBuffer
{
	GLuint vao;
	GLuint vbo;
	int capacity;
};

enum ShaderType
{
	ST_Vertex,
	ST_Fragment,
	ST_Program,
	ST_NUM
};

static const char* gShaderTypeNameLUT[ST_NUM] =
{
	"Vertex",
	"Fragment",
	"Program"
};

struct Shader
{
	GLuint program;
	GLuint frag;
	GLuint vert;
};

struct Shader gWorldspaceLineShader = {0,0,0};

struct Shader gUIShader = {0,0,0};

struct Shader gWorldspace2DShader = { 0,0,0 };

mat4 gScreenspaceOrtho;

OBJECT_POOL(struct VertexBuffer) gLineVertexPool = NULL;

OBJECT_POOL(struct VertexBuffer) gVertexBuffersPool = NULL;

OBJECT_POOL(struct IndexedVertexBuffer) gIndexedVertexBuffersPool = NULL;

static bool OpenGlGPULoadTexture(const unsigned char* data, unsigned int width, unsigned int height, unsigned int* id)
{
	glGenTextures(1, id);
	glBindTexture(GL_TEXTURE_2D, *id);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); // GL_NEAREST is the better filtering option for this game
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // as gives better more "pixelated" (less "smoothed out") textures
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	return true;
}

static void TestShaderStatus(GLuint shader, enum ShaderType type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != ST_Program)
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			Log_Error("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- \n", 
				gShaderTypeNameLUT[type],
				infoLog);
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			Log_Error("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- \n",
				gShaderTypeNameLUT[type],
				infoLog);
		}
	}
}

static void CreateShader(const char* vert, const char* frag, struct Shader* pShader)
{
	int success = GL_FALSE;

	pShader->vert = glCreateShader(GL_VERTEX_SHADER);
	const char* v = vert;
	const char* f = frag;


	glShaderSource(pShader->vert, 1, (const char**)&v, 0);
	glCompileShader(pShader->vert);
	TestShaderStatus(pShader->vert, ST_Vertex);

	pShader->frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(pShader->frag, 1, (const char**)&f, 0);
	glCompileShader(pShader->frag);
	TestShaderStatus(pShader->frag, ST_Fragment);

	pShader->program = glCreateProgram();
	glAttachShader(pShader->program, pShader->vert);
	glAttachShader(pShader->program, pShader->frag);
	glLinkProgram(pShader->program);

	TestShaderStatus(pShader->program, ST_Program);

}

static void CreateShaders()
{
	CreateShader(lineVert, lineFrag, &gWorldspaceLineShader);
	CreateShader(uiVert, uiFrag, &gUIShader);
	CreateShader(worldspaceVert, worldspaceFrag, &gWorldspace2DShader);
};

////////////////////////////////////////////////////////////////////////////////////////////////////// Worldspace lines

static HWorldspaceLineVertexBuffer NewWorldspaceLineBuffer(int size)
{
	HWorldspaceLineVertexBuffer buf = -1;
	gLineVertexPool = GetObjectPoolIndex(gLineVertexPool, &buf);
	struct VertexBuffer* pBuf = &gLineVertexPool[buf];
	pBuf->capacity = 0;
	glGenVertexArrays(1, &pBuf->vao);
	glBindVertexArray(pBuf->vao);

	glGenBuffers(1, &pBuf->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pBuf->vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(WorldspaceLineVertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(WorldspaceLineVertex), (void*)(sizeof(float) * 2));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return buf;
}

static void WorldspaceLineBufferData(HWorldspaceLineVertexBuffer hBuf, WorldspaceLineVertex* src, size_t size)
{
	struct VertexBuffer* pBuf = &gLineVertexPool[hBuf];

	glBindBuffer(GL_ARRAY_BUFFER, pBuf->vbo);
	if (size * sizeof(WorldspaceLineVertex) > pBuf->capacity)
	{
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(WorldspaceLineVertex), src, GL_DYNAMIC_DRAW);
		pBuf->capacity = size * sizeof(WorldspaceLineVertex);
	}
	else
	{
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(WorldspaceLineVertex), src);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void DrawWorldspaceLineVertexBuffer(HWorldspaceLineVertexBuffer hBuf, size_t vertexCount, mat4 view)
{
	const struct VertexBuffer* vertexBuffer = &gLineVertexPool[hBuf];

	glUseProgram(gWorldspaceLineShader.program);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vbo);
	glBindVertexArray(vertexBuffer->vao);
	
	unsigned int projectionViewUniform = glGetUniformLocation(gWorldspaceLineShader.program, "vp");
	
	mat4 m;
	glm_mat4_mul(&gScreenspaceOrtho[0], &view[0], &m[0]);

	glUniformMatrix4fv(projectionViewUniform, 1, false, &m[0][0]);

	glDrawArrays(GL_LINES, 0, vertexCount);
}
/*
	const struct IndexedVertexBuffer* vertexBuffer = &gIndexedVertexBuffersPool[hBuf];
	glUseProgram(gWorldspace2DShader.program);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->ebo);
	glBindVertexArray(vertexBuffer->vao);
	unsigned int projectionViewUniform = glGetUniformLocation(gWorldspace2DShader.program, "vp");
	mat4 m;
	glm_mat4_mul(&gScreenspaceOrtho[0], &view[0], &m[0]);
	glUniformMatrix4fv(projectionViewUniform, 1, false, &m[0][0]);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
*/
static void DestroyWorldspaceLineVertexBuffer(HWorldspaceLineVertexBuffer hBuf)
{
	const struct VertexBuffer* vertexBuffer = &gLineVertexPool[hBuf];
	glDeleteBuffers(1, &vertexBuffer->vbo);
	glDeleteVertexArrays(1, &vertexBuffer->vao);
}

////////////////////////////////////////////////////////////////////////////////////////////////////// UI (screen space) Sprites

static HUIVertexBuffer NewUIVertexBuffer(int size)
{
	HUIVertexBuffer buf = -1;
	gVertexBuffersPool = GetObjectPoolIndex(gVertexBuffersPool, &buf);
	struct VertexBuffer* pBuf = &gVertexBuffersPool[buf];
	pBuf->capacity = 0;
	glGenVertexArrays(1, &pBuf->vao);
	glBindVertexArray(pBuf->vao);

	glGenBuffers(1, &pBuf->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pBuf->vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(WidgetVertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(WidgetVertex), (void*)(sizeof(float) * 2));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(WidgetVertex), (void*)(sizeof(float) * 4));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return buf;
}

static void UIVertexBufferData(HUIVertexBuffer hBuf, WidgetVertex* src, size_t size)
{
	struct VertexBuffer* pBuf = &gVertexBuffersPool[hBuf];

	glBindBuffer(GL_ARRAY_BUFFER, pBuf->vbo);
	if (size * sizeof(WidgetVertex) > pBuf->capacity)
	{
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(WidgetVertex), src, GL_DYNAMIC_DRAW);
		pBuf->capacity = size * sizeof(WidgetVertex);
	}
	else
	{
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(WidgetVertex), src);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void DrawUIVertexBuffer(HUIVertexBuffer hBuf, size_t vertexCount)
{
	const struct VertexBuffer* vertexBuffer = &gVertexBuffersPool[hBuf];

	glUseProgram(gUIShader.program);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vbo);
	glBindVertexArray(vertexBuffer->vao);
	
	unsigned int projectionViewUniform = glGetUniformLocation(gUIShader.program, "screenToClipMatrix");
	glUniformMatrix4fv(projectionViewUniform, 1, false, &gScreenspaceOrtho[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

static void DestroyUIVertexBuffer(HUIVertexBuffer hBuf)
{
	const struct VertexBuffer* vertexBuffer = &gVertexBuffersPool[hBuf];
	glDeleteBuffers(1, &vertexBuffer->vbo);
	glDeleteVertexArrays(1, &vertexBuffer->vao);
}

////////////////////////////////////////////////////////////////////////////////////////////////////// textures


static hTexture UploadTexture(void* src, int channels, int pxWidth, int pxHeight)
{
	EASSERT(channels == 4);
	hTexture txture = 0;
	OpenGlGPULoadTexture(src, pxWidth, pxHeight, &txture);
	return txture;
}

static void SetCurrentAtlas(hTexture atlas)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas);
}

static void DestroyTexture(hTexture tex)
{
	glDeleteTextures(1, &tex);
}


////////////////////////////////////////////////////////////////////////////////////////////////////// worldspace sprites

static HWorldspaceVertexBuffer NewWorldspaceVertexBuffer(int size)
{
	HWorldspaceVertexBuffer buf = -1;
	gVertexBuffersPool = GetObjectPoolIndex(gIndexedVertexBuffersPool, &buf);
	struct IndexedVertexBuffer* pBuf = &gIndexedVertexBuffersPool[buf];
	pBuf->capacity = 0;
	pBuf->eboCapacity = 0;
	glGenVertexArrays(1, &pBuf->vao);
	glBindVertexArray(pBuf->vao);

	glGenBuffers(1, &pBuf->ebo);
	glGenBuffers(1, &pBuf->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pBuf->ebo);
	glBindBuffer(GL_ARRAY_BUFFER, pBuf->vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Worldspace2DVert), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Worldspace2DVert), (void*)(sizeof(float) * 2));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return buf;
}

void WorldspaceVertexBufferData(HUIVertexBuffer hBuf, Worldspace2DVert* src, size_t size, VertIndexT* indices, u32 numIndices)
{
	struct IndexedVertexBuffer* pBuf = &gIndexedVertexBuffersPool[hBuf];

	glBindBuffer(GL_ARRAY_BUFFER, pBuf->vbo);
	if (size * sizeof(Worldspace2DVert) > pBuf->capacity)
	{
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(Worldspace2DVert), src, GL_DYNAMIC_DRAW);
		pBuf->capacity = size;
	}
	else
	{
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(Worldspace2DVert), src);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pBuf->ebo);
	if (numIndices * sizeof(VertIndexT) > pBuf->eboCapacity)
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);
		pBuf->eboCapacity = numIndices * sizeof(VertIndexT);
	}
	else
	{
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size * sizeof(VertIndexT), indices);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void DrawWorldspaceVertexBuffer(H2DWorldspaceVertexBuffer hBuf, size_t indexCount, mat4 view)
{
	const struct IndexedVertexBuffer* vertexBuffer = &gIndexedVertexBuffersPool[hBuf];
	glUseProgram(gWorldspace2DShader.program);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->ebo);
	glBindVertexArray(vertexBuffer->vao);
	unsigned int projectionViewUniform = glGetUniformLocation(gWorldspace2DShader.program, "vp");
	mat4 m;
	glm_mat4_mul(&gScreenspaceOrtho[0], &view[0], &m[0]);
	glUniformMatrix4fv(projectionViewUniform, 1, false, &m[0][0]);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
}

void DestroyWorldspaceVertexBuffer(H2DWorldspaceVertexBuffer hBuf)
{
	const struct IndexedVertexBuffer* vertexBuffer = &gIndexedVertexBuffersPool[hBuf];
	glDeleteBuffers(1, &vertexBuffer->vbo);
	glDeleteBuffers(1, &vertexBuffer->ebo);
	glDeleteVertexArrays(1, &vertexBuffer->vao);
}

#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE

// static void GLAPIENTRY MessageCallback(GLenum source,
//     GLenum type,
//     GLuint id,
//     GLenum severity,
//     GLsizei length,
//     const GLchar* message,
//     const void* userParam)
// {
//     //if (severity >= minimumLogSeverityIncluding) 
//     {
//         Log_Warning("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n\n",
//             (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
//             type, severity, message);
//     }
// }

#endif

static void ClearScreen()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
} 

static void SetLineWidth(float w)
{
	glLineWidth(w);
}

DrawContext Dr_InitDrawContext()
{
	// configure global opengl state
    // -----------------------------
    Log_Verbose("configuring global opengl state");
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


#if GAME_GL_API_TYPE == GAME_GL_API_TYPE_CORE
    // During init, enable debug output
    //glEnable(GL_DEBUG_OUTPUT);

    //glDebugMessageCallback(MessageCallback, 0);
#endif

	DrawContext d;
	memset(&d, 0, sizeof(DrawContext));

	d.NewWorldspaceLineBuffer = &NewWorldspaceLineBuffer;
	d.WorldspaceLineBufferData = &WorldspaceLineBufferData;
	d.DrawWorldspaceLineVertexBuffer = &DrawWorldspaceLineVertexBuffer;
	d.DestroyWorldspaceLineVertexBuffer = &DestroyWorldspaceLineVertexBuffer;

	d.DestroyVertexBuffer = &DestroyUIVertexBuffer;
	d.DrawUIVertexBuffer = &DrawUIVertexBuffer;
	d.NewUIVertexBuffer = &NewUIVertexBuffer;
	d.UIVertexBufferData = &UIVertexBufferData;

	d.SetCurrentAtlas = &SetCurrentAtlas;
	d.UploadTexture = &UploadTexture;

	d.NewWorldspaceVertBuffer = &NewWorldspaceVertexBuffer;
	d.WorldspaceVertexBufferData = &WorldspaceVertexBufferData;
	d.DrawWorldspaceVertexBuffer = &DrawWorldspaceVertexBuffer;
	d.DestroyWorldspaceVertexBuffer = &DestroyWorldspaceVertexBuffer;

	d.ClearScreen = &ClearScreen;
	d.SetLineWidth = &SetLineWidth;

	gVertexBuffersPool = NEW_OBJECT_POOL(struct VertexBuffer, 256);
	gIndexedVertexBuffersPool = NEW_OBJECT_POOL(struct IndexedVertexBuffer, 256);
	gLineVertexPool = NEW_OBJECT_POOL(struct LineVertex, 256);
	glm_mat4_identity(gScreenspaceOrtho);
	CreateShaders();
	return d;
}

void Dr_OnScreenDimsChange(DrawContext* pCtx, int newW, int newH)
{
	pCtx->screenWidth = newW;
	pCtx->screenHeight = newH;
	glViewport(0, 0, newW, newH);
	glm_ortho(0.0f, newW, newH, 0.0f, -1.0f, 1.0f, gScreenspaceOrtho);
}
