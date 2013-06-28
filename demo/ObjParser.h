#ifndef OBJPARSER_H
#define OBJPARSER_H

#include "d3dUtil.h"
#include "Vertex.h"
#include "Material.h"
#include "Light.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class ObjParser {
    public:
        ObjParser();
        ~ObjParser();

        void init(ID3D10Device *dev);
        vector<ID3D10ShaderResourceView*> getSRV();
        vector<wstring>& getTextureNames();

        void handleFaceData(string& faceData, DWORD& vIndex, DWORD& numTriangles, vector<DWORD>& indices,
                            vector<DWORD>& vertPosIndex, vector<DWORD>& vertNormIndex, vector<DWORD>& vertTCIndex,
                            DWORD& vertexPosIndexTmp, DWORD& vertexNormIndexTmp, DWORD& vertexTexIndexTmp);
        bool handleMaterialData(string& mtlData);
        void computeNormals(vector<Vertex>& vertices, vector<DWORD>& indices, DWORD numTriangles, DWORD numVertex);
        bool parse(string fileName, bool isRHS, bool calculateNormals);

        void draw();

    private:
        vector<string> m_objContent;
        vector<string> m_material;			//mtl file

        vector<wstring> m_textureNameArray;	//material texture name vector
        vector<Material> m_materialSet;		//material struct vector

        vector<ID3D10ShaderResourceView*> m_meshSRV;	//Texture container
        ID3D10ShaderResourceView* m_tempSRV;			//Texture container

        ID3D10Buffer* m_meshVertexBuff;
        ID3D10Buffer* m_meshIndexBuff;

        ID3D10EffectMatrixVariable* m_fxTexMtxVar;
        ID3D10EffectShaderResourceVariable* m_fxDiffuseMapVar;

        /*****/
        ID3D10RasterizerState* m_RSCullNone;
        ID3D10SamplerState* m_cubesTexSamplerState;
        ID3D10RasterizerState* m_CCWcullMode;
        ID3D10RasterizerState* m_CWcullMode;
        /*****/

        vector<DWORD> m_meshSubsetIndexStart;
        vector<DWORD> m_meshSubsetTexture;

        ID3D10Device* m_d3dDevice;

        DWORD m_numVertex;
        DWORD m_numTriangles;
        DWORD m_numLine;
        DWORD m_subsetCount;
};

#endif //OBJPARSER_H