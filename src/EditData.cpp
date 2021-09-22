
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <EditData.h>
#include <atomic>
#include <deque>
#include <asura_sdk/StringHelper.h>


namespace {

uint64_t g_NextId = 1;

static const char* kSamplerName[] = {
    "PointWrap",
    "PointClamp",
    "PointMirror",
    "LinearWrap",
    "LinearClamp",
    "LinearMirror",
    "AnisotropicWrap",
    "AnisotropicClamp",
    "AnisotropicMirror"
};

static const char* kTextureName[] = {
    "MaterialTexture0",
    "MaterialTexture1",
    "MaterialTexture2",
    "MaterialTexture3",
    "MaterialTexture4",
    "MaterialTexture5",
    "MaterialTexture6",
    "MaterialTexture7",
    "MaterialTexture8",
    "MaterialTexture9",
    "MaterialTexture10",
    "MaterialTexture11",
    "MaterialTexture12",
    "MaterialTexture13",
    "MaterialTexture14",
    "MaterialTexture15",
};

static const char* kDefaultValueString[] = {
    "0.0f",
    "float2(0.0f, 0.0f)",
    "float3(0.0f, 0.0f, 0.0f)",
    "float4(0.0f, 0.0f, 0.0f, 0.0f)"
};

//-----------------------------------------------------------------------------
//      コード生成に必要なノードのみを収集します.
//-----------------------------------------------------------------------------
void CollectNodes(Node* node, std::deque<Node*>& result)
{
    if (node == nullptr)
    { return; }

    // ノードが有効かどうかチェック.
    auto valid = true;
    for(size_t i=0; i<node->pSlots.size(); ++i)
    {
        auto slot = node->pSlots[i];

        // 入力ピンのみを調べる.
        if (slot->Kind == SlotType::Output)
        { continue; }

        auto prevSlot = slot->pPrev;

        // 接続無し.
        if (prevSlot == nullptr)
        {
            // つながっていないピンがあるので，このノードは無効.
            valid = false;
            continue;
        }
    }

    // 無効なノード.
    if (!valid) {
        return;
    }

    // ノードを追加.
    result.push_front(node);

    for(size_t i=0; i<node->pSlots.size(); ++i)
    {
        auto slot = node->pSlots[i];

        // 入力ピンのみを調べる.
        if (slot->Kind == SlotType::Output)
        { continue; }

        auto prevSlot = slot->pPrev;
        if (prevSlot == nullptr)
        { continue; }

        CollectNodes(prevSlot->pOwner, result);
    }
}

} // namespace


