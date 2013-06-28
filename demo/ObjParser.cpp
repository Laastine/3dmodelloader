#include "ObjParser.h"
#include "Vertex.h"
#include "d3dUtil.h"
#include "Camera.h"
#include "main.h"
#include "Material.h"
#include "Vertex.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>

using namespace std;

ObjParser::ObjParser() : m_d3dDevice(0), m_numLine(0), m_numTriangles(0), m_numVertex(0),
    m_subsetCount(0), m_objContent(0), m_material(0), m_materialSet(0), m_meshVertexBuff(0), m_meshIndexBuff(0),
    m_meshSubsetIndexStart(0), m_meshSubsetTexture(0), m_fxTexMtxVar(0), m_fxDiffuseMapVar(0) {}

ObjParser::~ObjParser() {
    ReleaseCOM(m_meshVertexBuff);
    ReleaseCOM(m_meshIndexBuff);
    m_objContent.clear();
    m_material.clear();
    m_textureNameArray.clear();
    m_materialSet.clear();
    m_meshSRV.clear();
    m_meshSubsetIndexStart.clear();
    m_meshSubsetTexture.clear();
}

void ObjParser::init(ID3D10Device *dev) {
    m_d3dDevice = dev;
}

vector<ID3D10ShaderResourceView*> ObjParser::getSRV() {
    return m_meshSRV;
}

vector<wstring>& ObjParser::getTextureNames() {
    return m_textureNameArray;
}

