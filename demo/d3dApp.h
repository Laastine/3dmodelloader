#ifndef D3DAPP_H
#define D3DAPP_H

#include "d3dUtil.h"
#include "GameTimer.h"
#include <string>

class D3DApp {
    public:
        D3DApp(HINSTANCE hInstance);
        virtual ~D3DApp();

        HINSTANCE getAppInst();
        HWND      getMainWnd();

        int run();

        virtual void initApp();				// init app
        virtual void onResize();			// reset projection/etc
        virtual void updateScene(float dt);	//Update loop
        virtual void drawScene();			//Call D3D objects to do magic tricks
        virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);	//Win32 window handler

    protected:
        void initMainWindow();
        void initDirect3D();

    protected:
        HINSTANCE m_hAppInst;
        HWND m_hMainWnd;
        bool m_appPaused;
        bool m_minimized;
        bool m_maximized;
        bool m_resizing;

        GameTimer m_timer;

        std::wstring m_frameStats;

        ID3D10Device *m_d3dDevice;					//Device interface
        IDXGISwapChain *m_swapChain;				//Swapchain interface for double buffering
        ID3D10Texture2D *m_depthStencilBuffer;		//2D texture interface
        ID3D10RenderTargetView *m_renderTargetView;	//Resource management
        ID3D10DepthStencilView *m_depthStencilView;	//Texture acces during depth-stencil testing
        ID3DX10Font *m_font;						//Encapsulates the textures and resources needed to text

        // Derived class should set these in derived constructor to customize starting values.
        std::wstring m_mainWndCaption;
        D3D10_DRIVER_TYPE m_d3dDriverType;
        D3DXCOLOR m_clearColor;
        int m_clientWidth;
        int m_clientHeight;
};

#endif // D3DAPP_H