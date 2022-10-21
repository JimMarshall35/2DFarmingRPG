#include "Camera2D.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/transform.hpp>

glm::mat4x4 Camera2D::GetProjectionMatrix(int windowWidth, int windowHeight) const
{
    float left = FocusPosition.x - ((float)windowWidth / 2.0f);
    float right = FocusPosition.x + ((float)windowWidth / 2.0f);
    float top = FocusPosition.y - ((float)windowHeight / 2.0f);
    float bottom = FocusPosition.y + ((float)windowHeight / 2.0f);
    auto proj_matrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    auto zoom_matrix = glm::scale(glm::vec3(Zoom));
    return zoom_matrix * proj_matrix;
}

glm::vec4 Camera2D::GetTLBR(int _ScreenW, int _ScreenH) const
{
    glm::vec4 tlbr;
    float halfWindowW = ((float)_ScreenW / 2.0f) / Zoom;
    float halfWindowH = ((float)_ScreenH / 2.0f) / Zoom;
    tlbr[0] = FocusPosition.y - halfWindowH;
    tlbr[1] = FocusPosition.x - halfWindowW;
    tlbr[2] = FocusPosition.y + halfWindowH;
    tlbr[3] = FocusPosition.x + halfWindowW;


    return tlbr;
}

glm::vec2 Camera2D::MouseScreenPosToWorld(double mousex, double mousey, unsigned int windowW, unsigned int windowH) const
{
    glm::vec2 norm = glm::vec2(mousex / (double)windowW, mousey / (double)windowH);
    glm::vec4 tlbr = GetTLBR(windowW, windowH);
    float view_w = tlbr[3] - tlbr[1];
    float view_h = tlbr[2] - tlbr[0];
    return glm::vec2(
        tlbr[1] + norm.x * view_w,
        tlbr[0] + norm.y * view_h
    );
}
