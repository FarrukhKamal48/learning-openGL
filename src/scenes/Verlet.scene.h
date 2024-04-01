#pragma once
#include "../layer/BatchRenderer.h"
#include "../meshes/Quad.h"
#include "Scene.h"
#include <cmath>
#define PI (3.141592653589793)

class VerletObject {
public:
    glm::vec2 pos_old;
    glm::vec2 pos;
    glm::vec2 acceleration;
    Mesh::Quad* Mesh;
    float Radius;

    void SetRadius(float radius) { 
        Mesh->SetScale(glm::vec2(radius)); 
        Radius = radius;
    }
    float GetRadius() const { return Radius; }

    void updatePosition(float dt) {
        const glm::vec2 deltaPos = pos - pos_old;
        pos_old = pos;
        pos = pos + deltaPos + acceleration*dt*dt;
        acceleration = {};
        Mesh->SetCentre(pos);
    }

    void accelerate(glm::vec2 acc) {
        acceleration += acc;
    }
};

struct CircleConstraint {
    glm::vec2 centre;
    float radius;
    CircleConstraint() : centre({WIDTH/2, HEIGHT/2}), radius(HEIGHT/2) {}
};

struct SimulationData {
    unsigned int EnabledObjCount = 1;
    unsigned int SubSteps = 8;
    float Subdt = 0;
    float ObjSpawnRate = 0.1f;
    float SpawnTimer = 0;
};

namespace Scene {
class Verlet : public Scene {
private:
    static const unsigned int m_ObjCount = 512;
    VerletObject* m_Objs = new VerletObject[m_ObjCount];
    Mesh::Quad* m_Shapes = new Mesh::Quad[m_ObjCount];
    
    BatchRenderer<Mesh::Quad, m_ObjCount> m_BatchRenderer;

    glm::vec2 gravity = {0, -3000};
    CircleConstraint m_Constraint;
    SimulationData m_SimData;
    
public:
    Verlet() : 
        m_BatchRenderer(m_Shapes, m_ObjCount, glm::ortho(0.0f, WIDTH, 0.0f, HEIGHT, -1.0f, 1.0f), 
                        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)))
    {
        m_BatchRenderer.ShaderInit("assets/shaders/Batching.vert", 
                                   "assets/shaders/BatchingCircle.frag");
        m_BatchRenderer.BatchShader->SetUniform("u_EdgeSmooth", 1.2f/m_Shapes[0].GetScale().x);
        m_BatchRenderer.BatchShader->SetUniform("u_CullRadius", 0.5f);
    }
    ~Verlet() { 
        delete [] m_Objs;
        delete [] m_Shapes;
    }

    void Start() override {
        glm::vec2 velocityDir(0);
        float iPercent = 0;
        float theta = 0;
        for (unsigned int i=0; i<m_ObjCount; i++) {
            iPercent = (float)i/m_ObjCount;
            
            
            m_Objs[i].Mesh = &m_Shapes[i];
            m_Objs[i].Mesh->SetColor(iPercent, 1-(float)i/m_ObjCount, 1, 1);
            m_Objs[i].SetRadius(5 * (2.0f+sin(iPercent * PI * 20.0f)));
            // m_Objs[i].SetRadius(10);
            m_Objs[i].Mesh->SetCentre(m_Constraint.centre);
            
            theta = -PI/2 + PI/2 * sin(iPercent * 2 * PI);
            velocityDir = glm::vec2(cos(theta), sin(theta));
            // velocityDir = glm::vec2(1, -0.2);
            m_Objs[i].pos = m_Constraint.centre;// + glm::vec2(-200, 200);
            m_Objs[i].pos_old = m_Objs[i].pos - velocityDir*10.0f;
        }
        m_SimData.ObjSpawnRate = 100.0f;
        m_SimData.SubSteps = 8;
    }

    void ConstrainObj(VerletObject& obj) {
        glm::vec2 displacement = -m_Constraint.centre + obj.pos;
        float displaceDist = glm::length(displacement);
        
        if (displaceDist > m_Constraint.radius - obj.GetRadius()) {
            obj.pos = m_Constraint.centre + displacement/displaceDist * (m_Constraint.radius - obj.GetRadius());
        }
    }

    void Collide(VerletObject& objA, VerletObject& objB) {
        glm::vec2 axis = -objA.pos + objB.pos;
        float dist = glm::length(axis);

        if (dist < objA.Radius + objB.Radius) {
            float overlap = objA.Radius + objB.Radius - dist;
            objA.pos -= axis/dist * overlap/2.0f;
            objB.pos += axis/dist * overlap/2.0f;
        }
    }

    void Update(float dt) override {
        m_SimData.SpawnTimer += dt;
        if (m_SimData.SpawnTimer >= 1/m_SimData.ObjSpawnRate && m_SimData.EnabledObjCount < m_ObjCount) {
            m_SimData.EnabledObjCount++;
            m_SimData.SpawnTimer = 0;
        }
        m_SimData.Subdt = dt/m_SimData.SubSteps;
        
        for (unsigned int s = 0; s < m_SimData.SubSteps; s++) {
            for (unsigned int i = 0; i < m_SimData.EnabledObjCount; i++) {
                m_Objs[i].accelerate(gravity);
                ConstrainObj(m_Objs[i]);
                
                for (unsigned int j = i+1; j < m_SimData.EnabledObjCount; j++) { 
                    if (i == j) continue;
                    Collide(m_Objs[i], m_Objs[j]);
                }
                
                m_Objs[i].updatePosition(m_SimData.Subdt);
            }
        }
        
    }
    
    void Render() override {
        Renderer::BasicBlend();
        Renderer::Clear(0, 0, 0, 1);

        // m_BatchRenderer.DrawBatches(m_SimData.EnabledObjCount);
        m_BatchRenderer.DrawBatches();
    }
    
};}

