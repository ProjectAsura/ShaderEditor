//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <ShaderEditor.h>
#include <Windows.h>
#include <shlobj.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <asura_sdk/StringHelper.h>
#include <BuiltinNode.h>


namespace {

const char* SAKURA_EDITOR_PATH = "C:\\Program Files (x86)\\sakura\\sakura.exe";
const char* VS_CODE_PATH = "C:\\Program Files (x86)\\Microsoft VS Code\\Code.exe";

const ImColor NodeTagColor[] = {
    ImColor(125, 125, 255), // OBJECT_TYPE_FUNC
    ImColor(255, 125, 125), // OBJECT_TYPE_TEXTURE
    ImColor(125, 255, 125), // OBJECT_TYPE_CONSTANT
    ImColor(255, 125, 50),  // OBJECT_TYPE_STAGE_DATA
};

const ImColor FUNC_TAG_COLOR = ImColor(125, 125, 255);
const ImColor TEXTURE_TAG_COLOR = ImColor(255, 125, 125);

const ImVec4 RegisterTagColor = ImVec4(0.3f, 0.5f, 1.0f, 0.9f);

static constexpr float NODE_SLOT_RADIUS = 4.0f;
const ImVec2 NODE_WINDOW_PADDING(8.0, 8.0);

static const char* kDataType[] = {
    u8"float",
    u8"float2",
    u8"float3",
    u8"float4",
};

static const char* kSlotKind[] = {
    u8"入力スロット",
    u8"出力スロット",
};


const char* DefinedFuncName[] = {
    "abs\0",
    "acos\0",
    "all\0",
    "any\0",
    "asfloat\0",
    "asin\0",
    "asint\0",
    "asuint\0",
    "atan\0",
    "atan2\0",
    "ceil\0",
    "clamp\0",
    "clip\0",
    "cos\0",
    "cross\0",
    "ddx\0",
    "ddy\0",
    "degrees\0",
    "determinant\0",
    "distance\0",
    "dot\0",
    "exp\0",
    "exp2\0",
    "floor\0",
    "fmod\0",
    "frac\0",
    "isfinite\0",
    "isinf\0",
    "isnan\0",
    "ldexp\0",
    "length\0",
    "lerp\0",
    "log\0",
    "log10\0",
    "log2\0",
    "max\0",
    "min\0",
    "modf\0",
    "mul\0",
    "normalize\0",
    "pow\0",
    "radians\0",
    "reflect\0",
    "refract\0",
    "round\0",
    "rsqrt\0",
    "saturate\0",
    "sign\0",
    "sin\0",
    "sincos\0",
    "sinh\0",
    "smoothstep\0",
    "sqrt\0",
    "step\0",
    "tan\0",
    "tanh\0",
    "transpose\0",
    "trunc\0",
};



//-----------------------------------------------------------------------------
//      オープンファイルダイアログを開きます.
//-----------------------------------------------------------------------------
bool OpenFileDlg
(
    const char*     title,
    const char*     filter,
    const char*     ext,
    std::string&    result
)
{
    char path[MAX_PATH + 1] = {};

    OPENFILENAMEA ofn = {};
    ofn.lStructSize     = sizeof(OPENFILENAMEA);
    ofn.hwndOwner       = nullptr;
    ofn.lpstrFilter     = filter;
    ofn.lpstrFile       = path;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrDefExt     = ext;
    ofn.nMaxFileTitle   = MAX_PATH;
    ofn.lpstrTitle      = title;

    if (GetOpenFileNameA(&ofn))
    {
        result = path;
        return true;
    }

    return false;
}

int __stdcall BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    CHAR dir[MAX_PATH] = {};
    ITEMIDLIST* lpid;
    HWND hEdit;

    switch (uMsg) {
    case BFFM_INITIALIZED:
    {
        SendMessageA(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);    // コモンダイアログの初期ディレクトリ
    }
    break;

    case BFFM_VALIDATEFAILED:   // 無効なフォルダー名が入力された
    {
        MessageBoxA(hWnd, (CHAR*)lParam, "無効なフォルダー名が入力されました", MB_OK | MB_ICONERROR);
        hEdit = FindWindowExA(hWnd, NULL, "EDIT", NULL);    // エディットボックスのハンドルを取得する
        SetWindowTextA(hEdit, "");
        return 1;   // ダイアログボックスを閉じない
    }
    break;

    case BFFM_IUNKNOWN:
        break;

    case BFFM_SELCHANGED:   // 選択フォルダーが変化した場合
    {
        lpid = (ITEMIDLIST*)lParam;
        SHGetPathFromIDListA(lpid, dir);
        hEdit = FindWindowExA(hWnd, NULL, "EDIT", NULL);    // エディットボックスのハンドルを取得する
        SetWindowTextA(hEdit, dir);
    }
    break;
    }
    return 0;
}

