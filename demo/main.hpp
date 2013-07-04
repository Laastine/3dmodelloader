#include "d3dApp.hpp"
#include "ObjParser.hpp"
#include "Light.hpp"

class Direct3DApp : public D3DApp {
    public:
        Direct3DApp(HINSTANCE hInstance);
        ~Direct3DApp();

        void initApp();
        void onResize();
        void updateScene(float dt);
        void drawScene();

        LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        void buildFX();
        void buildVertexLayouts();

    private:
        ObjParser m_objParser;
        Material m_material;

        Light m_ParallelLight;

        RECT m_clip;
        RECT m_oldClip;
        POINT m_oldMousePos;

        ID3D10Effect* m_FX;
        ID3D10EffectTechnique* m_tech;
        ID3D10InputLayout* m_vertexLayout;

        ID3D10EffectVariable* m_fxEyePosVar;
        ID3D10EffectMatrixVariable* m_fxWVPVar;
        ID3D10EffectMatrixVariable* m_fxWorldVar;
        ID3D10EffectMatrixVariable* m_fxTexMtxVar;
        ID3D10EffectVariable* m_fxLightVar;

        ID3D10EffectShaderResourceVariable* m_fxTextMapVar;
        ID3D10ShaderResourceView* m_tempSRV;

        D3DXVECTOR3 m_EyePos;

        D3DXMATRIX m_worldViewProjection;
        D3DXMATRIX m_worldMatrix;	//World view projection
        D3DXMATRIX m_TexMtx;
};