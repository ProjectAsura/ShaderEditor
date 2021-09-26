//-------------------------------------------------------------------------------------------------
// File : Gui.h
// Desc : Gui Manager.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>
#include <chrono>
#include <d3d11.h>
#include <wrl/client.h>

//-------------------------------------------------------------------------------------------------
// Forward Declarations.
//-------------------------------------------------------------------------------------------------
struct ImDrawData;


///////////////////////////////////////////////////////////////////////////////////////////////////
// GuiMgr class
///////////////////////////////////////////////////////////////////////////////////////////////////
class GuiMgr
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

    //---------------------------------------------------------------------------------------------
    // Static
    //---------------------------------------------------------------------------------------------
    static constexpr uint32_t   MaxPrimitiveCount = 6 * 1024;
    static GuiMgr               s_Instance;

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    static GuiMgr& GetInstance();
    bool Init( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd, uint32_t width, uint32_t height );
    void Term();
    void Update( uint32_t width, uint32_t height );
    void Draw();
    void OnMouse( int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR );
    void OnKey( bool isDown, bool isAltDown, uint32_t code );
    void OnTyping( uint32_t code );

private:
    //=============================================================================================
    // private varaibles.
    //=============================================================================================
    Microsoft::WRL::ComPtr<ID3D11Device>                m_pDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>         m_pContext;
    Microsoft::WRL::ComPtr<ID3D11Buffer>                m_pVB;
    Microsoft::WRL::ComPtr<ID3D11Buffer>                m_pIB;
    Microsoft::WRL::ComPtr<ID3D11Buffer>                m_pCB;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_pSmp;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_pTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_pSRV;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>       m_pRS;
    Microsoft::WRL::ComPtr<ID3D11BlendState>            m_pBS;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState>     m_pDSS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>           m_pIL;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>          m_pVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>           m_pPS;
    std::chrono::system_clock::time_point               m_LastTime;
    uint32_t                                            m_SizeVB;
    uint32_t                                            m_SizeIB;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    GuiMgr();
    ~GuiMgr();

    void OnDraw( ImDrawData* draw_data );
};