//-----------------------------------------------------------------------------
//      フォルダ参照ダイアログを表示します.
//-----------------------------------------------------------------------------
bool OpenFolderDlg(const char* title, const char* default_dir, char* path)
{
    BROWSEINFOA bInfo = {};
    LPITEMIDLIST pIDList;

    bInfo.hwndOwner         = nullptr;  // ダイアログの親ウインドウのハンドル 
    bInfo.pidlRoot          = nullptr;  // ルートフォルダをデスクトップフォルダとする 
    bInfo.pszDisplayName    = path;     // フォルダ名を受け取るバッファへのポインタ 
    bInfo.lpszTitle         = title;    // ツリービューの上部に表示される文字列 
    bInfo.ulFlags           = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_VALIDATE | BIF_NEWDIALOGSTYLE; // 表示されるフォルダの種類を示すフラグ 
    bInfo.lpfn              = BrowseCallbackProc; // BrowseCallbackProc関数のポインタ 
    bInfo.lParam            = (LPARAM)default_dir;
    pIDList                 = SHBrowseForFolderA(&bInfo);
    if (pIDList == nullptr)
    {
        path[0] = '\0';
        return false; //何も選択されなかった場合 
    }

    if (!SHGetPathFromIDListA(pIDList, path))
    {
        return false;
    }

    CoTaskMemFree(pIDList);// pIDListのメモリを開放 
    return true;
}

//-----------------------------------------------------------------------------
//      フォルダ入力用GUI
//-----------------------------------------------------------------------------
bool ImGuiInputFolder(const char* tag, char* dir)
{
    bool result = false;
    ImGui::PushID(tag);
    if (ImGui::InputText("", dir, 512, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        result = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("..."))
    {
        if (OpenFolderDlg("フォルダを指定", dir, dir))
        {
            result = true;
        }
    }
    ImGui::SameLine();
    ImGui::LabelText(tag, "%s", tag);
    ImGui::PopID();

    return result;
}

//-----------------------------------------------------------------------------
//      情報ダイアログを表示.
//-----------------------------------------------------------------------------
bool InfoDlg(const char* title, const char* msg)
{
    return MessageBoxA(nullptr, msg, title, MB_ICONINFORMATION | MB_OK) == TRUE;
}

//-----------------------------------------------------------------------------
//      エラーダイアログを表示.
//-----------------------------------------------------------------------------
bool ErrorDlg(const char* title, const char* msg)
{
    return MessageBoxA(nullptr, msg, title, MB_ICONERROR | MB_OK) == TRUE;
}

//-----------------------------------------------------------------------------
//      ファイル入力GUI
//-----------------------------------------------------------------------------
bool ImGuiInputFile
(
    const char*     tag,
    const char*     filter,
    const char*     ext,
    std::string&    result
)
{
    bool process = false;
    ImGui::PushID(tag);

    char path[512] = {};
    strcpy_s(path, result.c_str());
    if (ImGui::InputText("", path, 512, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        process = true;
        result = path;
    }
    ImGui::SameLine();
    if (ImGui::Button("..."))
    {
        if (OpenFileDlg("ファイルを指定", filter, ext, result))
        {
            process = true;
        }
    }
    ImGui::SameLine();
    ImGui::Text("%s", tag);
    ImGui::PopID();
    return process;
}


inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
{ return ImVec2(lhs.x+rhs.x, lhs.y+rhs.y); }

inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)
{ return ImVec2(lhs.x-rhs.x, lhs.y-rhs.y); }

//-----------------------------------------------------------------------------
//      スロット接続用のドラッグ挙動処理.
//-----------------------------------------------------------------------------
bool ImGuiDragBehavior2D
(
    const ImVec2&   center,
    float           radius,
    ImGuiID         id,
    ImVec2*         pos,
    ImGuiID*        hoveredId
)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Draw Circle
    const ImU32 frame_col = ImGui::GetColorU32(
        g.ActiveId  == id ? ImGuiCol_FrameBgActive  :
        g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, frame_col);

    bool value_changed = false;

    if (g.ActiveId == id)
    {
        *hoveredId = 0;

        if (g.IO.MouseDown[0])
        {
            pos->x = g.IO.MousePos.x;
            pos->y = g.IO.MousePos.y;
            value_changed = true;
        }
        else
        {
            if (ImGui::IsAnyItemHovered())
            {
                *hoveredId = g.HoveredIdPreviousFrame;
                value_changed = true;
            }
            ImGui::SetActiveID(0, nullptr);
        }
    }

    return value_changed;
}

