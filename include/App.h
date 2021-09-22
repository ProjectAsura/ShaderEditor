//-------------------------------------------------------------------------------------------------
// File : App.h
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <ShaderEditor.h>

template<typename T>
using RefPtr = Microsoft::WRL::ComPtr<T>;

///////////////////////////////////////////////////////////////////////////////////////////////////
// MOUSE_BUTTON enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum MOUSE_BUTTON
{
    MOUSE_BUTTON_L = 0x0,       //!< 左ボタンです.
    MOUSE_BUTTON_R,             //!< 右ボタンです.
    MOUSE_BUTTON_M,             //!< 中ボタンです.
    MOUSE_BUTTON_X1,            //!< サイドボタン1です.
    MOUSE_BUTTON_X2,            //!< サイドボタン2です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// KEYCODE enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum KEYCODE
{
    KEY_RETURN = 0x80,      //!< リターンキーです.
    KEY_TAB,                //!< Tabキーです.
    KEY_ESC,                //!< Escapeキーです.
    KEY_BACK,               //!< BackSpaceキーです.
    KEY_SHIFT,              //!< Shiftキーです.
    KEY_CONTROL,            //!< Controlキーです.
    KEY_ALT,                //!< Altキーです.
    KEY_F1,                 //!< F1キーです.
    KEY_F2,                 //!< F2キーです.
    KEY_F3,                 //!< F3キーです.
    KEY_F4,                 //!< F4キーです.
    KEY_F5,                 //!< F5キーです.
    KEY_F6,                 //!< F6キーです.
    KEY_F7,                 //!< F7キーです.
    KEY_F8,                 //!< F8キーです.
    KEY_F9,                 //!< F9キーです.
    KEY_F10,                //!< F10キーです.
    KEY_F11,                //!< F11キーです.
    KEY_F12,                //!< F12キーです.
    KEY_UP,                 //!< ↑キーです.
    KEY_DOWN,               //!< ↓キーです.
    KEY_LEFT,               //!< ←キーです.
    KEY_RIGHT,              //!< →キーです.
    KEY_NUM0,               //!< Num0キーです.
    KEY_NUM1,               //!< Num1キーです.
    KEY_NUM2,               //!< Num2キーです.
    KEY_NUM3,               //!< Num3キーです.
    KEY_NUM4,               //!< Num4キーです.
    KEY_NUM5,               //!< Num5キーです.
    KEY_NUM6,               //!< Num6キーです.
    KEY_NUM7,               //!< Num7キーです.
    KEY_NUM8,               //!< Num8キーです.
    KEY_NUM9,               //!< Num9キーです.
    KEY_INSERT,             //!< Insertキーです.
    KEY_DELETE,             //!< Deleteキーです.
    KEY_HOME,               //!< Homeキーです.
    KEY_END,                //!< Endキーです.
    KEY_PAGE_UP,            //!< PageUpキーです.
    KEY_PAGE_DOWN,          //!< PageDownキーです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MouseEvent structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MouseEvent
{
    int     CursorX;        //!< マウスカーソルのX座標です.
    int     CursorY;        //!< マウスカーソルのY座標です.
    int     WheelDelta;     //!< マウスホイールの移動量です.
    bool    IsDown[5];      //!< ボタンが押されているかどうか?
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// KeyEvent structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct KeyEvent
{
    bool        IsKeyDown;      //!< キーが押されているかどうか?
    bool        IsAltDown;      //!< ALTキーがおされているかどうか?
    uint32_t    KeyCode;        //!< キーコードです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TypingEvent structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TypingEvent
{
    uint32_t    KeyCode;        //!< キーコードです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResizerEvent structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResizeEvent
{
    uint32_t    Width;          //!< ウィンドウの横幅です.
    uint32_t    Height;         //!< ウィンドウの縦幅です.
    float       AspectRatio;    //!< ウィンドウのアスペクト比です.
};

//-------------------------------------------------------------------------------------------------
// Type definitions.
//-------------------------------------------------------------------------------------------------
typedef void (*MouseHandler)   (const MouseEvent&  args, void* pUser);
typedef void (*KeyboardHandler)(const KeyEvent&    args, void* pUser);
typedef void (*TypingHandler)  (const TypingEvent& args, void* pUser);
typedef void (*ResizeHandler)  (const ResizeEvent& args, void* pUser);


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////
class App
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    App();
    ~App();

    void Run();

private:
    static constexpr uint32_t SwapChainCount = 2;

    //=============================================================================================
    // private variables.
    //=============================================================================================
    HINSTANCE                       m_hInst             = nullptr;
    HWND                            m_hWnd              = nullptr;
    HDC                             m_hDC               = nullptr;
    D3D_FEATURE_LEVEL               m_FeatureLevel      = D3D_FEATURE_LEVEL_11_0;
    D3D_DRIVER_TYPE                 m_DriverType        = D3D_DRIVER_TYPE_HARDWARE;
    RefPtr<ID3D11Device>            m_pDevice           = nullptr;
    RefPtr<ID3D11DeviceContext>     m_pDeviceContext    = nullptr;
    RefPtr<IDXGISwapChain>          m_pSwapChain        = nullptr;
    RefPtr<ID3D11Texture2D>         m_pDSB              = nullptr;
    RefPtr<ID3D11DepthStencilView>  m_pDSV              = nullptr;
    RefPtr<ID3D11RenderTargetView>  m_pRTV              = nullptr;
    uint32_t                        m_Width             = 1920;
    uint32_t                        m_Height            = 1080;
    D3D11_VIEWPORT                  m_Viewport          = { 0.0f, 0.0f, 900.0f, 540.0f, 0.0f, 1.0f };
    D3D11_RECT                      m_ScissorRect       = { 0, 900, 0, 540 };
    RefPtr<ID3D11RasterizerState>   m_pRS               = nullptr;
    Editor                          m_Editor;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    bool Init();
    void Term();
    void MainLoop();

    bool InitWnd();
    void TermWnd();
    bool InitD3D11();
    void TermD3D11();

    bool OnInitApp();
    void OnTermApp();
    void OnMouse(const MouseEvent& args);
    void OnKeyboard(const KeyEvent& args);
    void OnTyping(const TypingEvent& args);
    void OnResize(const ResizeEvent& args);
    void OnDrop(uint32_t count, const std::vector<std::wstring>& files);
    void OnRender();

    static LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};
