#include "main.hpp"
#include "Camera.hpp"
#include "console.hpp"
#include "ObjParser.hpp"
#include "Vertex.hpp"
#include "d3dUtil.hpp"
#include <iostream>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) {
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    toConsole();
#endif

    ShowCursor(false);

    Direct3DApp theApp(hInstance);

    theApp.initApp();

    return theApp.run();
}

Direct3DApp::Direct3DApp(HINSTANCE hInstance) : D3DApp(hInstance), m_FX(0), m_tech(0),
    m_vertexLayout(0), m_fxWVPVar(0), m_fxWorldVar(0), m_fxTexMtxVar(0), m_fxTextMapVar(0), m_tempSRV(0) {
    D3DXMatrixIdentity(&m_worldViewProjection);
    D3DXMatrixIdentity(&m_TexMtx);
    D3DXMatrixIdentity(&m_worldMatrix);
}

Direct3DApp::~Direct3DApp() {
    if( m_d3dDevice )
    { m_d3dDevice->ClearState(); }

    ReleaseCOM(m_FX);
    ReleaseCOM(m_vertexLayout);
}

void Direct3DApp::initApp() {
    D3DApp::initApp();
    m_objParser.init(m_d3dDevice);

    LPWSTR *argumentList;
    string fileArgument = "";
    int argumentCount;

    argumentList = CommandLineToArgvW(GetCommandLine(), &argumentCount);
    if (argumentList == NULL) {
        MessageBox(NULL, L"Unable to parse command line", L"Error", MB_OK);
    } else if(argumentList[1] == NULL) {
        fileArgument = "..\\data\\spikeball.obj";
    } else {
        convertLPWtoString(fileArgument, argumentList[1]);
    }

#if defined( DEBUG ) || defined( _DEBUG )
    __int64 start = getTimeMs64();
#endif

    if(m_objParser.parse(fileArgument, false, true) == false) {
        cout<<"OBJ file loading failed"<<endl;
    }

#if defined( DEBUG ) || defined( _DEBUG )
    __int64 end = getTimeMs64();
    __int64 res = end-start;
    printf("C-STYLE EXEC TIME: %I64d\n", res);
#endif

    m_ParallelLight.dir = D3DXVECTOR3(0.57735f, -0.57735f, 0.57735f);
    m_ParallelLight.ambient = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
    m_ParallelLight.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
    m_ParallelLight.specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

    Camera::CameraInstance()->position() = D3DXVECTOR3(0.0f, 0.0f, -20.0f);

    buildFX();
    buildVertexLayouts();
}

void Direct3DApp::onResize() {
    D3DApp::onResize();

    float aspect = (float)m_clientWidth/m_clientHeight;
    Camera::CameraInstance()->setLens(0.25f*PI, aspect, 0.5f, 1000.0f);
}

void Direct3DApp::updateScene(float dt) {
    D3DApp::updateScene(dt);

    // Update angles based on input to orbit camera around box.
    if(GetAsyncKeyState('A') & 0x8000)	{ Camera::CameraInstance()->strafe(-40.0f*dt); }
    if(GetAsyncKeyState('D') & 0x8000)	{ Camera::CameraInstance()->strafe(+40.0f*dt); }
    if(GetAsyncKeyState('W') & 0x8000)	{ Camera::CameraInstance()->walk(+40.0f*dt); }
    if(GetAsyncKeyState('S') & 0x8000)	{ Camera::CameraInstance()->walk(-40.0f*dt); }

    m_EyePos = Camera::CameraInstance()->position();
    Camera::CameraInstance()->rebuildView();
}