//-----------------------------------------------------------------------------
//      スロットGUI定義.
//-----------------------------------------------------------------------------
bool ImGuiDragSlot(Slot* slot, ImGuiID* hoverredId, bool& remove)
{
    const float NODE_SLOT_RADIUS = 4.0f;
    remove = false;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }

    const void* ptr_id = (const void*)slot;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(ptr_id);
    const float w = ImGui::CalcItemWidth();

    slot->Id = id;

    const ImVec2 label_size = ImGui::CalcTextSize(slot->Tag.c_str(), nullptr, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w + NODE_SLOT_RADIUS + style.FramePadding.x + 8.0f, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(style.ItemInnerSpacing.x, 0));

    ImVec2 center = window->DC.CursorPos;
    ImVec2 textPos;
    if (slot->Kind == SlotType::Output)
    {
        center.x  = total_bb.Max.x - NODE_SLOT_RADIUS;
        center.y  = frame_bb.Max.y - style.FramePadding.y * 2.0f - NODE_SLOT_RADIUS;
        textPos.x = center.x - label_size.x - NODE_WINDOW_PADDING.x;
        textPos.y = inner_bb.Min.y;
    }
    else
    {
        center.x  = frame_bb.Min.x + style.FramePadding.x + NODE_SLOT_RADIUS * 0.5f;
        center.y  = frame_bb.Max.y - style.FramePadding.y * 2.0f - NODE_SLOT_RADIUS;

        textPos.x = window->DC.CursorPos.x + style.FramePadding.x + NODE_SLOT_RADIUS + 8.0f;
        textPos.y = inner_bb.Min.y;
    }
    slot->Pos = center;

    if (!ImGui::ItemAdd(total_bb, id))
    {
        ImGui::ItemSize(total_bb, style.FramePadding.y);
        return false;
    }

    const bool hoverred = ImGui::IsItemHovered();
    if (hoverred)
    {
        g.HoveredId = id;
    }

    if (hoverred && (g.IO.MouseClicked[0] || g.IO.MouseDoubleClicked[0]))
    {
        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
        g.ActiveIdAllowOverlap = true;
    }

    // 削除.
    if (hoverred && g.IO.KeysDown[VK_DELETE])
    {
        remove = true;
    }

    // Actual drag behavior
    ImGui::ItemSize(total_bb, style.FramePadding.y);

    ImVec2 dst;
    const bool value_changed = ImGuiDragBehavior2D(
        center, NODE_SLOT_RADIUS, id, &dst, hoverredId);
    ImGui::RenderText(textPos, slot->Tag.c_str());

    if (value_changed && *hoverredId == 0)
    {
        if (center.x <= dst.x)
        {
            ImGui::GetWindowDrawList()->AddBezierCurve(
                center,
                center + ImVec2(+50.0f, 0.0f),
                dst    + ImVec2(-50.0f, 0.0f),
                dst,
                ImColor(255, 255, 100),
                3.0f);
        }
        else
        {
            ImGui::GetWindowDrawList()->AddBezierCurve(
                dst,
                dst    + ImVec2(+50.0f, 0.0f),
                center + ImVec2(-50.0f, 0.0f),
                center,
                ImColor(255, 255, 100),
                3.0f);
        }
    }

    return value_changed;
}

void ImGuiRect(ImVec2 size_arg, ImVec4 color)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(u8"Rect");

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(
        size_arg, style.FramePadding.x * 2.0f, style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return;

    // Render
    const ImU32 col = ImColor(color.x, color.y, color.z, color.w);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
}

//-----------------------------------------------------------------------------
//      グリッドを描画します.
//-----------------------------------------------------------------------------
void DrawGrid(ImVec2 offset)
{
    ImU32 lineColor = ImColor(250, 250, 250, 40);
    auto  gridSize  = 32.0f;

    auto drawList   = ImGui::GetWindowDrawList();
    auto winPos     = ImGui::GetCursorScreenPos();
    auto canvasSize = ImGui::GetWindowSize();

    // 水平方向.
    for (auto x = fmodf(offset.x, gridSize); x < canvasSize.x; x += gridSize)
    {
        drawList->AddLine(
            winPos + ImVec2(x, 0.0f),
            winPos + ImVec2(x, canvasSize.y),
            lineColor);
    }

    // 垂直方向.
    for (auto y = fmodf(offset.y, gridSize); y < canvasSize.y; y += gridSize)
    {
        drawList->AddLine(
            winPos + ImVec2(0.0f, y),
            winPos + ImVec2(canvasSize.x, y),
            lineColor);
    }
}

