//-------------------------------------------------------------------------------------------------
// File : App.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "App.h"
#include "Gui.h"
#include <imgui-1.49/imgui.h>

//-------------------------------------------------------------------------------------------------
// Constant Values
//-------------------------------------------------------------------------------------------------
constexpr wchar_t*      AppClassName = L"ShaderEditor";
constexpr DXGI_FORMAT   SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
App::App()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
App::~App()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      実行します.
//-------------------------------------------------------------------------------------------------
void App::Run()
{
    if (Init())
    { MainLoop(); }

    Term();
}

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::Init()
{
    if(!InitWnd())
    { return false; }

    if(!InitD3D11())
    { return false; }

    if(!OnInitApp())
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void App::Term()
{
    if (m_pDeviceContext)
    {
        m_pDeviceContext->ClearState();
        m_pDeviceContext->Flush();
    }

    OnTermApp();
    TermD3D11();
    TermWnd();
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの初期化です.
//-------------------------------------------------------------------------------------------------
bool App::InitWnd()
{
    // インスタンスハンドルを取得.
    HINSTANCE hInst = GetModuleHandle( nullptr );
    if ( !hInst )
    {
        return false;
    }

    // 拡張ウィンドウクラスの設定.
    WNDCLASSEXW wc;
    wc.cbSize           = sizeof( WNDCLASSEXW );
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MsgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(nullptr, IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = AppClassName;
    wc.hIconSm          = LoadIcon(nullptr, IDI_APPLICATION);

    // ウィンドウクラスを登録します.
    if ( !RegisterClassExW( &wc ) )
    {
        // 異常終了.
        return false;
    }

    // インスタンスハンドルを設定.
    m_hInst = hInst;

    // 矩形の設定.
    RECT rc = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

    // 指定されたクライアント領域を確保するために必要なウィンドウ座標を計算します.
    DWORD style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect( &rc, style, FALSE );

    // ウィンドウを生成します.
    m_hWnd = CreateWindowW(
        AppClassName,
        L"ShaderEditor",
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        ( rc.right - rc.left ),
        ( rc.bottom - rc.top ),
        NULL,
        NULL,
        hInst,
        this
    );

    // 生成チェック.
    if ( !m_hWnd )
    {
        // 異常終了.
        return false;
    }

    // ウィンドウを表示します.
    ShowWindow( m_hWnd, SW_SHOWNORMAL );
    UpdateWindow( m_hWnd );

    // フォーカスを設定します.
    SetFocus( m_hWnd );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの終了処理です.
//-------------------------------------------------------------------------------------------------
void App::TermWnd()
{
    if (m_hInst != nullptr)
    { UnregisterClassW( AppClassName, m_hInst); }

    m_hInst = nullptr;
    m_hWnd  = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      D3D11の初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::InitD3D11()
{
    HRESULT hr = S_OK;

    // ウィンドウサイズを取得します.
    RECT rc;
    GetClientRect( m_hWnd, &rc );
    UINT w = rc.right - rc.left;
    UINT h = rc.bottom - rc.top;

    // 取得したサイズを設定します.
    m_Width       = w;
    m_Height      = h;


    // デバイス生成フラグ.
    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif//defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    // ドライバータイプ.
    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTytpes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

    // 機能レベル.
    D3D_FEATURE_LEVEL featureLevels[] = {
        //D3D_FEATURE_LEVEL_11_1,       // へぼPC向け対応 [ D3D11.1に対応していないハードウェアでは，問答無用で落とされる場合があるので仕方なく除外. ]
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };
    UINT numFeatureLevels = sizeof( featureLevels ) / sizeof( featureLevels[0] );

    // スワップチェインの構成設定.
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof(DXGI_SWAP_CHAIN_DESC) );
    sd.BufferCount                          = SwapChainCount;
    sd.BufferDesc.Width                     = w;
    sd.BufferDesc.Height                    = h;
    sd.BufferDesc.Format                    = SwapChainFormat;
    sd.BufferDesc.RefreshRate.Numerator     = 60;
    sd.BufferDesc.RefreshRate.Denominator   = 1;
    sd.BufferUsage                          = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sd.OutputWindow                         = m_hWnd;
    sd.SampleDesc.Count                     = 1;
    sd.SampleDesc.Quality                   = 0;
    sd.Windowed                             = TRUE;

    for( UINT idx = 0; idx < numDriverTytpes; ++idx )
    {
        // ドライバータイプ設定.
        m_DriverType = driverTypes[ idx ];

        // デバイスとスワップチェインの生成.
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            m_DriverType,
            nullptr,
            createDeviceFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &sd,
            &m_pSwapChain,
            &m_pDevice,
            &m_FeatureLevel,
            &m_pDeviceContext
        );

        // 成功したらループを脱出.
        if ( SUCCEEDED( hr ) )
        {
            break;
        }
    }

    // 失敗していないかチェック.
    if ( FAILED( hr ) )
    {
        return false;
    }

    // レンダーターゲット生成.
    {
        RefPtr<ID3D11Texture2D> pBuffer;
        hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBuffer.GetAddressOf()));
        if (FAILED(hr))
        { return false; }

        hr = m_pDevice->CreateRenderTargetView(pBuffer.Get(), nullptr, m_pRTV.GetAddressOf());
        if (FAILED(hr))
        { return false; }
    }

    // 深度バッファ生成.
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
        desc.ArraySize          = 1;

        hr = m_pDevice->CreateTexture2D(&desc, nullptr, m_pDSB.GetAddressOf());
        if ( FAILED(hr) )
        { return false; }
    }

    // 深度ステンシルバッファ生成.
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
        desc.Format				= DXGI_FORMAT_D32_FLOAT;
        desc.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;

        hr = m_pDevice->CreateDepthStencilView(m_pDSB.Get(), &desc, m_pDSV.GetAddressOf());
        if ( FAILED(hr) )
        { return false; }
    }

    {
        D3D11_RASTERIZER_DESC desc = {};
        desc.FillMode				= D3D11_FILL_SOLID;
        desc.CullMode				= D3D11_CULL_BACK;
        desc.FrontCounterClockwise	= FALSE;
        desc.DepthClipEnable		= FALSE;
        desc.ScissorEnable			= TRUE;
        desc.MultisampleEnable		= FALSE;
        desc.AntialiasedLineEnable	= TRUE;

        hr = m_pDevice->CreateRasterizerState( &desc, m_pRS.GetAddressOf() );
        if ( FAILED(hr) )
        { return false; }
    }

    m_Viewport.TopLeftX     = 0;
    m_Viewport.TopLeftY     = 0;
    m_Viewport.Width        = float(m_Width);
    m_Viewport.Height       = float(m_Height);
    m_Viewport.MinDepth     = 0.0f;
    m_Viewport.MaxDepth     = 1.0f;

    m_ScissorRect.left      = 0;
    m_ScissorRect.right     = m_Width;
    m_ScissorRect.top       = 0;
    m_ScissorRect.bottom    = m_Height;

    m_pDeviceContext->OMSetRenderTargets( 1, m_pRTV.GetAddressOf(), m_pDSV.Get() );
    m_pDeviceContext->RSSetViewports( 1, &m_Viewport );
    m_pDeviceContext->RSSetScissorRects( 1, &m_ScissorRect );
    m_pDeviceContext->RSSetState( m_pRS.Get() );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      D3D11の終了処理です.
