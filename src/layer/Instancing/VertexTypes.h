#pragma once
#include "../../vendor/glm/ext/vector_float2.hpp"
#include "../../vendor/glm/ext/vector_float4.hpp"
#include "../VertexBufferLayout.h"
#include <algorithm>
#include <vector>

class Pos {
public:
    inline static std::vector<Pos*> instances;
    glm::vec2 position;

    Pos() : position(0) { instances.push_back(this); }
    ~Pos() { 
        auto it = std::find(instances.begin(), instances.end(), this);
        if (it != instances.end())
            instances.erase(it);
    }

    static VertexBufferLayout Layout(unsigned int divisor) {
        VertexBufferLayout layout;
        layout.Push<float>(2, divisor);
        return layout;
    }
};

class Pos_Col {
public:
    inline static std::vector<Pos_Col*> instances;
    glm::vec2 position;
    glm::vec4 color;

    Pos_Col() : position(0), color(0) { instances.push_back(this); }
    ~Pos_Col() {
        auto it = std::find(instances.begin(), instances.end(), this);
        if (it != instances.end())
            instances.erase(it);
    }

    static VertexBufferLayout Layout(unsigned int divisor) {
        VertexBufferLayout layout;
        layout.Push<float>(2, divisor);
        layout.Push<float>(4, divisor);
        return layout;
    }
};

class Pos_Scale_Col {
public:
    inline static std::vector<Pos_Scale_Col*> instances;
    glm::vec2 position;
    glm::vec2 scale;
    glm::vec4 color;

    Pos_Scale_Col() : position(0), scale(0), color(0) { instances.push_back(this); }
    ~Pos_Scale_Col() { 
        auto it = std::find(instances.begin(), instances.end(), this);
        if (it != instances.end())
            instances.erase(it);
    }

    static VertexBufferLayout Layout(unsigned int divisor) {
        VertexBufferLayout layout;
        layout.Push<float>(2, divisor);
        layout.Push<float>(2, divisor);
        layout.Push<float>(4, divisor);
        return layout;
    }
};

// ###################################################################################################
// ##############################  VertexLookup  #####################################################
// ###################################################################################################

template<typename VertexT> class VertexLookup {
public:
    VertexLookup() = delete;

    static VertexBufferLayout Layout(unsigned int divisor);
    static unsigned int SizeOfVertex();
};

template<>
class VertexLookup<Pos> {
public:
    VertexLookup() = delete;

    static VertexBufferLayout Layout(unsigned int divisor) {
        return Pos::Layout(divisor);
    }
    static unsigned int SizeOfVertex() {
        return sizeof(Pos);
    }
};

template<>
class VertexLookup<Pos_Col> {
public:
    VertexLookup() = delete;

    static VertexBufferLayout Layout(unsigned int divisor) {
        return Pos_Col::Layout(divisor);
    }
    static unsigned int SizeOfVertex() {
        return sizeof(Pos_Col);
    }
};

template<>
class VertexLookup<Pos_Scale_Col> {
public:
    VertexLookup() = delete;
    
    static VertexBufferLayout Layout(unsigned int divisor) {
        return Pos_Scale_Col::Layout(divisor);
    }
    static unsigned int SizeOfVertex() {
        return sizeof(Pos_Scale_Col);
    }
    
};