//-----------------------------------------------------------------------------
//      リンクの接続線を描画します.
//-----------------------------------------------------------------------------
void DrawLink(Link& link)
{
    auto drawList = ImGui::GetWindowDrawList();
    drawList->AddBezierCurve(
        link.Lhs->Pos,
        link.Lhs->Pos + ImVec2(50.0f, 0.0f),
        link.Rhs->Pos - ImVec2(50.0f, 0.0f),
        link.Rhs->Pos,
        ImColor(200, 200, 200),
        3.0f);
}

//-----------------------------------------------------------------------------
//      名前チェック.
//-----------------------------------------------------------------------------
bool CheckName(const char* name)
{
    if (strlen(name) == 0)
    {
        ErrorDlg("エラー", "名前が空欄のままです.\n名前を入力してください");
        return false;
    }

    if (!StringHelper::IsVariable(name))
    {
        ErrorDlg("エラー", "名前が不適切です.\n名前は[a-zA-z_]+[a-zA-z0-9]*の形式で入力してください.");
        return false;
    }

    return true;
}

} // namespace


///////////////////////////////////////////////////////////////////////////////
// Editor class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Editor::Editor()
{ 
    auto node = m_EditData.GetStageOutput();
    node->Pos.x = 800;
    node->Pos.y = 400;
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Editor::~Editor()
{ m_EditData.Reset(); }

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void Editor::Render(uint32_t w, uint32_t h)
{
    m_Size = ImVec2(float(w), float(h));

    // 編集パネル.
    DrawEditPanel();

    // プレビューパネル.
    DrawPreviewPanel();

    // プロパティパネル.
    DrawPropPanel();
}

//-----------------------------------------------------------------------------
//      編集パネルを描画します.
//-----------------------------------------------------------------------------
void Editor::DrawEditPanel()
{
    bool openContextMenu = false;

    auto& nodes = m_EditData.GetNodes();

    ImVec2 panelSize = ImVec2(m_Size.x - 400.0f, m_Size.y);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(panelSize, ImGuiCond_Once);

    if (!ImGui::Begin(u8"ノードエディタ"))
    {
        ImGui::End();
        return;
    }

    ImGui::BeginGroup();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImU32(ImColor(50, 50, 50, 200)));

    int flag = 0;
    flag |= ImGuiWindowFlags_NoScrollbar;
    flag |= ImGuiWindowFlags_NoMove;

    ImGui::BeginChild(u8"背景", ImVec2(0, 0), true, flag);
    ImGui::PushItemWidth(120.0);

    ImVec2 offset;

    // 編集パネル.
    {
        offset = ImGui::GetCursorScreenPos() - m_Scroll;
        auto drawList = ImGui::GetWindowDrawList();

        // 2レイヤーに分割.
        drawList->ChannelsSplit(2);

        // グリッドを描画.
        DrawGrid(offset);

        // ノードを描画.
        for (size_t i = 0; i < nodes.size(); ++i)
        { DrawNode(nodes[i], offset, openContextMenu); }

        // ステージ出力を描画.
        DrawNode(m_EditData.GetStageOutput(), offset, openContextMenu);

        // 接続確定後にリンクを描画.
        for(size_t i=0; i<m_Links.size(); ++i)
        { DrawLink(m_Links[i]); }

        // レイヤーを合成.
        drawList->ChannelsMerge();

    } // 編集パネル おしまい.

    // コンテキストメニュー
    {
        // 右クリックされたかどうか?
        if (!ImGui::IsAnyItemHovered()
          //&& ImGui::IsMouseHoveringWindow()
          && ImGui::IsMouseClicked(1))
        {
            m_pSelectedNode = nullptr;
            openContextMenu = true;
            m_GeneratePos = ImGui::GetMousePos();
        }

        // コンテキストメニューを開く処理.
        if (openContextMenu)
        { ImGui::OpenPopup("ContextMenu"); }

        // コンテキストメニューを描画.
        DrawContextMenu();
    }

    // スクロール量を更新.
    if (ImGui::IsWindowHovered()
    && !ImGui::IsAnyItemActive()
    &&  ImGui::IsMouseDragging(2, 0.0f))
    {
        auto& scroll = m_Scroll;
        scroll = scroll - ImGui::GetIO().MouseDelta;
    }

    ImGui::PopItemWidth();
    ImGui::EndChild();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    ImGui::EndGroup();
    ImGui::End();

    //m_SelectedId = (m_pSelectedNode != nullptr) ? m_pSelectedNode->Id : -1;
}