bool ObjParser::parse(string fileName, bool isRHS, bool calculateNormals) {
    string path = "..\\data\\";
    HRESULT hr = 0;
    ifstream file(fileName);
    string temp = "";

    while(getline(file, temp)) {
        m_objContent.push_back(temp);
    }

    file.close();	//Close the obj file

#if defined(DEBUG) || defined(_DEBUG)
    cout<<fileName<<" read"<<endl;
#endif

    vector<string> tmp;

    vector<DWORD> indices;
    vector<D3DXVECTOR3> vPos;
    vector<D3DXVECTOR2> vTex;
    vector<D3DXVECTOR3> vNorm;

    vector<string> meshMaterials;

    string mtlName;
    string mtlLibName;

    bool hasNormCoord = false;
    bool hasTexCoord = false;

    //Vertex definition indices
    vector<DWORD> vertPosIndex;
    vector<DWORD> vertNormIndex;
    vector<DWORD> vertTCIndex;

    //Indexing variables
    string meshMaterialTmp;
    DWORD vertexPosIndexTmp = 0;
    DWORD vertexNormIndexTmp = 0;
    DWORD vertexTexIndexTmp = 0;

    DWORD vIndex = 0;
    DWORD triangleCount = 0;	//Total Triangles
    DWORD index = 0;
    m_numVertex = 0;

    for(DWORD line = 0; line<m_objContent.size(); line++) {
#if defined(DEBUG) || defined(_DEBUG)
        m_numLine++;
#endif
        //split(tmp, m_objContent[line], " ");
        boost::split(tmp, m_objContent[line], boost::is_any_of(" "), boost::token_compress_on);

        //Geometric vertices
        if(tmp[0] == "v") {
            if(isRHS) {
                vPos.push_back(D3DXVECTOR3( stringToFloat(tmp[1]), stringToFloat(tmp[2]), (stringToFloat(tmp[3]) * -1.0f) ));	//Invert Z-axis
            } else {
                vPos.push_back(D3DXVECTOR3( stringToFloat(tmp[1]), stringToFloat(tmp[2]), stringToFloat(tmp[3]) ));
            }
        }

        //Texture coordinates
        else if(tmp[0] == "t" || tmp[0] == "vt") {
            if(isRHS) {
                vTex.push_back(D3DXVECTOR2( stringToFloat(tmp[1]), (1.0f - stringToFloat(tmp[2]))));	//Reserve V-axis
            } else {
                vTex.push_back(D3DXVECTOR2( stringToFloat(tmp[1]), stringToFloat(tmp[2])));
            }
            hasTexCoord = true;
        }

        //Normal coordinates
        else if(tmp[0] == "n" ||tmp[0] == "vn") {
            if(isRHS) {
                vNorm.push_back(D3DXVECTOR3( stringToFloat(tmp[1]), stringToFloat(tmp[2]), stringToFloat(tmp[3]) * -1.0f));	//Invert Z-axis
            } else {
                vNorm.push_back(D3DXVECTOR3( stringToFloat(tmp[1]), stringToFloat(tmp[2]), stringToFloat(tmp[3]) ));
            }
            hasNormCoord = true;
        }

        //Faces
        else if(tmp[0] == "f") {
            handleFaceData(m_objContent[line], vIndex, m_numTriangles, indices, vertPosIndex, vertNormIndex, vertTCIndex, vertexNormIndexTmp, vertexPosIndexTmp, vertexTexIndexTmp);
        }

        //Group
        else if(tmp[0] == "g") {
            m_meshSubsetIndexStart.push_back(vIndex);
            m_subsetCount++;
        }

        //Smoothing group
        else if(tmp[0] == "s") {
            //TODO
        }

        //Material lib
        else if(tmp[0] == "usemtl") {
#if defined(DEBUG) || defined(_DEBUG)
            cout<<"mtllib file read"<<tmp[1]<<endl;
#endif
            //Save material name to meshMaterials vector
            meshMaterials.push_back(tmp[1]);
        }

        //Material file(textures and surface properties)
        else if(tmp[0] == "mtllib") {
            //Pass material file name as parameter
            handleMaterialData(tmp[1]);
        }

        //Object name
        else if(tmp[0] == "o") {
#if defined(DEBUG) || defined(_DEBUG)
            cout<<"Object name: "<<tmp[1]<<endl;
#endif
        }

        //Unknown keyword
        else {
#if defined(DEBUG) || defined(_DEBUG)
            //cout<<"Discarding line: "<<tmp[0]<<"..."<<m_numLine<<" "<<endl;
#endif
        }
        tmp.clear();
    }

    m_objContent.clear();

    m_meshSubsetIndexStart.push_back(vIndex); //Set last subset

    //sometimes "g" is defined at the very top of the file, then again before the first group of faces.
    //This makes sure the first subset does not conatain "0" indices.
    if(m_meshSubsetIndexStart[1] == 0) {
        m_meshSubsetIndexStart.erase(m_meshSubsetIndexStart.begin()+1);
        m_subsetCount--;
    }

    //Default values for hasNormCoord and hasTexCoord, if not spesifief earlier
    if(!hasNormCoord) {
        vNorm.push_back(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
    }
    if(!hasTexCoord) {
        vTex.push_back(D3DXVECTOR2(0.0f, 0.0f));
    }

    //Set the subsets m_materialSet to the index value
    //of the its material in our m_materialSet array
    for(DWORD i = 0; i < m_subsetCount; ++i) {
        bool hasMat = false;
        for(DWORD j = 0; j < m_materialSet.size(); ++j) {
            if(meshMaterials[i] == m_materialSet[j].matName) {
                m_meshSubsetTexture.push_back(j);
                hasMat = true;
            }
        }
        if(!hasMat) {
            m_meshSubsetTexture.push_back(0); //Use first material in m_materialSet array
        }
    }

#if defined(DEBUG) || defined(_DEBUG)
    cout<<"m_meshSubset: "<<m_subsetCount<<endl;
    cout<<"m_numVertex: "<<m_numVertex<<endl;
    cout<<"m_numTriangles: "<<m_numTriangles<<endl;
#endif

    std::vector<Vertex> vertices;
    Vertex tempVert;

    //Create our vertices using the information we got
    //from the file and store them in a vector
    for(DWORD j = 0; j < m_numVertex; ++j) {
        tempVert.pos = vPos[vertPosIndex[j]];
        tempVert.normal = vNorm[vertNormIndex[j]];
        tempVert.tex = vTex[vertTCIndex[j]];
        vertices.push_back(tempVert);
    }

    //Compute object normals, if not provided
    if(calculateNormals) {
        computeNormals(vertices, indices, m_numTriangles, m_numVertex);
#if defined(DEBUG) || defined(_DEBUG)
        cout<<"Surface normals calculated"<<endl;
#endif
    }

    //Dump indices data to graphics card
    D3D10_BUFFER_DESC ibd;
    ibd.Usage = D3D10_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(DWORD) * m_numTriangles * 3;
    ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(m_d3dDevice->CreateBuffer(&ibd, &iinitData, &m_meshIndexBuff));

    //Dump vertices data to graphics card
    D3D10_BUFFER_DESC vbd;
    vbd.Usage = D3D10_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * m_numVertex;
    vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(m_d3dDevice->CreateBuffer(&vbd, &vinitData, &m_meshVertexBuff));

    //Delete vectors
    vertices.clear();
    indices.clear();

    return true;
}

void ObjParser::draw() {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    for(DWORD i = 0; i<m_subsetCount; ++i) {
        //Set index buffer
        m_d3dDevice->IASetIndexBuffer(m_meshIndexBuff, DXGI_FORMAT_R32_UINT, 0);
        //Set vertex buffer
        m_d3dDevice->IASetVertexBuffers(0, 1, &m_meshVertexBuff, &stride, &offset);

        int indexStart = m_meshSubsetIndexStart[i];
        int indexDrawAmount = m_meshSubsetIndexStart[i+1] - m_meshSubsetIndexStart[i];
        m_d3dDevice->PSSetShaderResources(0, 1, &m_meshSRV[m_materialSet[m_meshSubsetTexture[i]].texArrayIndex]);

        m_d3dDevice->DrawIndexed(indexDrawAmount, indexStart, 0);
    }
}

void ObjParser::computeNormals(vector<Vertex>& vertices, vector<DWORD>& indices, DWORD numTriangles, DWORD numVertex) {
    std::vector<D3DXVECTOR3> tempNormal;

    //normalized and unnormalized normals
    D3DXVECTOR3 unnormalized = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    //Used to get vectors (sides) from the position of the verts
    float vecX, vecY, vecZ;

    //Two edges of our triangle
    D3DXVECTOR3 edge1 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 edge2 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    //Compute face normals
    for(DWORD i = 0; i < m_numTriangles; ++i) {
        //Get the vector describing one edge of our triangle (edge 0,2)
        vecX = vertices[indices[(i*3)]].pos.x - vertices[indices[(i*3)+2]].pos.x;
        vecY = vertices[indices[(i*3)]].pos.y - vertices[indices[(i*3)+2]].pos.y;
        vecZ = vertices[indices[(i*3)]].pos.z - vertices[indices[(i*3)+2]].pos.z;
        edge1 = D3DXVECTOR3(vecX, vecY, vecZ);	//Create our first edge

        //Get the vector describing another edge of our triangle (edge 2,1)
        vecX = vertices[indices[(i*3)+2]].pos.x - vertices[indices[(i*3)+1]].pos.x;
        vecY = vertices[indices[(i*3)+2]].pos.y - vertices[indices[(i*3)+1]].pos.y;
        vecZ = vertices[indices[(i*3)+2]].pos.z - vertices[indices[(i*3)+1]].pos.z;
        edge2 = D3DXVECTOR3(vecX, vecY, vecZ);	//Create our second edge

        //Cross multiply the two edge vectors to get the un-normalized face normal
        D3DXVECTOR3 out;
        D3DXVec3Cross(&out, &edge1, &edge2);

        tempNormal.push_back(out);			//Save unormalized normal (for normal averaging)
    }

    //Compute vertex normals (normal Averaging)
    D3DXVECTOR4 normalSum = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
    float facesUsing = 0.0f;
    float tX, tY, tZ;

    //Go through each vertex
    for(DWORD i = 0; i < m_numVertex; ++i) {
        //Check which triangles use this vertex
        for(DWORD j = 0; j < m_numTriangles; ++j) {
            if(indices[j*3] == i || indices[(j*3)+1] == i || indices[(j*3)+2] == i) {
                tX = normalSum.x + tempNormal[j].x;
                tY = normalSum.y + tempNormal[j].y;
                tZ = normalSum.z + tempNormal[j].z;

                normalSum = D3DXVECTOR4(tX, tY, tZ, 0.0f);	//If a face is using the vertex, add the unormalized face normal to the normalSum
                facesUsing++;
            }
        }

        //Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
        normalSum = normalSum / facesUsing;

        //Normalize the normalSum vector
        D3DXVECTOR4 normalSumOut;
        D3DXVec4Normalize(&normalSumOut, &normalSum);

        //Store the normal in our current vertex
        vertices[i].normal.x = normalSum.x;
        vertices[i].normal.y = normalSum.y;
        vertices[i].normal.z = normalSum.z;

        //Clear normalSum and facesUsing for next vertex
        normalSum = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
        facesUsing = 0;
    }
}

void ObjParser::handleFaceData(string& faceData, DWORD& vIndex, DWORD& m_numTriangles, vector<DWORD>& indices, vector<DWORD>& vertPosIndex, vector<DWORD>& vertNormIndex,
                               vector<DWORD>& vertTCIndex, DWORD& vertexPosIndexTmp, DWORD& vertexNormIndexTmp, DWORD& vertexTexIndexTmp) {
    wstring VertDef;	//Holds 1 vertex definition at a time
    DWORD index = 2;
    wstringstream ss;
    DWORD triangleCount = 0;
    DWORD whichPart;
    string vertPart;

    for(DWORD i=3; i<faceData.length(); i++) {
        if(faceData[i] == ' ') {
            triangleCount++;
        }
    }

    //Check for space at the end of our face string
    if(faceData.length() == ' ') {
        triangleCount--;	//Each space adds to our triangle count
    }

    ss<<(faceData.substr(2, (faceData.length()-2))).c_str();	//Cut "f " away from string

    if(faceData.length() > 0) {
        DWORD firstVIndex, lastVIndex;	//Holds the first and last vertice's index

        for(DWORD i = 0; i < 3; ++i) {		//First three vertices (first triangle)
            ss >> VertDef;			//Get vertex definition (vPos/vTexCoord/vNorm)
            whichPart = 0;			//(vPos, vTexCoord, or vNorm)

            //Parse this string
            for(DWORD j = 0; j < VertDef.length(); ++j) {
                if(VertDef[j] != '/') {	//If there is no divider "/", add a char to our vertPart
                    vertPart += VertDef[j];
                }
                if(VertDef[j] == '/' || j ==  VertDef.length()-1) {	//If the current char is a divider "/", or its the last character in the string

                    stringstream stringToInt(vertPart);	//Used to convert wstring to int

                    if(whichPart == 0) {	//If vPos

                        stringToInt >> vertexPosIndexTmp;
                        vertexPosIndexTmp -= 1;		//subtract 1 since arrays start with 0, and *.obj files start with 1

                        //Check to see if the vert pos was the only thing specified
                        if(j == VertDef.length()-1) {
                            vertexNormIndexTmp = 0;
                            vertexTexIndexTmp = 0;
                        }
                    }

                    else if(whichPart == 1) {
                        if(vertPart != "") {	//Check to see if there even is a tex coord
                            stringToInt >> vertexTexIndexTmp;
                            vertexTexIndexTmp -= 1;	//subtract 1 since arrays start with 0, and *.obj files start with 1
                        } else	//If there is no texture coordinates, make a default
                        { vertexTexIndexTmp = 0; }

                        //If the cur. char is the second to last in the string, then
                        //there must be no normal, so set a default normal
                        if(j == VertDef.length()-1) {
                            vertexNormIndexTmp = 0;
                        }
                    } else if(whichPart == 2) {	//If vNorm
                        istringstream stringToInt(vertPart);

                        stringToInt >> vertexNormIndexTmp;
                        vertexNormIndexTmp -= 1;		//subtract 1 since arrays start with 0, and *.obj files start with 1
                    }

                    vertPart = "";	//Get ready for next vertex part
                    whichPart++;	//Move on to next vertex part
                }
            }

            //Check to make sure there is at least one subset
            if(m_subsetCount == 0) {
                m_meshSubsetIndexStart.push_back(vIndex);		//Start indexing for this subset
                m_subsetCount++;
            }

            //Check and delete duplicated vertices
            bool vertAlreadyExists = false;
            if(m_numVertex >= 3) {	//at least one triangle to check

                //Loop through all the vertices
                for(DWORD iCheck = 0; iCheck < m_numVertex; ++iCheck) {
                    //Check that vertex and texture coordinates match between memory(indices vector) and vertPosIndex
                    //and mark them as "vertAlreadyExists"
                    if(vertexPosIndexTmp == vertPosIndex[iCheck] && !vertAlreadyExists) {
                        if(vertexTexIndexTmp == vertTCIndex[iCheck]) {
                            indices.push_back(iCheck);		//Set index for this vertex
                            vertAlreadyExists = true;		//and mark it as existing one
                        }
                    }
                }
            }

            //If this vertex is not already in our vertex arrays, put it there
            if(!vertAlreadyExists) {
                vertPosIndex.push_back(vertexPosIndexTmp);
                vertTCIndex.push_back(vertexTexIndexTmp);
                vertNormIndex.push_back(vertexNormIndexTmp);
                m_numVertex++;	//Create new vertex
                indices.push_back(m_numVertex-1);	//Set index for this vertex
            }

            //If first vertex in the face, link rest of the triangles to it
            if(i == 0) {
                firstVIndex = indices[vIndex];	//The first vertex index of this FACE
            }

            //If this was the last vertex in the first triangle, we will make sure
            //the next triangle uses this one (eg. tri1(1,2,3) tri2(1,3,4) tri3(1,4,5))
            if(i == 2) {
                lastVIndex = indices[vIndex];	//The last vertex index of this TRIANGLE
            }
            vIndex++;	//Increment index count
        }

        m_numTriangles++;	//One triangle less

        //If face definition contains more than 3 indices then convert rest of the indices to triangles
        for(DWORD l = 0; l < triangleCount-1; ++l) {	//Loop through the next vertices to create new triangles
            //First vertex of this triangle (the very first vertex of the face too)
            indices.push_back(firstVIndex);			//Set index for this vertex
            vIndex++;

            //Second Vertex of this triangle (the last vertex used in the tri before this one)
            indices.push_back(lastVIndex);			//Set index for this vertex
            vIndex++;

            ss >> VertDef;	//Get the third vertex for this triangle
            wstring vertPart;
            DWORD whichPart = 0;

            //Parse this string (same as above)
            for(DWORD j = 0; j < VertDef.length(); ++j) {
                if(VertDef[j] != '/') {
                    vertPart += VertDef[j];
                }
                if(VertDef[j] == '/' || j ==  VertDef.length()-1) {
                    std::wistringstream wstringToInt(vertPart);

                    if(whichPart == 0) {
                        wstringToInt >> vertexPosIndexTmp;
                        vertexPosIndexTmp -= 1;

                        //Check to see if the vert pos was the only thing specified
                        if(j == VertDef.length()-1) {
                            vertexTexIndexTmp = 0;
                            vertexNormIndexTmp = 0;
                        }
                    } else if(whichPart == 1) {
                        if(vertPart != L"") {
                            wstringToInt >> vertexTexIndexTmp;
                            vertexTexIndexTmp -= 1;
                        } else {
                            vertexTexIndexTmp = 0;
                        }
                        if(j == VertDef.length()-1) {
                            vertexNormIndexTmp = 0;
                        }
                    } else if(whichPart == 2) {
                        std::wistringstream wstringToInt(vertPart);

                        wstringToInt >> vertexNormIndexTmp;
                        vertexNormIndexTmp -= 1;
                    }

                    vertPart = L"";
                    whichPart++;
                }
            }

            //Check for duplicate vertices
            bool vertAlreadyExists = false;
            if(m_numVertex >= 3) {	//At least one triangle to check

                for(DWORD iCheck = 0; iCheck < m_numVertex; ++iCheck) {
                    if(vertexPosIndexTmp == vertPosIndex[iCheck] && !vertAlreadyExists) {
                        if(vertexTexIndexTmp == vertTCIndex[iCheck]) {
                            indices.push_back(iCheck);		//Set index for this vertex
                            vertAlreadyExists = true;		//If we've made it here, the vertex already exists
                        }
                    }
                }
            }

            if(!vertAlreadyExists) {
                vertPosIndex.push_back(vertexPosIndexTmp);
                vertTCIndex.push_back(vertexTexIndexTmp);
                vertNormIndex.push_back(vertexNormIndexTmp);
                m_numVertex++;							//New vertex created, add to total verts
                indices.push_back(m_numVertex-1);		//Set index for this vertex
            }

            //Set the second vertex for the next triangle to the last vertex we got
            lastVIndex = indices[vIndex];	//The last vertex index of this TRIANGLE

            m_numTriangles++;	//New triangle defined
            vIndex++;
        }
    }
}

bool ObjParser::handleMaterialData(string& mtlData) {
    string temp = "";
    try {
        ifstream file;
        file.open("..\\data\\"+mtlData);
        while(getline(file, temp)) {
            m_material.push_back(temp);
        }
    } catch (ifstream::failure e) {
        cout<<"mtl file reading failed"<<endl;
    }

#if defined(DEBUG) || defined(_DEBUG)
    cout<<"mtl file "<<mtlData<<" read"<<endl;
#endif

    vector<string> mtlTmp;
    bool diffColor = false;

    DWORD mtlCount = 0;//m_materialSet.size();

    for(DWORD mtlLine=0; mtlLine<m_material.size(); mtlLine++) {
#if defined( DEBUG ) || defined( _DEBUG )
        wcout<<"\tMaterial line number: "<<mtlLine+2<<endl;
#endif
        if(m_material[mtlLine].empty()) {
            continue;
        }

        boost::split(mtlTmp, m_material[mtlLine], boost::is_any_of(" "), boost::token_compress_on);

        //Comment
        if(mtlTmp[0] == "#") {
            //Comment line
        }

        //Diffuse color
        else if(mtlTmp[0] == "Kd") {
            m_materialSet[mtlCount-1].difColor.x = stringToFloat(mtlTmp[1]);
            m_materialSet[mtlCount-1].difColor.y = stringToFloat(mtlTmp[2]);
            m_materialSet[mtlCount-1].difColor.z = stringToFloat(mtlTmp[3]);
            diffColor = true;
        }

        //Ambient color
        else if(mtlTmp[0] == "Ka") {
            //cout<<"test: "<<mtlCount<<endl;
            if(!diffColor) {
                m_materialSet[mtlCount-1].difColor.x = stringToFloat(mtlTmp[1]);
                m_materialSet[mtlCount-1].difColor.y = stringToFloat(mtlTmp[2]);
                m_materialSet[mtlCount-1].difColor.z = stringToFloat(mtlTmp[3]);
            }
        }

        //Transparency
        else if(mtlTmp[0] == "Tr" || mtlTmp[0] == "d") {
            float transparency = stringToFloat(mtlData);
            m_materialSet[mtlCount-1].difColor.w = transparency;
            if(transparency > 0.0f) {
                m_materialSet[mtlCount-1].isTransparent = true;
            }
        }

        //Material(texture)
        else if(mtlTmp[0] == "map_Kd") {
            bool isLoaded = false;
            wchar_t textureName[64];
            stringToWchar(mtlTmp[1], textureName);

            for(DWORD k=0; k<m_textureNameArray.size(); ++k) {
                if(textureName == m_textureNameArray[k]) {
                    isLoaded = true;
                    m_materialSet[mtlCount-1].texArrayIndex = k;
                    m_materialSet[mtlCount-1].hasTexture = true;
                }
            }

            //If not loaded
            if(!isLoaded) {
                wchar_t fileName[64];
                stringToWchar(("..\\data\\"+mtlTmp[1]), fileName);

#if defined( DEBUG ) || defined( _DEBUG )
                wcout<<"Texture: "<<fileName<<endl;
#endif

                ID3D10ShaderResourceView* tmpMeshSRV;

                //Load texture to tmpMeshSRV
                HRESULT hr = D3DX10CreateShaderResourceViewFromFile(m_d3dDevice, fileName, NULL, NULL, &tmpMeshSRV, NULL);

                if(SUCCEEDED(hr)) {
#if defined(DEBUG) || defined(_DEBUG)
                    cout<<"Texture "<<mtlTmp[1]<<" loading OK. Size now: "<<m_meshSRV.size()+1<<endl;
#endif
                    m_textureNameArray.push_back(textureName);
                    m_materialSet[mtlCount-1].texArrayIndex = m_meshSRV.size();
                    m_meshSRV.push_back(tmpMeshSRV);
                    m_materialSet[mtlCount-1].hasTexture = true;
                } else {
                    cout<<"Texture("<<mtlTmp[1]<<") loading failed"<<endl;
                    return false;
                }
            }
        }

        //New material
        else if(mtlTmp[0] == "newmtl") {
#if defined(DEBUG) || defined(_DEBUG)
            cout<<"New material: "<<mtlTmp[1]<<endl;
#endif

            m_materialSet.push_back(Material(mtlTmp[1], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 0, false, false));
#if defined(DEBUG) || defined(_DEBUG)
            cout<<"m_materialSet new size: "<<m_materialSet.size()<<endl;
#endif
            mtlCount++;
            diffColor = false;
        } else {	//Unknown keyword
#if defined(DEBUG) || defined(_DEBUG)
            cout<<"Discarding mtlLine: "<<mtlTmp[0]<<"..."<<mtlLine<<" "<<endl;
#endif
        }
    }
    mtlTmp.clear();

    return true;
}