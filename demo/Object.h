#ifndef CBOBJECT_H
#define CBOBJECT_H

#include "d3dUtil.h"

struct Object
{
	//Object() {}
	//Object(float x, float y, float z, float w, bool tex) : difColor(x, y, z, w), hasTexture(0)
	//{
	//	D3DXMatrixIdentity(&WVP);
	//	D3DXMatrixIdentity(&WVP);
	//	//D3DXMatrixIdentity(&texMtx);
	//} 
	D3DXMATRIX  WVP;
	D3DXMATRIX World;
	D3DXMATRIX texMtx;
	double foo;
	double foo2;
	double foo3;
	
	D3DXVECTOR4 difColor;
	double foo4;
	bool hasTexture;	//Zero means false
	double boo;
};

#endif //CBOBJECT_H