//-----------------------------------------------------------------------------
//      ノードを描画します.
//-----------------------------------------------------------------------------
void Editor::DrawNode(Node* node, ImVec2& offset, bool& openContextMenu)
{
    auto drawList = ImGui::GetWindowDrawList();
   
    // IDを設定.
    ImGui::PushID(node);
    {
        auto rectMin = offset + node->Pos;
        auto oldAnyActive = ImGui::IsAnyItemActive();

        // 前景レイヤーに切り替え.
        drawList->ChannelsSetCurrent(1);

        // スロット位置を設定.
        ImGui::SetCursorScreenPos(rectMin + NODE_WINDOW_PADDING);

        // グループ開始.
        ImGui::BeginGroup();
        {
            auto color = NodeTagColor[(int)node->Type];

            // ノード名.
            ImGui::TextColored(color, "%s", node->Tag.c_str());

            // スロットを描画.
            for(size_t idx=0; idx<node->pSlots.size(); ++idx)
            { DrawSlot(node->pSlots[idx]); }

            if (node->Type == NodeType::Constant)
            {
                if (node->AsColor)
                {
                    auto color = ImVec4(node->Values[0], node->Values[1], node->Values[2], node->Values[3]);
                    ImGuiRect(ImVec2(64, 64), color);
                }
                else
                {
                    switch(node->pSlots[0]->Type)
                    {
                    case DataType::Float1:
                        {
                            ImGui::Text("%.3f", node->Values[0]);
                        }
                        break;

                    case DataType::Float2:
                        {
                            ImGui::Text("x : %.3f", node->Values[0]);
                            ImGui::Text("y : %.3f", node->Values[1]);
                        }
                        break;

                    case DataType::Float3:
                        {
                            ImGui::Text("x : %.3f", node->Values[0]);
                            ImGui::Text("y : %.3f", node->Values[1]);
                            ImGui::Text("z : %.3f", node->Values[2]);
                        }
                        break;

                    case DataType::Float4:
                        {
                            ImGui::Text("x : %.3f", node->Values[0]);
                            ImGui::Text("y : %.3f", node->Values[1]);
                            ImGui::Text("z : %.3f", node->Values[2]);
                            ImGui::Text("w : %.3f", node->Values[3]);
                        }
                        break;
                    }
                }
            }
            else if (node->Type == NodeType::Texture)
            {
                ImGui::Image(node->TextureId, ImVec2(64, 64));
            }
        }
        ImGui::EndGroup();

        // ノード矩形のサイズを決定.
        node->Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;

        auto rectMax = rectMin + node->Size;
        auto widgetsActive = (!oldAnyActive && ImGui::IsAnyItemActive());

        // 背景レイヤーに切り替え.
        drawList->ChannelsSetCurrent(0);

        // 矩形位置を設定.
        ImGui::SetCursorScreenPos(rectMin);

        // ドラッグ移動できるように設定.
        ImGui::InvisibleButton("Node", node->Size);

        if (ImGui::IsItemHovered())
        {
            m_pHoveredNode = node;
            openContextMenu |= ImGui::IsMouseClicked(1);
        }

        bool movingActive = ImGui::IsItemActive();

        // 選択ノード更新.
        if (widgetsActive || movingActive)
        { m_pSelectedNode = node; }

        // ドラッグ移動.
        if (movingActive && ImGui::IsMouseDragging(0))
        { node->Pos = node->Pos + ImGui::GetIO().MouseDelta; }

        bool match = false;
        if (m_pSelectedNode != nullptr)
        { match = m_pSelectedNode == node; }

        bool active = match;

        // 塗りつぶし色決定
        ImU32 bgColor = (active) ? ImColor(80, 80, 80) : ImColor(0, 0, 0);

        // 矩形の背景
        drawList->AddRectFilled(
            rectMin,
            rectMax,
            bgColor,
            2.0f);

        // 矩形の枠線.
        drawList->AddRect(
            rectMin,
            rectMax,
            ImColor(100, 100, 100),
            2.0f);
    }
    ImGui::PopID();
}