//-----------------------------------------------------------------------------
//      変数IDを取得します.
//-----------------------------------------------------------------------------
uint64_t GetNextId()
{
    auto result = g_NextId;
    g_NextId++;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Slot structure
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      変数名を生成します.
//-----------------------------------------------------------------------------
std::string Slot::GenVarName() const
{
    std::string name;
    name += "var_" + std::to_string(VarId);
    return name;
}

///////////////////////////////////////////////////////////////////////////////
// Node structure
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      リセット処理を行ないます.
//-----------------------------------------------------------------------------
void Node::Reset()
{
    for(size_t i=0; i<pSlots.size(); ++i)
    {
        auto slot = pSlots[i];
        pSlots[i] = nullptr;

        if (slot != nullptr)
        { delete slot; }
    }

    pSlots.clear();
    Tag.clear();
    SourceCodeTemplate.clear();
}

//-----------------------------------------------------------------------------
//      マイクロコードを生成します.
//-----------------------------------------------------------------------------
std::string Node::GenMicroCode() const 
{
    auto code = SourceCodeTemplate;

    switch(Type)
    {
    case NodeType::Constant:
        {
            code = StringHelper::Replace(code, "%Output0", pSlots[0]->GenVarName());
            code = StringHelper::Replace(code, "%Value0", std::to_string(Values[0]));
            code = StringHelper::Replace(code, "%Value1", std::to_string(Values[1]));
            code = StringHelper::Replace(code, "%Value2", std::to_string(Values[2]));
            code = StringHelper::Replace(code, "%Value3", std::to_string(Values[3]));
        }
        break;

    case NodeType::Function:
        {
            int input = 0;
            int output = 0;
            for(size_t i=0; i<pSlots.size(); ++i)
            {
                auto slot = pSlots[i];
                char pattern[32] = {};
                std::string varName;

                if (slot->Kind == SlotType::Input)
                {
                    sprintf_s(pattern, "%sInput%d", "%", input);
                    input++;
                    varName = slot->pPrev->GenVarName();
                }
                else
                {
                    sprintf_s(pattern, "%sOutput%d", "%", output);
                    output++;
                    varName = slot->GenVarName();
                }

                code = StringHelper::Replace(code, pattern, varName);
            }
        }
        break;

    case NodeType::Texture:
        {
            code = StringHelper::Replace(code, "%Output0", pSlots[0]->GenVarName());
            code = StringHelper::Replace(code, "%Sampler", kSamplerName[Sampler]);
            code = StringHelper::Replace(code, "%Texture", kTextureName[0]);
            code = StringHelper::Replace(code, "%Input0",  pSlots[0]->pPrev->GenVarName());
        }
        break;

    case NodeType::StageOutput:
        {
            int input = 0;
            for(size_t i=0; i<pSlots.size(); ++i)
            {
                auto slot = pSlots[i];
                char pattern[32] = {};
                std::string varName;
                sprintf_s(pattern, "%sInput%d", "%", input);
                input++;

                if (slot->pPrev == nullptr)
                {
                    varName = kDefaultValueString[slot->Type];
                }
                else
                {
                    varName = slot->pPrev->GenVarName();
                }

                code = StringHelper::Replace(code, pattern, varName);
            }
        }
        break;
    }

    return code;
}

//-----------------------------------------------------------------------------
//      入力スロットを追加します.
//-----------------------------------------------------------------------------
void Node::AddInput(const char* tag, DataType type)
{
    auto slot = new Slot(SlotType::Input, type, tag, this);
    pSlots.push_back(slot);
}

//-----------------------------------------------------------------------------
//      出力スロットを追加します.
//-----------------------------------------------------------------------------
void Node::AddOutput(const char* tag, DataType type)
{
    auto slot = new Slot(SlotType::Output, type, tag, this);
    pSlots.push_back(slot);
}

//-----------------------------------------------------------------------------
//      入力スロットにfloatを追加します.
//-----------------------------------------------------------------------------
void Node::AddInput1(const char* tag)
{ AddInput(tag, DataType::Float1); }

//-----------------------------------------------------------------------------
//      入力スロットにfloat2を追加します.
//-----------------------------------------------------------------------------
void Node::AddInput2(const char* tag)
{ AddInput(tag, DataType::Float2); }

//-----------------------------------------------------------------------------
//      入力スロットにfloat3を追加します.
//-----------------------------------------------------------------------------
void Node::AddInput3(const char* tag)
{ AddInput(tag, DataType::Float3); }

//-----------------------------------------------------------------------------
//      入力スロットにfloat4を追加します.
//-----------------------------------------------------------------------------
void Node::AddInput4(const char* tag)
{ AddInput(tag, DataType::Float4); }

//-----------------------------------------------------------------------------
//      出力スロットにfloatを追加します.
//-----------------------------------------------------------------------------
void Node::AddOutput1(const char* tag)
{ AddOutput(tag, DataType::Float1); }

//-----------------------------------------------------------------------------
//      出力スロットにfloat2を追加します.
//-----------------------------------------------------------------------------
void Node::AddOutput2(const char* tag)
{ AddOutput(tag, DataType::Float2); }

//-----------------------------------------------------------------------------
//      出力スロットにfloat3を追加します.
//-----------------------------------------------------------------------------
void Node::AddOutput3(const char* tag)
{ AddOutput(tag, DataType::Float3); }

//-----------------------------------------------------------------------------
//      出力スロットにfloat4を追加します.
//-----------------------------------------------------------------------------
void Node::AddOutput4(const char* tag)
{ AddOutput(tag, DataType::Float4); }


///////////////////////////////////////////////////////////////////////////////
// EditData class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditData::EditData()
{
    std::string code;
    code += "GBuffer gbuffer;\r\n";
    code += "gbuffer.BaseColor = %Input0;\r\n";
    code += "gbuffer.Normal    = %Input1;\r\n";
    code += "gbuffer.Roughness = %Input2;\r\n";
    code += "gbuffer.Metalness = %Input3;\r\n";
    code += "gbuffer.Occlusion = %Input4;\r\n";
    code += "gbuffer.Emissive  = %Input5;\r\n";
    code += "output = EncodeGBuffer(gbuffer);\r\n";

    m_StageOutput.Type = NodeType::StageOutput;
    m_StageOutput.Tag = "Stage Output";
    m_StageOutput.AddInput3("BaseColor");
    m_StageOutput.AddInput3("Normal");
    m_StageOutput.AddInput1("Roughness");
    m_StageOutput.AddInput1("Metalness");
    m_StageOutput.AddInput1("Occlusion");
    m_StageOutput.AddInput3("Emissive");
    m_StageOutput.SourceCodeTemplate = code;

    m_ExportPath = "shader.hlsl";
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
EditData::~EditData()
{ Reset(); }

//-----------------------------------------------------------------------------
//      リセット処理を行ないます.
//-----------------------------------------------------------------------------
void EditData::Reset()
{
    for(size_t i=0; i<m_pNodes.size(); ++i)
    { m_pNodes[i]->Reset(); }

    m_pNodes.clear();
}

//-----------------------------------------------------------------------------
//      ファイルを読み込みます.
//-----------------------------------------------------------------------------
bool EditData::Load(const char* path)
{

    g_NextId = m_NextId;

    return false;
}

//-----------------------------------------------------------------------------
//      ファイルを書き込みます.
//-----------------------------------------------------------------------------
bool EditData::Save(const char* path)
{
    m_NextId = g_NextId;

    return false;
}

//-----------------------------------------------------------------------------
//      シェーダコードを生成します.
//-----------------------------------------------------------------------------
void EditData::GenShaderCode()
{
    std::string code;

    code += "//-----------------------------------------------------------------------------\r\n";
    code += "// <auto-generated>\r\n";
    code += "//     This code was generated by a tool.\r\n";
    code += "//\r\n";
    code += "//     Changes to this file may cause incorrect behavior and will be lost if\r\n";
    code += "//     the code is regenerated.\r\n";
    code += "// </auto-generated>\r\n";
    code += "//-----------------------------------------------------------------------------\r\n";
    code += "\r\n";
    code += "#include \"ShaderEditorDefine.hlsli\"\r\n";
    code += "#include \"ShaderEditorPreset.hlsli\"\r\n";
    code += "\r\n";
    code += "PSOutput main(const PSInput input)\r\n";
    code += "{\r\n";
    code += "     PSOutput output = (PSOutput)0;\r\n";
    code += "     Geometry geometry;\r\n";
    code += "     geometry.Normal    = normalize(input.Normal);\r\n";
    code += "     geometry.Tangent   = normalize(input.Tangent);\r\n";
    code += "     geometry.Bitangent = normalize(input.Bitangent);\r\n";
    code += "\r\n";

    // 自動生成コード挿入.
    {
        std::deque<Node*> validNodes;
        validNodes.push_back(&m_StageOutput);

        for(size_t i=0; i<m_StageOutput.pSlots.size(); ++i)
        {
            auto slot = m_StageOutput.pSlots[i]->pPrev;
            if (slot == nullptr)
            { continue; }

            CollectNodes(slot->pOwner, validNodes);
        }

        for(size_t i=0; i<validNodes.size(); ++i)
        { code += validNodes[i]->GenMicroCode(); }

        validNodes.clear();
    }

    code += "\r\n";
    code += "     return output;\r\n";
    code += "}\r\n";

    m_ShaderCode = code;
}

//-----------------------------------------------------------------------------
//      シェーダを出力します.
//-----------------------------------------------------------------------------
bool EditData::Export()
{
    GenShaderCode();

    // ファイルに出力.
    {
        FILE* pFile;
        auto err = fopen_s(&pFile, m_ExportPath.c_str(), "wb");
        if (err != 0)
        { return false; }

        fwrite(m_ShaderCode.c_str(), m_ShaderCode.size(), 1, pFile);
        fclose(pFile);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ノードを追加します.
//-----------------------------------------------------------------------------
void EditData::AddNode(Node* node)
{ m_pNodes.push_back(node); }

//-----------------------------------------------------------------------------
//      ノードを削除します.
//-----------------------------------------------------------------------------
void EditData::RemoveNode(Node* node)
{
    auto itr = m_pNodes.begin();
    while(itr != m_pNodes.end())
    {
        if (*itr == node)
        {
            m_pNodes.erase(itr);
            break;
        }

        itr++;
    }
}

//-----------------------------------------------------------------------------
//      ノードを取得します.
//-----------------------------------------------------------------------------
std::vector<Node*>& EditData::GetNodes()
{ return m_pNodes; }

//-----------------------------------------------------------------------------
//      ステージ出力を取得します.
//-----------------------------------------------------------------------------
Node* EditData::GetStageOutput() 
{ return &m_StageOutput; }

//-----------------------------------------------------------------------------
//      シェーダコードを生成します.
//-----------------------------------------------------------------------------
const std::string& EditData::GetShaderCode() const
{ return m_ShaderCode; }

//-----------------------------------------------------------------------------
//      スロットを検索します.
//-----------------------------------------------------------------------------
bool EditData::FindSlot(ImGuiID slotId, Slot** pResult)
{
    for(size_t i=0; i<m_pNodes.size(); ++i)
    {
        auto node = m_pNodes[i];
        for(size_t j=0; j<node->pSlots.size(); ++j)
        {
            auto slot = node->pSlots[j];
            if (slot->Id == slotId)
            {
                *pResult = slot;
                return true;
            }
        }
    }

    for(size_t i=0; i<m_StageOutput.pSlots.size(); ++i)
    {
        auto slot = m_StageOutput.pSlots[i];
        if (slot->Id == slotId)
        {
            *pResult = slot;
            return true;
        }
    }

    return false;
}
