#include <string>
#include <iostream>
#include "Block.h"
#include "Vertex.h"

using namespace std;

Block::Block() : m_blockVertexBuff(0), m_blockIndexBuff(0)
{
}

Block::~Block()
{
	ReleaseCOM(m_blockVertexBuff);
	ReleaseCOM(m_blockIndexBuff);
}

void Block::parseData(ID3D10Device *dev, float xCoord, float zCoord, string texturePath)
{
	m_d3dDevice = dev;
	wchar_t wCharPath[64];
	int numVertex = 4;
	int numIndices = 6;
	Vertex v[4];	//numVertex
	DWORD i[6];		//numIndices
	//#if defined( DEBUG ) || defined( _DEBUG )
	//cout<<"XCoord: "<<xCoord<<" zCoord: "<<zCoord<<endl;
	//#endif

	//Fill in vertex data
	v[0] = Vertex((-1.0f+xCoord), 0.0f, (-1.0f+zCoord), 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[1] = Vertex((-1.0f+xCoord), 0.0f, ( 1.0f+zCoord), 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex(( 1.0f+xCoord), 0.0f, ( 1.0f+zCoord), 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	v[3] = Vertex(( 1.0f+xCoord), 0.0f, (-1.0f+zCoord), 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	
	//Fill in index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	D3D10_BUFFER_DESC vbd;
    vbd.Usage = D3D10_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * numVertex;
    vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = v;
    HR(m_d3dDevice->CreateBuffer(&vbd, &vinitData, &m_blockVertexBuff));

	D3D10_BUFFER_DESC ibd;
    ibd.Usage = D3D10_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(DWORD) * numIndices;
    ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = i;
    HR(m_d3dDevice->CreateBuffer(&ibd, &iinitData, &m_blockIndexBuff));
	
	//#if defined( DEBUG ) || defined( _DEBUG )
	//cout<<"Using texture: "<<texturePath<<endl;
	//#endif

	stringToWchar(texturePath, wCharPath);
	HR(D3DX10CreateShaderResourceViewFromFile(m_d3dDevice, wCharPath, NULL, NULL, &m_textureContainer, NULL));
}

void Block::draw()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_d3dDevice->IASetVertexBuffers(0, 1, &m_blockVertexBuff, &stride, &offset);
	m_d3dDevice->IASetIndexBuffer(m_blockIndexBuff, DXGI_FORMAT_R32_UINT, 0);
	m_d3dDevice->DrawIndexed(6, 0, 0);
}