//-----------------------------------------------------------------------------
//      スロットを描画.
//-----------------------------------------------------------------------------
void Editor::DrawSlot(Slot* slot)
{
    ImGuiID targetId = 0;

    auto window = ImGui::GetCurrentWindow();
    bool remove = false;

    // スロット描画.
    auto ret = ImGuiDragSlot(
        slot,
        &targetId,
        remove);

    // 複数つながっているときに困るので入力ピンに刺さっている側から削除させない.
    if (remove && slot->Kind == SlotType::Input)
    { RemoveLink(slot); }

    if (!ret)
    { return; }

    // 接続された場合で，さらに自分自身でない場合.
    if ((targetId != 0) && (targetId != slot->Id))
    {
        Slot* targetSlot;
        if (!m_EditData.FindSlot(targetId, &targetSlot))
        {
            return;
        }

        // スロットタイプが一緒なら接続しない.
        if (targetSlot->Kind == slot->Kind)
        {
            return;
        }

        // 既に入力スロットは1つしか入力を受け付けない。
        // そのため，入力スロットに既に接続がある場合は除外する.
        if (slot->Kind == SlotType::Output && targetSlot->pPrev != nullptr
         || slot->Kind == SlotType::Input  && targetSlot->pNext != nullptr)
        {
            return;
        }

        // データ型をチェックする.
        if (slot->Type != targetSlot->Type)
        {
            ErrorDlg("接続不能", "データ型が異なるため接続できません\n");
            return;
        }

        // 入力と出力を決定.
        if (targetSlot->Kind == SlotType::Input)
        { AddLink(slot, targetSlot); }
        else
        { AddLink(targetSlot, slot); }
    }
    else if (slot->Kind == SlotType::Input)
    {
        RemoveLink(slot);
    }
}