//-------------------------------------------------------------------------------------------------
void App::TermD3D11()
{
    m_pRTV.Reset();
    m_pDSV.Reset();
    m_pDSB.Reset();
    m_pRS.Reset();
    m_pSwapChain.Reset();
    m_pDeviceContext.Reset();
    m_pDevice.Reset();
}

//-------------------------------------------------------------------------------------------------
//      メインループです.
//-------------------------------------------------------------------------------------------------
void App::MainLoop()
{
    MSG msg = {};

    while( WM_QUIT != msg.message )
    {
        if ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            OnRender();
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      リサイズ時の処理です.
//-------------------------------------------------------------------------------------------------
void App::OnResize(const ResizeEvent& param)
{
    // ウインドウ非表示状態に移行する時に縦横1ピクセルのリサイズイベントが発行される
    // マルチサンプル等の関係で縦横1ピクセルは問題が起こるので処理をスキップ
    if ( param.Width == 1 && param.Height == 1 )
    { return; }

    m_Width       = param.Width;
    m_Height      = param.Height;

    if ( ( m_pSwapChain     != nullptr )
      && ( m_pDeviceContext != nullptr ) )
    {
        // リサイズ前に一度コマンドを実行(実行しないとメモリリークするぽい).
        m_pSwapChain->Present( 0, 0 );

        ID3D11RenderTargetView* pNull = nullptr;
        m_pDeviceContext->OMSetRenderTargets( 1, &pNull, nullptr );

        m_pRTV.Reset();
        m_pDSV.Reset();
        m_pDSB.Reset();

        HRESULT hr = S_OK;

        // バッファをリサイズ.
        hr = m_pSwapChain->ResizeBuffers( SwapChainCount, 0, 0, SwapChainFormat, 0 );
        if (FAILED(hr))
        { return; }

        {
            RefPtr<ID3D11Texture2D> pBuffer;
            auto hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBuffer.GetAddressOf()));
            if (FAILED(hr))
            { return; }

            hr = m_pDevice->CreateRenderTargetView(pBuffer.Get(), nullptr, m_pRTV.GetAddressOf());
            if (FAILED(hr))
            { return; }
        }

        // 深度バッファ生成.
        {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width              = m_Width;
            desc.Height             = m_Height;
            desc.MipLevels          = 1;
            desc.Format             = DXGI_FORMAT_D32_FLOAT;
            desc.SampleDesc.Count   = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage              = D3D11_USAGE_DEFAULT;
            desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
            desc.ArraySize          = 1;

            hr = m_pDevice->CreateTexture2D(&desc, nullptr, m_pDSB.GetAddressOf());
            if ( FAILED(hr) )
            { return; }
        }

        // 深度ステンシルバッファ生成.
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
            desc.Format             = DXGI_FORMAT_D32_FLOAT;
            desc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;

            hr = m_pDevice->CreateDepthStencilView(m_pDSB.Get(), &desc, m_pDSV.GetAddressOf());
            if ( FAILED(hr) )
            { return; }
        }

        // デバイスコンテキストにレンダーターゲットを設定.
        m_pDeviceContext->OMSetRenderTargets( 1, m_pRTV.GetAddressOf(), m_pDSV.Get() );

        // ビューポートの設定.
        m_Viewport.Width    = (FLOAT)m_Width;
        m_Viewport.Height   = (FLOAT)m_Height;
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;
        m_Viewport.TopLeftX = 0;
        m_Viewport.TopLeftY = 0;

        // シザー矩形の設定.
        m_ScissorRect.left   = 0;
        m_ScissorRect.right  = m_Width;
        m_ScissorRect.top    = 0;
        m_ScissorRect.bottom = m_Height;

        // デバイスコンテキストにビューポートを設定.
        m_pDeviceContext->RSSetViewports( 1, &m_Viewport );

        // デバイスコンテキストにシザー矩形を設定.
        m_pDeviceContext->RSSetScissorRects( 1, &m_ScissorRect );
    }
}

