#pragma once
#include "Scene.h"
#include "../layer/BatchRenderer.h"
#include "../meshes/Quad.h"

#include "../vendor/glm/ext/matrix_transform.hpp"
#include "../vendor/glm/ext/matrix_clip_space.hpp"

namespace Scene {

class Batching : public Scene {
private:
    Mesh::Quad m_Shapes[4];
    BatchRenderer<2> m_BatchRenderer;
    
public:
    Batching() : 
        m_BatchRenderer(m_Shapes, 4, glm::ortho(0.0f, WIDTH, 0.0f, HEIGHT, -1.0f, 1.0f), 
                        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)))
    {
        Renderer renderer;
        renderer.BasicBlend();
        m_BatchRenderer.ShaderInit("assets/shaders/Batching.vert", 
                                   "assets/shaders/Batching.frag");
        
        m_BatchRenderer.BatchShader->SetUniform("u_EdgeSmooth", 1.2f/m_Shapes[0].GetScale().x);
        m_BatchRenderer.BatchShader->SetUniform("u_CullRadius", 0.5f);
    }
    ~Batching() { }

    void Start() override {
        for (int i=0; i<4; i++) {
            m_Shapes[i].SetCentre(m_Shapes[i].GetCentre() + glm::vec2(sin(i), cos(i))*100.0f);
        }
    }

    void Update(float dt) override {
        for (int i=0; i<4; i++) {
            m_Shapes[i].SetCentre(m_Shapes[i].GetCentre() + glm::vec2(sin(i), cos(i))*50.0f*dt);
        }
    }
    
    void Render() override {
        Renderer renderer;
        renderer.BasicBlend();
        renderer.Clear(1, 1, 1, 1);
        
        m_BatchRenderer.DrawBatches();
    }
    
};
};

