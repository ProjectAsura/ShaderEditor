#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d11.h>
#include <EditData.h>
#include <imgui-1.49/imgui.h>


///////////////////////////////////////////////////////////////////////////////
// Link structure
///////////////////////////////////////////////////////////////////////////////
struct Link
{
    Slot* Lhs = nullptr;
    Slot* Rhs = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
// Editor class
///////////////////////////////////////////////////////////////////////////////
class Editor
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    Editor();
    ~Editor();
    void Render(uint32_t w, uint32_t h);

    void SetPreview(ImTextureID preview) 
    { m_Preview = preview; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    EditData            m_EditData;                 //!< 編集データ.
    ImVec2              m_Size;                     //!< ウィンドウサイズ.
    Node*               m_pSelectedNode = nullptr;  //!< 選択済みノード.
    Node*               m_pHoveredNode  = nullptr;  //!< ホバーノード.
    std::string         m_FilePath;                 //!< 中間ファイルパス.
    ImVec2              m_GeneratePos;              //!< ノード生成位置.
    ImVec2              m_Scroll;                   //!< スクロール.
    std::vector<Link>   m_Links;
    ImTextureID         m_Preview = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================
    void DrawEditPanel();
    void DrawPropPanel();
    void DrawPreviewPanel();

    void DrawNode(Node* node, ImVec2& offset, bool& openContextMenu);
    void DrawSlot(Slot* slot);
    void DrawContextMenu();

    void AddLink(Slot* lhs, Slot* rhs);
    void RemoveLink(Slot* slot);

    void DrawOperatorNodeMenu();
    void DrawTextureNodeMenu();
    void DrawPackingNodeMenu();
    void DrawConstantNodeMenu();
    void DrawBuiltinFuncNodeMenu();
    void DrawPresetFuncNodeMenu();
};