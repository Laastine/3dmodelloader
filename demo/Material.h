#ifndef MATERIAL_H
#define MATERIAL_H

#include "d3dUtil.h"
#include <iostream>

using namespace std;

struct Material {
    Material() {}
    Material (string name, D3DXVECTOR4 color, int index, bool texture, bool transparent) :
        matName(name), difColor(color), texArrayIndex(index), hasTexture(texture), isTransparent(transparent) {}
    string matName;
    D3DXVECTOR4 difColor;
    int texArrayIndex;
    bool hasTexture;
    bool isTransparent;
};

#endif //MATERIAL_H