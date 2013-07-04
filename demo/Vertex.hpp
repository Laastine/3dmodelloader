#ifndef VERTEX_H
#define VERTEX_H

#include "d3dUtil.hpp"
#include <vector>

struct Vertex {
    Vertex() {}
    Vertex(float x, float y, float z,
           float u, float v,
           float nx, float ny, float nz) : pos(x,y,z), tex(u, v), normal(nx, ny, nz) {}
    D3DXVECTOR3 pos;
    D3DXVECTOR2 tex;
    D3DXVECTOR3 normal;
};

#endif //VERTEX_H