//-------------------------------------------------------------------------------------------------
//      アプリ固有の初期化です.
//-------------------------------------------------------------------------------------------------
bool App::OnInitApp()
{
    if (!GuiMgr::GetInstance().Init(m_pDevice.Get(), m_pDeviceContext.Get(), m_hWnd, m_Width, m_Height))
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      アプリ固有の終了処理です.
//-------------------------------------------------------------------------------------------------
void App::OnTermApp()
{
    GuiMgr::GetInstance().Term();
}

//-------------------------------------------------------------------------------------------------
//      ファイルドロップ時の処理です.
//-------------------------------------------------------------------------------------------------
void App::OnDrop(uint32_t count, const std::vector<std::wstring>& files)
{
}

//-------------------------------------------------------------------------------------------------
//      マウスの処理です.
//-------------------------------------------------------------------------------------------------
void App::OnMouse(const MouseEvent& arg)
{
    GuiMgr::GetInstance().OnMouse(
        arg.CursorX,
        arg.CursorY,
        arg.WheelDelta, 
        arg.IsDown[MOUSE_BUTTON_L],
        arg.IsDown[MOUSE_BUTTON_M],
        arg.IsDown[MOUSE_BUTTON_R]);
}

//-------------------------------------------------------------------------------------------------
//      キーボードの処理です.
//-------------------------------------------------------------------------------------------------
void App::OnKeyboard(const KeyEvent& arg)
{
    GuiMgr::GetInstance().OnKey(arg.IsKeyDown, arg.IsAltDown, arg.KeyCode);
}

//-------------------------------------------------------------------------------------------------
//      タイピングの処理です.
//-------------------------------------------------------------------------------------------------
void App::OnTyping(const TypingEvent& arg)
{
    GuiMgr::GetInstance().OnTyping(arg.KeyCode);
}

//-------------------------------------------------------------------------------------------------
//      レンダリングの処理です.
//-------------------------------------------------------------------------------------------------
void App::OnRender()
{
    if (m_pRTV.Get() == nullptr || m_pDSV.Get() == nullptr)
    { return; }

    float clearColor[] = { 0.0, 0.0, 0.0, 1.0f };

    // クリア処理.
    m_pDeviceContext->ClearRenderTargetView( m_pRTV.Get(), clearColor );
    m_pDeviceContext->ClearDepthStencilView( m_pDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    // 出力マネージャに設定.
    m_pDeviceContext->OMSetRenderTargets( 1, m_pRTV.GetAddressOf(), m_pDSV.Get() );

    GuiMgr::GetInstance().Update(m_Width, m_Height);
    {
        m_Editor.Render(m_Width, m_Height);
    }
    GuiMgr::GetInstance().Draw();

    m_pSwapChain->Present( 1, 0 );
}

//-------------------------------------------------------------------------------------------------
//      メッセージコールバックです.
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK App::MsgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto instance = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    PAINTSTRUCT ps;
    HDC         hdc;

    if ( ( msg == WM_KEYDOWN )
      || ( msg == WM_SYSKEYDOWN )
      || ( msg == WM_KEYUP )
      || ( msg == WM_SYSKEYUP ) )
    {
        bool isKeyDown = ( msg == WM_KEYDOWN  || msg == WM_SYSKEYDOWN );

        DWORD mask = ( 1 << 29 );
        bool isAltDown =( ( lp & mask ) != 0 );

        KeyEvent args;
        args.KeyCode   = uint32_t( wp );
        args.IsAltDown = isAltDown;
        args.IsKeyDown = isKeyDown;

		instance->OnKeyboard(args);
    }

    // 古いWM_MOUSEWHEELの定義.
    const UINT OLD_WM_MOUSEWHEEL = 0x020A;

    if ( ( msg == WM_LBUTTONDOWN )
      || ( msg == WM_LBUTTONUP )
      || ( msg == WM_LBUTTONDBLCLK )
      || ( msg == WM_MBUTTONDOWN )
      || ( msg == WM_MBUTTONUP )
      || ( msg == WM_MBUTTONDBLCLK )
      || ( msg == WM_RBUTTONDOWN )
      || ( msg == WM_RBUTTONUP )
      || ( msg == WM_RBUTTONDBLCLK )
      || ( msg == WM_XBUTTONDOWN )
      || ( msg == WM_XBUTTONUP )
      || ( msg == WM_XBUTTONDBLCLK )
      || ( msg == WM_MOUSEHWHEEL )             // このWM_MOUSEWHEELは0x020Eを想定.
      || ( msg == WM_MOUSEMOVE )
      || ( msg == OLD_WM_MOUSEWHEEL ) )
    {
        int x = (short)LOWORD( lp );
        int y = (short)HIWORD( lp );

        int wheelDelta = 0;
        if ( ( msg == WM_MOUSEHWHEEL )
          || ( msg == OLD_WM_MOUSEWHEEL ) )
        {
            POINT pt;
            pt.x = x;
            pt.y = y;

            ScreenToClient( hWnd, &pt );
            x = pt.x;
            y = pt.y;

            wheelDelta += (short)HIWORD( wp );
        }

        int  buttonState = LOWORD( wp );
        bool isLeftButtonDown   = ( ( buttonState & MK_LBUTTON  ) != 0 );
        bool isRightButtonDown  = ( ( buttonState & MK_RBUTTON  ) != 0 );
        bool isMiddleButtonDown = ( ( buttonState & MK_MBUTTON  ) != 0 );
        bool isSideButton1Down  = ( ( buttonState & MK_XBUTTON1 ) != 0 );
        bool isSideButton2Down  = ( ( buttonState & MK_XBUTTON2 ) != 0 );

        MouseEvent args = {};
        args.CursorX = x;
        args.CursorY = y;
        args.WheelDelta = wheelDelta;
        args.IsDown[MOUSE_BUTTON_L]  = isLeftButtonDown;
        args.IsDown[MOUSE_BUTTON_M]  = isMiddleButtonDown;
        args.IsDown[MOUSE_BUTTON_R]  = isRightButtonDown;
        args.IsDown[MOUSE_BUTTON_X1] = isSideButton1Down;
        args.IsDown[MOUSE_BUTTON_X2] = isSideButton2Down;

        instance->OnMouse(args);
    }

    switch( msg )
    {
    case WM_CREATE:
        {
            // ドラッグアンドドロップ可能.
            DragAcceptFiles(hWnd, TRUE);

            auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lp);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        break;

    case WM_PAINT:
        {
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
        }
        break;

    case WM_DESTROY:
        { PostQuitMessage( 0 ); }
        break;

    case WM_SIZE:
        {
            UINT w = (UINT)LOWORD( lp );
            UINT h = (UINT)HIWORD( lp );

            ResizeEvent arg;
            arg.Width  = w;
            arg.Height = h;
            arg.AspectRatio = float(w) / float(h);
            instance->OnResize(arg);
        }
        break;

    case WM_CHAR:
        {
            TypingEvent arg = {};
            arg.KeyCode = static_cast<uint32_t>(wp);
            instance->OnTyping(arg);
        }
        break;

    case WM_DROPFILES:
        {
            // ドロップされたファイル数を取得.
            uint32_t numFiles = DragQueryFileW((HDROP)wp, 0xFFFFFFFF, NULL, 0);

            std::vector<std::wstring> files;
            files.resize(numFiles);

            for (uint32_t i=0; i < numFiles; i++)
            {
                // ドロップされたファイル名を取得.
                wchar_t temp[MAX_PATH];
                DragQueryFileW((HDROP)wp, i, temp, MAX_PATH);
                files[i] = temp;
            }

            instance->OnDrop( numFiles, files);

            // 作業用のバッファを解放.
            for(size_t i=0; i<files.size(); ++i)
            {
                files[i].clear();
            }

            files.clear();
            DragFinish((HDROP)wp);
        }
        break;
    }

    return DefWindowProc( hWnd, msg, wp, lp );
}