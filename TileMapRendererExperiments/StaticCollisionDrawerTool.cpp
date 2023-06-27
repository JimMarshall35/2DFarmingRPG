#include "StaticCollisionDrawerTool.h"
#include "NewRenderer.h"
#include "PhysicsWorld.h"
#include "ECS.h"
#include "Rect.h"
#include "Camera2D.h"
#include "QuadTree.h"
#include "StaticPhysicsBody.h"
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>


StaticCollisionDrawerTool::StaticCollisionDrawerTool(NewRenderer* newRenderer, DynamicQuadTreeContainer<flecs::entity>* entityQuadTree, PhysicsWorld* physicsWorld, ECS* ecs) 
    :m_newRenderer(newRenderer),
    m_entityQuadTree(entityQuadTree),
    m_physicsWorld(physicsWorld),
    m_ecs(ecs)
{

}
void StaticCollisionDrawerTool::DoUi()
{
}

void StaticCollisionDrawerTool::RecieveTileClick(i32 x, i32 y, i32 z)
{
}

void StaticCollisionDrawerTool::TileSelectionChanged(u32 newTile)
{
}

const std::string& StaticCollisionDrawerTool::GetName()
{
    // TODO: insert return statement here
    static std::string name = "Static Collision Tool";
    return name;
}

void StaticCollisionDrawerTool::RecieveWorldspaceClick(const glm::vec2& worldspace)
{
    m_points.push_back(worldspace);
}

void StaticCollisionDrawerTool::RecieveWorldspaceRightClick(const glm::vec2& worldspace)
{
    m_points.push_back(worldspace);

    auto entity = m_ecs->GetWorld()->entity();

    m_physicsWorld->AddStaticPolygon(m_points.data(), m_points.size(), entity);
    m_points.clear();
}

bool StaticCollisionDrawerTool::WantsToDrawOverlay() const
{
    return true;
}

void StaticCollisionDrawerTool::DrawOverlay(const Camera2D& camera, const glm::vec2& mouseWorldSpacePos) const
{
    u32 numPoints = m_points.size();
    if (numPoints > 0) {
        for (u32 i = 0; i < numPoints; i++) {
            glm::vec2 pt1;
            glm::vec2 pt2;
            pt1 = m_points[i];
            pt2 = (i == (numPoints - 1)) ? mouseWorldSpacePos : m_points[i + 1];

            m_newRenderer->DrawLine(pt1, pt2, { 0,1,0,1.0 }, 3, camera);
        }
    }
    auto camTLBR = camera.GetTLBR();
    Rect r;
    r.pos.x = camTLBR.y;
    r.pos.y = camTLBR.x;

    r.dims.x = camTLBR.w - camTLBR.y;
    r.dims.y = camTLBR.z - camTLBR.x;
    auto vis = m_entityQuadTree->search(r);
    for (const auto& qItem : vis) {

        auto entity = qItem->item;
        const StaticPhysicsBody* physicsBody = entity.get<StaticPhysicsBody>();
        if (physicsBody) {
            const b2Shape* shape = physicsBody->fixture->GetShape();
            if (shape->GetType() == b2Shape::e_polygon) {
                const b2PolygonShape* polygonShape = static_cast<const b2PolygonShape*>(shape);
                for (int i = 0; i < polygonShape->m_count; i++) {
                    if (i + 1 < polygonShape->m_count) {
                        glm::vec2 pt1 = { polygonShape->m_vertices[i].x, polygonShape->m_vertices[i].y };
                        glm::vec2 pt2 = { polygonShape->m_vertices[i+1].x, polygonShape->m_vertices[i+1].y };
                        m_newRenderer->DrawLine(pt1, pt2, { 1.0,0.0,0.0,1.0 }, 3, camera);
                    }
                }
            }
        }
       
    }
}
