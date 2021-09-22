#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>
#include <vector>
#include <imgui-1.49/imgui.h>
#include <tinyxml2/tinyxml2.h>


//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------
struct Node;

uint64_t GetNextId();

///////////////////////////////////////////////////////////////////////////////
// NodeType enum
//////////////////////////////////////////////////////////////////////////////
enum NodeType
{
    Function,
    Texture,
    Constant,
    StageOutput,
};

///////////////////////////////////////////////////////////////////////////////
// SlotType enum
///////////////////////////////////////////////////////////////////////////////
enum SlotType
{
    Input,
    Output,
};

///////////////////////////////////////////////////////////////////////////////
// TextureDimension enum
///////////////////////////////////////////////////////////////////////////////
enum TextureDimension
{
    None,
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube,
    Texture1DArray,
    Texture2DArray,
    TextureCubeArray,
};

///////////////////////////////////////////////////////////////////////////////
// SamplerType enum
///////////////////////////////////////////////////////////////////////////////
enum SamplerType
{
    PointWrap,
    PointClamp,
    PointMirror,
    LinearWrap,
    LinearClamp,
    LinearMirror,
    AnisotropicWrap,
    AnisotropicClamp,
    AnisotropicMirror,
};

///////////////////////////////////////////////////////////////////////////////
// DataType enum
///////////////////////////////////////////////////////////////////////////////
enum DataType
{
    Float1,
    Float2,
    Float3,
    Float4,
};

///////////////////////////////////////////////////////////////////////////////
// Slot structure
///////////////////////////////////////////////////////////////////////////////
struct Slot
{
    SlotType    Kind    = SlotType::Input;
    DataType    Type    = DataType::Float1;
    Node*       pOwner  = nullptr;
    Slot*       pPrev   = nullptr;
    Slot*       pNext   = nullptr;
    std::string Tag;

    ImVec2      Pos    = ImVec2(0, 0);  // 描画位置.
    ImGuiID     Id     = 0;             // ImGuiでの判別用.
    uint64_t    VarId  = 0;             // 変数番号.

    std::string GenVarName() const;

    Slot(SlotType kind, DataType type, const char* tag, Node* owner)
    : Kind  (kind)
    , Type  (type)
    , pOwner(owner)
    , Tag   (tag)
    , VarId (GetNextId())
    { /* DO_NOTHING */ }
};

///////////////////////////////////////////////////////////////////////////////
// Node structure
///////////////////////////////////////////////////////////////////////////////
struct Node
{
    NodeType            Type        = NodeType::Function;
    std::vector<Slot*>  pSlots;
    std::string         Tag;
    std::string         SourceCodeTemplate;
    ImVec2              Pos         = ImVec2(0, 0);
    ImVec2              Size        = ImVec2(100, 10);

    std::string         TexturePath;
    TextureDimension    TextureDimension = TextureDimension::None;
    SamplerType         Sampler          = LinearWrap;
    float               Values[4]        = {};
    bool                AsColor          = false;

    // 一時データ ---
    ImTextureID         TextureId        = nullptr;
    //--------------


    void Reset();
    std::string GenMicroCode() const;
    void AddInput1(const char* tag); // Float1
    void AddInput2(const char* tag); // Float2
    void AddInput3(const char* tag); // Float3
    void AddInput4(const char* tag); // Float4
    void AddOutput1(const char* tag); // Float1
    void AddOutput2(const char* tag); // Float2
    void AddOutput3(const char* tag); // Float3
    void AddOutput4(const char* tag); // Float4
    void AddInput(const char* tag, DataType type);
    void AddOutput(const char* tag, DataType type);
};

///////////////////////////////////////////////////////////////////////////////
// EditData class
///////////////////////////////////////////////////////////////////////////////
class EditData
{
public:
    EditData();
    ~EditData();
    void Reset();
    void AddNode(Node* node);
    void RemoveNode(Node* node);
    std::vector<Node*>& GetNodes();
    Node* GetStageOutput();

    bool Load(const char* path);
    bool Save(const char* path);
    bool Export();

    bool FindSlot(ImGuiID slotId, Slot** slot);
    const std::string& GetShaderCode() const;
    void GenShaderCode();

private:
    std::vector<Node*>  m_pNodes;
    Node                m_StageOutput;
    std::string         m_ExportPath;
    std::string         m_ShaderCode;
    uint64_t            m_NextId;
};

