#include "layer/Input.h"
#include "scenes/Scene.h"
#include "layer/Instancing/RendererInstanced.h"

struct Bound
{
    glm::vec2 centre;
    glm::vec2 scale;

    bool contains(glm::vec2 point) {
        return (
            point.x <= centre.x + scale.x/2 && 
            point.x >= centre.x - scale.x/2 && 
            point.y <= centre.y + scale.y/2 && 
            point.y >= centre.y - scale.y/2
        );
    }

    bool intersects(const Bound& box) {
        return (box.centre.x - centre.x <= box.scale.x+scale.x || box.centre.x - centre.x >= -box.scale.x-scale.x) && 
                (box.centre.y - centre.y <= box.scale.y+scale.y || (box.centre.y - centre.y) >= -box.scale.y-scale.y);
    }
};

template<int Capacity>
class QuadTree {
public:
    bool isDivided;
public:
    QuadTree(glm::vec2 centre, glm::vec2 scale) : 
        isDivided(false), m_Boundry({centre, scale}), m_Count(0) {
        m_Cells[0] = nullptr; m_Cells[1] = nullptr;
        m_Cells[2] = nullptr; m_Cells[3] = nullptr;
    }
    ~QuadTree() {
        Delete();
    }

    bool Insert(glm::vec2 pointPos, int pointID) {
        if (!m_Boundry.contains(pointPos))
            return false;
        if (m_Count < Capacity) {
            m_Count++;
            m_Points[m_Count-1].ID = pointID;
            m_Points[m_Count-1].pos = pointPos;
            return true;
        }
        if (!isDivided) {
            SubDivide();
        }
        if (m_Cells[0]->Insert(pointPos, pointID)) return true;
        if (m_Cells[1]->Insert(pointPos, pointID)) return true;
        if (m_Cells[2]->Insert(pointPos, pointID)) return true;
        if (m_Cells[3]->Insert(pointPos, pointID)) return true;
        return false;
    }

    void Query(Bound& range, std::vector<int>& result) {
        if (!m_Boundry.intersects(range))
            return;
        for (unsigned int i = 0; i < m_Count; i++) {
            if (range.contains(m_Points[i].pos)) {
                result.push_back(m_Points[i].ID);
            }
        }
        if (isDivided) {
            m_Cells[0]->Query(range, result);
            m_Cells[1]->Query(range, result);
            m_Cells[2]->Query(range, result);
            m_Cells[3]->Query(range, result);
        }
    }

    void Delete() {
        if (!isDivided) {}
            return;
        for (int i = 0; i < 4; i++) {
            m_Cells[i]->Delete();
            delete m_Cells[i];
        }
    }
    
private:
    struct Point {
        int ID;
        glm::vec2 pos;
    };

private:
    Bound m_Boundry;
    QuadTree<Capacity>* m_Cells[4];
    unsigned int m_Count;
    Point m_Points[Capacity];

    void SubDivide() {
        m_Cells[0] = new QuadTree<Capacity>(glm::vec2(m_Boundry.centre + m_Boundry.scale/4.0f), m_Boundry.scale/2.0f);
        m_Cells[1] = new QuadTree<Capacity>(glm::vec2(m_Boundry.centre.x - m_Boundry.scale.x/4.0f, m_Boundry.centre.y + m_Boundry.scale.y/4.0f), m_Boundry.scale/2.0f);
        m_Cells[2] = new QuadTree<Capacity>(glm::vec2(m_Boundry.centre.x + m_Boundry.scale.x/4.0f, m_Boundry.centre.y - m_Boundry.scale.y/4.0f), m_Boundry.scale/2.0f);
        m_Cells[3] = new QuadTree<Capacity>(glm::vec2(m_Boundry.centre - m_Boundry.scale/4.0f), m_Boundry.scale/2.0f);
        isDivided = true;
    }

};

namespace Scene {
class QuadTreeTest : public Scene {
private:
    constexpr static int m_ObjCount = 1000;
    Pos_Scale_Col_Quad* m_ObjData;
    QuadTree<4> m_QTree;
    Bound m_CheckRange;
public:
    QuadTreeTest() 
        : m_QTree({WIDTH/2, HEIGHT/2}, {WIDTH, HEIGHT}), m_CheckRange({glm::vec2(WIDTH/2, HEIGHT/2), glm::vec2(50)})
    {
        m_ObjData = Pos_Scale_Col_Quad_Manager().Instantiate(m_ObjCount, &ConfigureShader); 
    }
    ~QuadTreeTest() { }
    static void ConfigureShader(InstanceRenderer& m_Renderer) {
        m_Renderer.CreateShader("assets/shaders/instancing/BasicColorScale.vert", 
                              "assets/shaders/instancing/CircleInRectColor.frag");
        m_Renderer.InstanceShader->SetUniform<float>("u_CullRadius", 0.5f);
        m_Renderer.InstanceShader->SetUniform<float>("u_EdgeSmooth", 1.2f);
    }

    void Start() override {
        // float ip = 0;
        for (int i = 0; i < m_ObjCount; i++) {
            // p = (i+1.0f)/(m_ObjCount);
            // ip = i + 1.0f;
            m_ObjData[i].position = glm::vec2((float)rand()/RAND_MAX * (WIDTH-20) + 10, (float)rand()/RAND_MAX * (HEIGHT-20) + 10);
            // m_ObjData[i+2].scale = 2.0f * glm::vec2(4 + glm::sin(ip * m_SimData.SpawnRadiusFreq));
            m_ObjData[i].scale = glm::vec2(8.0f);
            // m_ObjData[i+2].color = glm::vec4(glm::sin(ip * m_SimData.SpawnColorFreq), 0.3, 1-glm::sin(ip * m_SimData.SpawnColorFreq), 1);
            // m_ObjData[i+2].color = glm::vec4(p, 0.3, 1-p, 1);
            // float theta = m_SimData.SpawnAngleDisplacement + m_SimData.SpawnAngle/2 * (sin(ip * m_SimData.SpawnAngleFreq) - 1);
            m_QTree.Insert(m_ObjData[i].position, i);
        }
    }


    void Update(float dt) override {
        m_CheckRange.centre = glm::vec2(Input::GetMousePos().x, HEIGHT - Input::GetMousePos().y);
        m_CheckRange.scale = glm::vec2(1000);
        
        m_QTree.Delete();
        for (int i = 0; i < m_ObjCount; i++) {
            m_QTree.Insert(m_ObjData[i].position, i);
        }
        
        std::vector<int> points;
        m_QTree.Query(m_CheckRange, points);
        
        for (int i = 0; i < m_ObjCount; i++) {
            m_ObjData[i].color = {0, 0, 0, 0};
        }
        for (int p = 0; p < (int)points.size(); p++) {
            m_ObjData[points[p]].color = {0, 1, 0, 0};
        }


    }

    void Render() override {
        Render::Clear(0.9, 0.9, 0.9, 1);
        Render::DrawAllInstanced();
    }

};
}
