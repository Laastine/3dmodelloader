#ifndef BLOCK_H
#define BLOCK_H

#include "Vertex.h"

using namespace std;

class Block
{
public:
	Block();
	~Block();
	void parseData(ID3D10Device *dev, float xCoord, float zCoord, string texturePath);
	void draw();

private:
	ID3D10Device* m_d3dDevice;

	ID3D10Buffer* m_blockVertexBuff;
	ID3D10Buffer* m_blockIndexBuff;

	ID3D10ShaderResourceView* m_textureContainer;	
	bool m_isBroken;
};

#endif //BLOCK_H