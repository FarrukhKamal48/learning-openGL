#pragma once
#include <iostream>
#include <memory>
#include "../Renderer.h"
#include "../Texture.h"
#include "../vendor/glm/ext/matrix_transform.hpp"
#include "../vendor/glm/ext/matrix_clip_space.hpp"

namespace Primative {

class Circle {
public:
    std::unique_ptr<VertexArray> vertexArray;
    std::unique_ptr<IndexBuffer> indexBuffer;
    std::unique_ptr<Shader> shader;
private:
    std::unique_ptr<VertexBuffer> m_VertexBuffer;

    glm::vec2 m_Centre;
    float m_Radius;
    glm::mat4 m_Model;
    
public:
    Circle() :
        m_Centre(glm::vec2(WIDTH/2, HEIGHT/2)),
        m_Radius(50.0f),
        m_Model(glm::translate(glm::mat4(1.0f), glm::vec3(m_Centre, 0.0f)))
    { 
        Init(); 
    }
    
    Circle(glm::vec2 centre, float radius) :
        m_Centre(centre),
        m_Radius(radius),
        m_Model(glm::translate(glm::mat4(1.0f), glm::vec3(centre, 0.0f)))
    { 
        Init(); 
    }
    
    Circle(const Circle& other) :
        m_Centre(other.m_Centre),
        m_Radius(other.m_Radius),
        m_Model(other.m_Model)
    { 
        Init(); 
    }
    ~Circle() { }

    void SetRadius(float radius) {
        m_Model[0][0] *= radius/m_Radius;
        m_Model[1][1] *= radius/m_Radius;
        m_Radius = radius;
        shader->Bind();
        shader->SetUniform("u_EdgeSmooth", 1.2f/m_Radius);
    }

    void SetCentre(glm::vec2 position) {
        m_Model[3][0] = position.x;
        m_Model[3][1] = position.y;
        m_Centre = position;
    }
    
    void SetColor(float r, float g, float b, float a) {
        shader->Bind();
        shader->SetUniformVec4("u_Color", r, g, b, a);
    }

    void Draw(glm::mat4& Proj, glm::mat4 View) {
        shader->Bind();
        shader->SetUniformMat4("u_MVP", Proj * View * m_Model);
        
        Renderer renderer;
        renderer.Draw(*vertexArray, *indexBuffer, *shader);
    }
    
private:
    void Init() {
        float positions[] = {
            -m_Radius, -m_Radius, 0.0f, 0.0f,
             m_Radius, -m_Radius, 1.0f, 0.0f,
             m_Radius,  m_Radius, 1.0f, 1.0f,
            -m_Radius,  m_Radius, 0.0f, 1.0f,
        };
        unsigned int indices[] = {
            0, 1, 2, 
            2, 3, 0
        };
        
        Renderer renderer;
        renderer.BasicBlend();

        vertexArray = std::make_unique<VertexArray>();
        m_VertexBuffer = std::make_unique<VertexBuffer>(positions, 4 * 4 * sizeof(float));
        indexBuffer =  std::make_unique<IndexBuffer>(indices, 2 * 3);
        
        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(2);
        vertexArray->AddBuffer(*m_VertexBuffer, layout);
        
        shader = std::make_unique<Shader>();
        shader->Push(GL_VERTEX_SHADER, "assets/shaders/Basic.vert");
        shader->Push(GL_FRAGMENT_SHADER, "assets/shaders/CircleInRect.frag");
        shader->Compile();
        shader->Bind();
        shader->SetUniformVec4("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
        shader->SetUniform("u_CullRadius", 0.5f);
        shader->SetUniform("u_EdgeSmooth", 1.2f/m_Radius);
        shader->UnBind();
    }
   
};

}
