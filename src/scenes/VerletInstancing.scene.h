#pragma once
#include "Scene.h"
#include "../layer/Instancing/RendererInstanced.h"
#include <iostream>
#define PI glm::pi<float>()

class RigidBody {
public:
    glm::vec2 pos_old;
    glm::vec2* pos;
    glm::vec2 acceleration;
    float bouncines;

    void updatePosition(float dt) {
        const glm::vec2 deltaPos = *pos - pos_old;
        pos_old = *pos;
        *pos = *pos + deltaPos + acceleration*dt*dt;
        acceleration = {};
    }

    void accelerate(glm::vec2 acc) {
        acceleration += acc;
    }

    void velocity(glm::vec2 vel) {
        pos_old = *pos - vel;
    }
};

class Constraint {
public:
    glm::vec2 topLeft;
    glm::vec2 bottomRight;
    glm::vec2 centre;

    Constraint(glm::vec2 topLeft, glm::vec2 bottomRight)
        : topLeft(topLeft), bottomRight(bottomRight), centre((bottomRight.x + topLeft.x)/2, ((bottomRight.y + topLeft.y)/2))
    { }
    
    void ApplyCircle(RigidBody& rb, glm::vec2 scale) {
        glm::vec2 displace = -centre + *rb.pos;
        float theta = atan(displace.y/displace.x);
        float sqrtDist = (displace.x * displace.x) + (displace.y * displace.y);
        float radius = HEIGHT/2 - scale.x;
        if (displace.x < 0) radius *= -1;
        glm::vec2 vel = *rb.pos - rb.pos_old;

        if (sqrtDist > radius*radius) {
            *rb.pos = centre + radius * glm::vec2(glm::cos(theta), glm::sin(theta));
            displace = glm::normalize(displace);
            vel -= (rb.bouncines + 1) * displace * glm::dot(vel, displace);
            rb.velocity(vel);
        }
    }
    void ApplyRect(RigidBody& rb, glm::vec2 scale) {
        glm::vec2& pos = *rb.pos;
        glm::vec2 vel = pos - rb.pos_old;
        
        if (pos.x > bottomRight.x - scale.x) { // right
            vel = pos - rb.pos_old;
            vel.x *= -1;
            pos.x = bottomRight.x - scale.x; 
            rb.velocity(vel * rb.bouncines);
        }
        else if (pos.x < topLeft.x + scale.x) { // left
            vel = pos - rb.pos_old;
            vel.x *= -1;
            pos.x = topLeft.x + scale.x;
            rb.velocity(vel * rb.bouncines);
        }
        
        if (pos.y > topLeft.y - scale.y) {      // top
            vel = pos - rb.pos_old;
            vel.y *= -1;
            pos.y = topLeft.y - scale.y;
            rb.velocity(vel * rb.bouncines);
        }
        else if (pos.y < bottomRight.y + scale.y) { // bottom
            vel = pos - rb.pos_old;
            vel.y *= -1;
            pos.y = bottomRight.y + scale.y;
            rb.velocity(vel * rb.bouncines);
        }

    }
};

struct SimData
{
    int EnabledCount = 1;
    float SpawnRate = 100;
    float SpawnTimer = 0;
    float SpawnAngleRate = 2 * PI;
    float SpawnAngle = PI/2;
    float SpawnAngleDisplacement = 0;
    glm::vec2 Gravity = {0, -1000};
};


namespace Scene {
class VerletInstanced : public Scene {
private:
    constexpr static int m_ObjCount = 10000;
    Pos_Scale_Col* m_ObjData = new Pos_Scale_Col[m_ObjCount+1];
    RigidBody* m_Bodies = new RigidBody[m_ObjCount];
    Constraint m_Constraint;
    SimData m_SimData;
    RendererInstanced<QuadData, Pos_Scale_Col, m_ObjCount+1> m_Renderer;
public:
    VerletInstanced() 
        : m_Constraint({0, HEIGHT}, {WIDTH, 0}), m_Renderer(m_ObjData) 
    {
        m_Renderer.ShaderInit("assets/shaders/instancing/BasicColorScale.vert", 
                              "assets/shaders/instancing/CircleInRectColor.frag");
        m_Renderer.InstanceShader->SetUniform<float>("u_CullRadius", 0.5f);
        m_Renderer.InstanceShader->SetUniform<float>("u_EdgeSmooth", 1.2f);
    }
    ~VerletInstanced() {
        delete [] m_ObjData;
        delete [] m_Bodies;
    }

    void Start() override {
        m_SimData.SpawnRate = m_ObjCount * 500;
        m_SimData.SpawnAngleRate = m_ObjCount/400.0f + 100;
        m_SimData.SpawnAngleDisplacement = -PI/4;
        
        float p = 0;
        for (int i = 0; i < m_ObjCount; i++) {
            p = (i+1.0f)/(m_ObjCount);
            m_ObjData[i+1].position = glm::vec2(WIDTH/2, HEIGHT/2);
            m_ObjData[i+1].scale = 5.0f * glm::vec2(2 + glm::sin(p * 200 * PI));
            m_ObjData[i+1].color = glm::vec4(p, 0.1, 1-p, 1);
            m_Bodies[i].pos = &m_ObjData[i+1].position;
            m_Bodies[i].pos_old = *m_Bodies[i].pos;
            m_Bodies[i].bouncines = p;
            float theta = m_SimData.SpawnAngleDisplacement + m_SimData.SpawnAngle/2 * (sin(p * m_SimData.SpawnAngleRate) - 1);
            m_Bodies[i].velocity(30.0f * glm::vec2(cos(theta), sin(theta)));
        }
        m_ObjData[0].position = glm::vec2(WIDTH/2, HEIGHT/2);
        m_ObjData[0].scale = glm::vec2(HEIGHT/2);
        m_ObjData[0].color = glm::vec4(0,0,0,1);
    }

    void Update(float dt) override {
        m_SimData.SpawnTimer += dt;
        if (m_SimData.SpawnTimer >= 1.0f/m_SimData.SpawnRate && m_SimData.EnabledCount < m_ObjCount) {
            m_SimData.SpawnTimer = 0;
            m_SimData.EnabledCount++;
        }
        for (int i = 0; i < m_SimData.EnabledCount; i++) {
            RigidBody& body = m_Bodies[i];
            body.accelerate(m_SimData.Gravity);
            body.updatePosition(dt);
            m_Constraint.ApplyCircle(body, m_ObjData[i+1].scale);
        }
    }

    void Render() override {
        Render::Clear(0.9, 0.9, 0.9, 1);
        m_Renderer.Draw();
    }
};
}

