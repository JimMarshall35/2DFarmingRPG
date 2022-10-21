#include "NewRenderer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Camera2D.h"
#include "TiledWorld.h"

#define TILE_NUM_INDICES 6


NewRenderer::NewRenderer(const NewRendererInitialisationInfo& info)
	:m_tileShader("shaders\\TilemapVert2.glsl", "shaders\\TilemapFrag2.glsl"),
	m_tilemapChunkSize(info.chunkSizeX, info.chunkSizeY),
	m_windowWidth(info.windowWidth),
	m_windowHeight(info.windowHeight)
{
	glGenVertexArrays(1, &m_vao);
}

void NewRenderer::DrawChunk(
	const glm::ivec2& chunkWorldMapOffset,
	const glm::vec2& pos,
	const glm::vec2& scale,
	float rotation,
	const TiledWorld& world,
	ArrayTexture2DHandle texArray,
	const Camera2D& cam
) const
{
	m_tileShader.use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos, 0.0f));
	model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(scale, 1.0f));

	m_tileShader.setMat4("vpMatrix", cam.GetProjectionMatrix(m_windowWidth, m_windowHeight));
	m_tileShader.setMat4("modelMatrix", model);
	m_tileShader.SetIVec2("chunkOffset", chunkWorldMapOffset);
	m_tileShader.SetIVec2("chunkSize", m_tilemapChunkSize);
	m_tileShader.setInt("masterTileTexture", 0);
	m_tileShader.setInt("atlasSampler", 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);
	glBindVertexArray(m_vao);

	world.IterateTileLayers([this](const TextureHandle& texture, u16* data) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glDrawArrays(GL_TRIANGLES, 0, m_tilemapChunkSize.x * m_tilemapChunkSize.y * TILE_NUM_INDICES);
	});

}

void NewRenderer::SetWindowWidthAndHeight(u32 width, u32 height)
{
	m_windowWidth = width;
	m_windowHeight = height;
}
