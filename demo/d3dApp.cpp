#include "d3dApp.h"
#include <sstream>

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static D3DApp *app = 0;

    switch( msg ) {
        case WM_CREATE: {
                // Get the 'this' pointer we passed to CreateWindow via the lpParam parameter.
                CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
                app = (D3DApp*)cs->lpCreateParams;
                return 0;
            }
    }

    // Don't start processing messages until after WM_CREATE.
    if( app ) {
        return app->msgProc(msg, wParam, lParam);
    } else {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

}

D3DApp::D3DApp(HINSTANCE hInstance) : m_d3dDevice(0), m_swapChain(0), m_depthStencilBuffer(0), m_depthStencilView(0), m_renderTargetView(0),
    m_font(0), m_hAppInst(0), m_hMainWnd(0), m_appPaused(false), m_minimized(false), m_maximized(false), m_resizing(false), m_frameStats(L""),
    m_mainWndCaption(L"demo"), m_d3dDriverType(D3D10_DRIVER_TYPE_HARDWARE), m_clearColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f)), m_clientWidth(1600), m_clientHeight(900) {}	//window size


D3DApp::~D3DApp() {
    ReleaseCOM(m_renderTargetView);
    ReleaseCOM(m_depthStencilView);
    ReleaseCOM(m_swapChain);
    ReleaseCOM(m_depthStencilBuffer);
    ReleaseCOM(m_d3dDevice);
    ReleaseCOM(m_font);
}

HINSTANCE D3DApp::getAppInst() {
    return m_hAppInst;
}

HWND D3DApp::getMainWnd() {
    return m_hMainWnd;
}

int D3DApp::run() {
    MSG msg = {0};
    m_timer.reset();

    while(msg.message != WM_QUIT) {
        // If there are Window messages then process them.
        if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE )) {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        // Otherwise, do animation/game stuff.
        else {
            m_timer.tick();

            if( !m_appPaused ) {
                updateScene(m_timer.getDeltaTime());
            } else {
                Sleep(50);
            }

            drawScene();
        }
    }
    return (int)msg.wParam;
}

void D3DApp::initApp() {
    initMainWindow();
    initDirect3D();

    D3DX10_FONT_DESC fontDesc;
    fontDesc.Height          = 24;
    fontDesc.Width           = 0;
    fontDesc.Weight          = 0;
    fontDesc.MipLevels       = 1;
    fontDesc.Italic          = false;
    fontDesc.CharSet         = DEFAULT_CHARSET;
    fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
    fontDesc.Quality         = DEFAULT_QUALITY;
    fontDesc.PitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;
    wcscpy(fontDesc.FaceName, L"Times New Roman");

    D3DX10CreateFontIndirect(m_d3dDevice, &fontDesc, &m_font);
}

void D3DApp::onResize() {
    // Release the old views, as they hold references to the buffers we
    // will be destroying.  Also release the old depth/stencil buffer
    ReleaseCOM(m_renderTargetView);
    ReleaseCOM(m_depthStencilView);
    ReleaseCOM(m_depthStencilBuffer);


    // Resize the swap chain and recreate the render target view
    HR(m_swapChain->ResizeBuffers(1, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
    ID3D10Texture2D *backBuffer;
    HR(m_swapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&backBuffer)));
    HR(m_d3dDevice->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView));
    ReleaseCOM(backBuffer);


    // Create the depth/stencil buffer and view
    D3D10_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width     = m_clientWidth;
    depthStencilDesc.Height    = m_clientHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count   = 8;	// multisampling must match
    depthStencilDesc.SampleDesc.Quality = 16;	// swap chain values
    depthStencilDesc.Usage          = D3D10_USAGE_DEFAULT;
    depthStencilDesc.BindFlags      = D3D10_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags      = 0;

    HR(m_d3dDevice->CreateTexture2D(&depthStencilDesc, 0, &m_depthStencilBuffer));
    HR(m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer, 0, &m_depthStencilView));


    // Bind the render target view and depth/stencil view to the pipeline
    m_d3dDevice->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);


    // Set the viewport transform
    D3D10_VIEWPORT vp;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width    = m_clientWidth;
    vp.Height   = m_clientHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    m_d3dDevice->RSSetViewports(1, &vp);
}

void D3DApp::updateScene(float dt) {
    // Code computes the average frames per second, and also the
    // average time it takes to render one frame.

    static int frameCnt = 0;
    static float t_base = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if( (m_timer.getGameTime() - t_base) >= 1.0f ) {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        std::wostringstream outs;
        outs.precision(6);
        outs << L"FPS: " << fps << L"\n"
             << "Milliseconds: Per Frame: " << mspf;
        m_frameStats = outs.str();

        // Reset for next average.
        frameCnt = 0;
        t_base  += 1.0f;
    }
}

void D3DApp::drawScene() {
    m_d3dDevice->ClearRenderTargetView(m_renderTargetView, m_clearColor);
    m_d3dDevice->ClearDepthStencilView(m_depthStencilView, D3D10_CLEAR_DEPTH|D3D10_CLEAR_STENCIL, 1.0f, 0);
}