//-----------------------------------------------------------------------------
//      コンテキストメニューを表示します.
//-----------------------------------------------------------------------------
void Editor::DrawContextMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, NODE_WINDOW_PADDING);
    if (ImGui::BeginPopup("ContextMenu"))
    {
        if (m_pSelectedNode != nullptr && m_pSelectedNode->Type != NodeType::StageOutput)
        {
            if (ImGui::MenuItem(u8"ノード削除"))
            {
                for(size_t i=0; i<m_pSelectedNode->pSlots.size(); ++i)
                {
                    RemoveLink(m_pSelectedNode->pSlots[i]);
                }
                m_EditData.RemoveNode(m_pSelectedNode);
                m_pSelectedNode = nullptr;
            }
        }
        else
        {
            if (ImGui::MenuItem(u8"新規作成"))
            {
            }

            if (ImGui::MenuItem(u8"ファイルを開く"))
            {
            }

            if (ImGui::MenuItem(u8"上書き保存"))
            {
            }

            if (ImGui::MenuItem(u8"名前をつけて保存"))
            {
            }

            if (ImGui::MenuItem(u8"シェーダコンパイル"))
            {

            }

            if (ImGui::MenuItem(u8"シェーダ出力"))
            {
                if (m_EditData.Export())
                { InfoDlg("シェーダ出力成功", "シェーダを出力しました!"); }
                else
                { ErrorDlg("シェーダ出力失敗", "シェーダの出力に失敗しました..."); }
            }

            ImGui::Separator();

            if (ImGui::BeginMenu(u8"ノードを追加"))
            {
                if (ImGui::BeginMenu(u8"定数"))
                {
                    DrawConstantNodeMenu();
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(u8"パッキング"))
                {
                    DrawPackingNodeMenu();
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(u8"演算子"))
                {
                    DrawOperatorNodeMenu(); 
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(u8"組み込み関数"))
                {
                    DrawBuiltinFuncNodeMenu();
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(u8"テクスチャ"))
                {
                    DrawTextureNodeMenu();
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(u8"プリセット関数"))
                {
                    DrawPresetFuncNodeMenu();
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
}

//-----------------------------------------------------------------------------
//      演算子ノードのコンテキストメニューを表示します.
//-----------------------------------------------------------------------------
void Editor::DrawOperatorNodeMenu()
{
    if (ImGui::BeginMenu("operator +"))
    {
        if (ImGui::MenuItem("float"))
        {
            auto node = OpAdd(DataType::Float1);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float2"))
        {
            auto node = OpAdd(DataType::Float2);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float3"))
        {
            auto node = OpAdd(DataType::Float3);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float4"))
        {
            auto node = OpAdd(DataType::Float4);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("operator -"))
    {
        if (ImGui::MenuItem("float"))
        {
            auto node = OpSub(DataType::Float1);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float2"))
        {
            auto node = OpSub(DataType::Float2);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float3"))
        {
            auto node = OpSub(DataType::Float3);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float4"))
        {
            auto node = OpSub(DataType::Float4);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("operator *"))
    {
        if (ImGui::MenuItem("float"))
        {
            auto node = OpMul(DataType::Float1);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float2"))
        {
            auto node = OpMul(DataType::Float2);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float3"))
        {
            auto node = OpMul(DataType::Float3);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float4"))
        {
            auto node = OpMul(DataType::Float4);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("operator /"))
    {
        if (ImGui::MenuItem("float"))
        {
            auto node = OpDiv(DataType::Float1);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float2"))
        {
            auto node = OpDiv(DataType::Float2);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float3"))
        {
            auto node = OpDiv(DataType::Float3);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }
        else if (ImGui::MenuItem("float4"))
        {
            auto node = OpDiv(DataType::Float4);
            node->Pos = m_GeneratePos;
            m_EditData.AddNode(node);
        }

        ImGui::EndMenu();
    }
}

//-----------------------------------------------------------------------------
//      パッキングノードのコンテキストメニューを表示します.
//-----------------------------------------------------------------------------
void Editor::DrawPackingNodeMenu()
{
    if (ImGui::MenuItem("ToFloat2"))
    {
        auto node = ToFloat2();
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("ToFloat3"))
    {
        auto node = ToFloat3();
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("ToFloat4"))
    {
        auto node = ToFloat4();
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("FromFloat2"))
    {
        auto node = FromFloat2();
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("FromFloat3"))
    {
        auto node = FromFloat3();
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("FromFloat4"))
    {
        auto node = FromFloat4();
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
}

//-----------------------------------------------------------------------------
//      定数ノードのコンテキストメニューを表示します.
//-----------------------------------------------------------------------------
void Editor::DrawConstantNodeMenu()
{
    if (ImGui::MenuItem("float"))
    {
        auto node = ConstantValue(DataType::Float1);
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("float2"))
    {
        auto node = ConstantValue(DataType::Float2);
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("float3"))
    {
        auto node = ConstantValue(DataType::Float3);
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("float4"))
    {
        auto node = ConstantValue(DataType::Float4);
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("color3"))
    {
        auto node = Color3();
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
    else if (ImGui::MenuItem("color4"))
    {
        auto node = Color4();
        node->Pos = m_GeneratePos;
        m_EditData.AddNode(node);
    }
}

//-----------------------------------------------------------------------------
//      テクスチャノードのコンテキストメニューを表示します.
//-----------------------------------------------------------------------------
void Editor::DrawTextureNodeMenu()
{
    if (ImGui::MenuItem(u8"Sample"))
    {
    }
    else if (ImGui::MenuItem(u8"SampleLevel"))
    {
    }
    else if (ImGui::MenuItem(u8"SampleGrad"))
    {
    }
    else if (ImGui::MenuItem(u8"Load"))
    {
    }
}

//-----------------------------------------------------------------------------
//      組み込み関数ノードのコンテキストメニューを表示します.
//-----------------------------------------------------------------------------
void Editor::DrawBuiltinFuncNodeMenu()
{
}

//-----------------------------------------------------------------------------
//      プリセット関数ノードのコンテキストメニューを表示します.
//-----------------------------------------------------------------------------
void Editor::DrawPresetFuncNodeMenu()
{
}

//-----------------------------------------------------------------------------
//      プレビューパネルを描画します.
//-----------------------------------------------------------------------------
void Editor::DrawPreviewPanel()
{
    ImGui::SetNextWindowPos(ImVec2(m_Size.x - 400, 0.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400.0f, 420.0f), ImGuiCond_Once);
    ImGui::Begin(u8"プレビュー");
    ImGui::Image(m_Preview, ImVec2(390, 390));
    ImGui::End();
}

//-----------------------------------------------------------------------------
//      プロパティパネルを描画します.
//-----------------------------------------------------------------------------
void Editor::DrawPropPanel()
{
    ImGui::SetNextWindowPos(ImVec2(m_Size.x - 400, 420.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400.0f, m_Size.y - 420.0f), ImGuiCond_Once);
    ImGui::Begin(u8"プロパティ");

    auto node = m_pSelectedNode;

    // 選択されていなければ終了.
    if (node == nullptr)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), u8"NONE");
        ImGui::End();
        return;
    }

    switch(node->Type)
    {
    case NodeType::Function:
        {
            ImGui::Text(u8"関数ノード");
            ImGui::Text(u8"関数名：%s", node->Tag.c_str());
            for(size_t i=0; i<node->pSlots.size(); ++i)
            {
                auto slot = node->pSlots[i];
                ImGui::Text(u8"%s : %s %s", kSlotKind[slot->Kind], kDataType[slot->Type], slot->Tag.c_str());
            }
        }
        break;

    case NodeType::Constant:
        {
            auto slot = node->pSlots[0];
            ImGui::Text(u8"定数ノード");
            if (slot->Type == DataType::Float1)
            {
                ImGui::Text(u8"データ型：float");
                ImGui::DragFloat(u8"値", &node->Values[0], 0.1f);
            }
            else if (slot->Type == DataType::Float2)
            {
                ImGui::Text(u8"データ型：float2");
                ImGui::DragFloat2(u8"値", node->Values, 0.1f);
            }
            else if (slot->Type == DataType::Float3)
            {
                ImGui::Text(u8"データ型：float3");
                if (node->AsColor)
                {
                    int flags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel;
                    ImGui::ColorPicker3(u8"色", node->Values, flags);
                }
                else
                {
                    ImGui::DragFloat3(u8"値", node->Values, 0.1f);
                }
            }
            else if (slot->Type == DataType::Float4)
            {
                ImGui::Text(u8"データ型：float4");
                if (node->AsColor)
                {
                    int flags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar;
                    ImGui::ColorPicker4(u8"色", node->Values, flags);
                }
                else
                {
                    ImGui::DragFloat4(u8"値", node->Values, 0.1f);
                }
            }
        }
        break;

    case NodeType::Texture:
        {
            ImGui::Text(u8"テクスチャノード");
            if (node->TextureDimension == TextureDimension::Texture1D)
            {
                ImGui::Text(u8"データ型：Texture1D");
            }
            else if (node->TextureDimension == TextureDimension::Texture2D)
            {
                ImGui::Text(u8"データ型：Texture2D");
            }
            else if (node->TextureDimension == TextureDimension::Texture3D)
            {
                ImGui::Text(u8"データ型：Texture3D");

            }
            else if (node->TextureDimension == TextureDimension::TextureCube)
            {
                ImGui::Text(u8"データ型：TextureCube");

            }
            else if (node->TextureDimension == TextureDimension::Texture1DArray)
            {
                ImGui::Text(u8"データ型：Texture1DArray");

            }
            else if (node->TextureDimension == TextureDimension::Texture2DArray)
            {
                ImGui::Text(u8"データ型：Texture2DArray");

            }
            else if (node->TextureDimension == TextureDimension::TextureCubeArray)
            {
                ImGui::Text(u8"データ型：TextureCubeArray");

            }
        }
        break;

    case NodeType::StageOutput:
        {
            ImGui::Text(u8"ステージ出力ノード");
            for(size_t i=0; i<node->pSlots.size(); ++i)
            {
                auto slot = node->pSlots[i];
                ImGui::Text(u8"%s : %s %s", kSlotKind[slot->Kind], kDataType[slot->Type], slot->Tag.c_str());
            }
        }
        break;
    }


    ImGui::End();
}

//-----------------------------------------------------------------------------
//      リンクを追加します.
//-----------------------------------------------------------------------------
void Editor::AddLink(Slot* lhs, Slot* rhs)
{
    lhs->pNext = rhs;
    rhs->pPrev = lhs;

    auto find = false;
    for(size_t i=0; i<m_Links.size(); ++i)
    {
        if (m_Links[i].Lhs == lhs && m_Links[i].Rhs == rhs)
        {
            find = true;
            break;
        }
    }

    if (!find)
    {
        Link link;
        link.Lhs = lhs;
        link.Rhs = rhs;
        m_Links.push_back(link);
    }
}

//-----------------------------------------------------------------------------
//      リンクを削除します.
//-----------------------------------------------------------------------------
void Editor::RemoveLink(Slot* slot)
{
    auto itr = m_Links.begin();
    while(itr != m_Links.end())
    {
        if (itr->Lhs == slot ||
            itr->Rhs == slot)
        {
            itr->Lhs->pNext = nullptr;
            itr->Rhs->pPrev = nullptr;
            itr = m_Links.erase(itr);
            break;
        }

        itr++;
    }
}