void Direct3DApp::drawScene() {
    D3DApp::drawScene();
    float blendFactors[] = {0.0f, 0.0f, 0.0f, 0.0f};
    D3D10_TECHNIQUE_DESC techDesc;
    D3DXMATRIX texMtx;

    m_d3dDevice->OMSetDepthStencilState(0, 0);
    m_d3dDevice->OMSetBlendState(0, blendFactors, 0xffffffff);
    m_d3dDevice->IASetInputLayout(m_vertexLayout);
    m_d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Calculate projection matrix
    m_fxEyePosVar->SetRawValue(&m_EyePos, 0, sizeof(D3DXVECTOR3));
    m_fxLightVar->SetRawValue(&m_ParallelLight, 0, sizeof(Light));
    m_worldViewProjection = m_worldMatrix * Camera::CameraInstance()->getviewMatrix() * Camera::CameraInstance()->getprojectionMatrix();

    //Pass variables to shader
    m_fxWVPVar->SetMatrix((float*)&m_worldViewProjection);
    m_fxWorldVar->SetMatrix((float*)&m_worldMatrix);
    m_fxTextMapVar->SetResource(m_tempSRV);

    D3DXMatrixIdentity(&texMtx);
    m_fxTexMtxVar->SetMatrix((float*)&texMtx);

    m_tech->GetDesc( &techDesc );

    for(UINT i = 0; i < techDesc.Passes; ++i) {
        m_tech->GetPassByIndex( i )->Apply(0);
        m_objParser.draw();
    }

    // We specify DT_NOCLIP, so we do not care about width/height of the rect.
    RECT R = {5, 5, 0, 0};
    m_d3dDevice->RSSetState(0);
    m_font->DrawText(0, m_frameStats.c_str(), -1, &R, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

    m_swapChain->Present(0, 0);
}

void Direct3DApp::buildFX() {
    DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
    shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

    ID3D10Blob *compilationErrors = 0;
    HRESULT hr = 0;
    hr = D3DX10CreateEffectFromFile(L"shader.fx", 0, 0,
                                    "fx_4_0", shaderFlags, 0, m_d3dDevice, 0, 0, &m_FX, &compilationErrors, 0);
    if(FAILED(hr)) {
        if( compilationErrors ) {
            MessageBoxA(0, (char*)compilationErrors->GetBufferPointer(), 0, 0);
            ReleaseCOM(compilationErrors);
        }
        DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX10CreateEffectFromFile", true);
    }

    m_tech = m_FX->GetTechniqueByName("TexTech");

    m_fxWVPVar = m_FX->GetVariableByName("g_WVP")->AsMatrix();
    m_fxWorldVar = m_FX->GetVariableByName("g_World")->AsMatrix();
    m_fxTexMtxVar = m_FX->GetVariableByName("g_TexMtx")->AsMatrix();
    m_fxTextMapVar = m_FX->GetVariableByName("g_DiffuseMap")->AsShaderResource();
    m_fxLightVar = m_FX->GetVariableByName("g_Light");
    m_fxEyePosVar = m_FX->GetVariableByName("g_EyePosW");
}

void Direct3DApp::buildVertexLayouts() {
    // Create the vertex input layout.
    D3D10_INPUT_ELEMENT_DESC vertexDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",	 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 }
    };

    UINT numElements = sizeof( vertexDesc ) / sizeof( vertexDesc[0] );

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    HRESULT hr = m_tech->GetPassByIndex(0)->GetDesc(&PassDesc);
    if(FAILED(hr)) {
        cout<<"Shader error"<<endl;
    }

    HR(m_d3dDevice->CreateInputLayout(vertexDesc, numElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &m_vertexLayout));
}

LRESULT Direct3DApp::msgProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    POINT mousePos;
    int dx = 0;
    int dy = 0;

    //Record the window area
    //GetClipCursor(&m_oldClip);
    //GetWindowRect(m_hMainWnd, &m_clip);

    //ClipCursor(&m_clip);

    switch(msg) {
        case WM_MOUSEMOVE:
            mousePos.x = (int)LOWORD(lParam);
            mousePos.y = (int)HIWORD(lParam);

            dx = mousePos.x - m_oldMousePos.x;
            dy = mousePos.y - m_oldMousePos.y;

            Camera::CameraInstance()->pitch( dy * 0.0087266f );
            Camera::CameraInstance()->rotateY( dx * 0.0087266f );

            m_oldMousePos = mousePos;
            return 0;
    }
    //ClipCursor(&m_oldClip);

    return D3DApp::msgProc(msg, wParam, lParam);
}