LRESULT D3DApp::msgProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch( msg ) {
            // Check if the user hit the escape key
        case VK_ESCAPE:
            PostQuitMessage(0);
            exit(1);
            return 0;

            // WM_ACTIVATE is sent when the window is activated or deactivated.
            // We pause the game when the window is deactivated and unpause it
            // when it becomes active.
        case WM_ACTIVATE:
            if( LOWORD(wParam) == WA_INACTIVE ) {
                m_appPaused = true;
                m_timer.stop();
            } else {
                m_appPaused = false;
                m_timer.start();
            }
            return 0;

            // WM_SIZE is sent when the user resizes the window.
        case WM_SIZE:
            // Save the new client area dimensions.
            m_clientWidth  = LOWORD(lParam);
            m_clientHeight = HIWORD(lParam);
            if( m_d3dDevice ) {
                if( wParam == SIZE_MINIMIZED ) {
                    m_appPaused = true;
                    m_minimized = true;
                    m_maximized = false;
                } else if( wParam == SIZE_MAXIMIZED ) {
                    m_appPaused = false;
                    m_minimized = false;
                    m_maximized = true;
                    onResize();
                } else if( wParam == SIZE_RESTORED ) {

                    // Restoring from minimized state?
                    if( m_minimized ) {
                        m_appPaused = false;
                        m_minimized = false;
                        onResize();
                    }

                    // Restoring from maximized state?
                    else if( m_maximized ) {
                        m_appPaused = false;
                        m_maximized = false;
                        onResize();
                    } else if( m_resizing ) {
                        // If user is dragging the resize bars, we do not resize
                        // the buffers here because as the user continuously
                        // drags the resize bars, a stream of WM_SIZE messages are
                        // sent to the window, and it would be pointless (and slow)
                        // to resize for each WM_SIZE message received from dragging
                        // the resize bars.  So instead, we reset after the user is
                        // done resizing the window and releases the resize bars, which
                        // sends a WM_EXITSIZEMOVE message.
                    } else { // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                        onResize();
                    }
                }
            }
            return 0;

            // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
        case WM_ENTERSIZEMOVE:
            m_appPaused = true;
            m_resizing  = true;
            m_timer.stop();
            return 0;

            // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
            // Here we reset everything based on the new window dimensions.
        case WM_EXITSIZEMOVE:
            m_appPaused = false;
            m_resizing  = false;
            m_timer.start();
            onResize();
            return 0;

            // WM_DESTROY is sent when the window is being destroyed.
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

            // The WM_MENUCHAR message is sent when a menu is active and the user presses
            // a key that does not correspond to any mnemonic or accelerator key.
        case WM_MENUCHAR:
            // Don't beep when we alt-enter.
            return MAKELRESULT(0, MNC_CLOSE);

            // Catch this message so to prevent the window from becoming too small.
        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            return 0;
    }

    return DefWindowProc(m_hMainWnd, msg, wParam, lParam);
}


void D3DApp::initMainWindow() {
    WNDCLASS wc;
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = m_hAppInst;
    wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName  = 0;
    wc.lpszClassName = L"D3DWndClassName";

    if( !RegisterClass(&wc) ) {
        MessageBox(0, L"RegisterClass FAILED", 0, 0);
        PostQuitMessage(0);
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = { 0, 0, m_clientWidth, m_clientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width  = R.right - R.left;
    int height = R.bottom - R.top;

    m_hMainWnd = CreateWindow(L"D3DWndClassName",
                              m_mainWndCaption.c_str(),
                              WS_OVERLAPPEDWINDOW,
                              620, 240,	//CW_USEDEFAULT, CW_USEDEFAULT,
                              width, height,
                              0,
                              0,
                              m_hAppInst,
                              this);
    if( !m_hMainWnd ) {
        MessageBox(0, L"CreateWindow FAILED", 0, 0);
        PostQuitMessage(0);
    }

    ShowWindow(m_hMainWnd, SW_SHOW);	//SW_MAXIMIZE
    UpdateWindow(m_hMainWnd);
}

void D3DApp::initDirect3D() {
    //swap chain.
    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width  = m_clientWidth;
    sd.BufferDesc.Height = m_clientHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count   = 8;
    sd.SampleDesc.Quality = 16;
    sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount  = 1;
    sd.OutputWindow = m_hMainWnd;
    sd.Windowed     = true;
    sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags        = 0;

    // Create the m_d3dDevice(interface handle)
    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

    HR( D3D10CreateDeviceAndSwapChain(0, m_d3dDriverType, 0, createDeviceFlags, D3D10_SDK_VERSION, &sd, &m_swapChain, &m_d3dDevice) );

    // The remaining steps that need to be carried out for d3d creation
    // also need to be executed every time the window is resized.  So
    // just call the onResize method here to avoid code duplication.
    onResize();
}
