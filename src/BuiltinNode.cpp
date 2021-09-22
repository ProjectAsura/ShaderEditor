
#include <BuiltinNode.h>

const std::string kType[] = {
    std::string("float"),
    std::string("float2"),
    std::string("float3"),
    std::string("float4"),
};


Node* Sample1D()
{
    std::string code;
    code += "float4 %Output0 = %Texture.Sample(%Sampler, %Input0);\n";

    auto node = new Node();
    node->Type = NodeType::Texture;
    node->Tag = "Sample1D";
    node->AddInput1("texcoord");
    node->AddOutput4("result");
    node->TextureDimension = TextureDimension::Texture1D;
    node->SourceCodeTemplate = code;

    return node;
}

Node* Sample2D()
{
    std::string code;
    code += "float4 %Output0 = %Texture.Sample(%Sampler, %Input0);\n";

    auto node = new Node();
    node->Type = NodeType::Texture;
    node->Tag = "Sample2D";
    node->AddInput2("texcoord");
    node->AddOutput4("result");
    node->TextureDimension = TextureDimension::Texture2D;
    node->SourceCodeTemplate = code;

    return node;
}

Node* ConstantValue(DataType type)
{
    std::string code;

    if (type == DataType::Float1)
    {
        code += "const float %Output0 = %Value0;\n";
    }
    else if (type == DataType::Float2)
    {
        code += "const float2 %Output0 = float2(%Value0, %Value1);\n";
    }
    else if (type == DataType::Float3)
    {
        code += "const float3 %Output0 = float3(%Value0, %Value1, %Value2);\n";
    }
    else if (type == DataType::Float4)
    {
        code += "const float4 %Output0 = float4(%Value0, %Value1, %Value2, %Value3);\n";
    }

    auto node = new Node();
    node->Type = NodeType::Constant;
    node->Tag = "Constant";
    node->AddOutput("value", type);
    node->SourceCodeTemplate = code;

    return node;
}

Node* Color3()
{
    std::string code;
    code += "const float3 %Output0 = float3(%Value0, %Value1, %Value2);\n";

    auto node = new Node();
    node->Type = NodeType::Constant;
    node->Tag = "Color3";
    node->AddOutput3("color");
    node->SourceCodeTemplate = code;
    node->AsColor = true;
    node->Values[0] = 1.0f;
    node->Values[1] = 1.0f;
    node->Values[2] = 1.0f;
    node->Values[3] = 1.0f;

    return node;
}

Node* Color4()
{
    std::string code;
    code += "const float4 %Output0 = float4(%Value0, %Value1, %Value2, %Value3);\n";

    auto node = new Node();
    node->Type = NodeType::Constant;
    node->Tag = "Color4";
    node->AddOutput4("color");
    node->SourceCodeTemplate = code;
    node->AsColor = true;
    node->Values[0] = 1.0f;
    node->Values[1] = 1.0f;
    node->Values[2] = 1.0f;
    node->Values[3] = 1.0f;

    return node;
}

Node* FromFloat2()
{
    std::string code;
    code += "float %Output0 = %Input0.x;\n";
    code += "float %Output1 = %Input0.y;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "FromFloat2";
    node->AddInput2("input");
    node->AddOutput1("x");
    node->AddOutput1("y");
    node->SourceCodeTemplate = code;

    return node;
}

Node* FromFloat3()
{
    std::string code;
    code += "float %Output0 = %Input0.x;\n";
    code += "float %Output1 = %Input0.y;\n";
    code += "float %Output2 = %Input0.z;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "FromFloat3";
    node->AddInput3("input");
    node->AddOutput1("x");
    node->AddOutput1("y");
    node->AddOutput1("z");
    node->SourceCodeTemplate = code;

    return node;
}

Node* FromFloat4()
{
    std::string code;
    code += "float %Output0 = %Input0.x;\n";
    code += "float %Output1 = %Input0.y;\n";
    code += "float %Output2 = %Input0.z;\n";
    code += "float %Output3 = %Input0.w;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "FromFloat4";
    node->AddInput4("input");
    node->AddOutput1("x");
    node->AddOutput1("y");
    node->AddOutput1("z");
    node->AddOutput1("w");
    node->SourceCodeTemplate = code;

    return node;
}

Node* ToFloat2()
{
    std::string code;
    code += "float2 %Output0 = float2(%Input0, %Input1);\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "ToFloat2";
    node->AddInput1("x");
    node->AddInput1("y");
    node->AddOutput2("output");
    node->SourceCodeTemplate = code;

    return node;
}

Node* ToFloat3()
{
    std::string code;
    code += "float3 %Output0 = float3(%Input0, %Input1, %Input2);\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "ToFloat3";
    node->AddInput1("x");
    node->AddInput1("y");
    node->AddInput1("z");
    node->AddOutput3("output");
    node->SourceCodeTemplate = code;

    return node;
}

Node* ToFloat4()
{
    std::string code;
    code += "float4 %Output0 = float4(%Input0, %Input1, %Input2, %Input3);\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "ToFloat4";
    node->AddInput1("x");
    node->AddInput1("y");
    node->AddInput1("z");
    node->AddInput1("w");
    node->AddOutput4("output");
    node->SourceCodeTemplate = code;

    return node;
}

Node* OpAdd(DataType type)
{
    std::string code;
    code += kType[type] + " %Output0 = %Input0 + %Input1;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "operator +";
    node->AddInput("lhs", type);
    node->AddInput("rhs", type);
    node->AddOutput("output", type);
    node->SourceCodeTemplate = code;

    return node;
}

Node* OpSub(DataType type)
{
    std::string code;
    code += kType[type] + " %Output0 = %Input0 - %Input1;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "operator -";
    node->AddInput("lhs", type);
    node->AddInput("rhs", type);
    node->AddOutput("output", type);
    node->SourceCodeTemplate = code;

    return node;
}

Node* OpMul(DataType type)
{
    std::string code;
    code += kType[type] + " %Output0 = %Input0 * %Input1;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "operator *";
    node->AddInput("lhs", type);
    node->AddInput("rhs", type);
    node->AddOutput("output", type);
    node->SourceCodeTemplate = code;

    return node;
}

Node* OpDiv(DataType type)
{
    std::string code;
    code += kType[type] + " %Output0 = %Input0 / %Input1;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "operator /";
    node->AddInput("lhs", type);
    node->AddInput("rhs", type);
    node->AddOutput("output", type);
    node->SourceCodeTemplate = code;

    return node;
}

Node* GetTexCoord0()
{
    std::string code;
    code += "float2 %Output0 = input.TexCoord0;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "TexCoord0";
    node->AddOutput2("uv");
    node->SourceCodeTemplate = code;

    return node;
}

Node* GetTexCoord1()
{
    std::string code;
    code += "float2 %Output0 = input.TexCoord1;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "TexCoord1";
    node->AddOutput2("uv");
    node->SourceCodeTemplate = code;

    return node;
}

Node* GetTexCoord2()
{
    std::string code;
    code += "float2 %Output0 = input.TexCoord2;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "TexCoord2";
    node->AddOutput2("uv");
    node->SourceCodeTemplate = code;

    return node;
}

Node* GetTexCoord3()
{
    std::string code;
    code += "float2 %Output0 = input.TexCoord3;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "TexCoord3";
    node->AddOutput2("uv");
    node->SourceCodeTemplate = code;

    return node;
}


Node* GetGeometryNormal()
{
    std::string code;
    code += "float3 %Output0 = geometry.Normal;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "Geometry Normal";
    node->AddOutput3("normal");
    node->SourceCodeTemplate = code;

    return node;
}

Node* GetGeometryTangent()
{
    std::string code;
    code += "float3 %Output0 = geometry.Tangent;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "Geometry Tangent";
    node->AddOutput3("tangent");
    node->SourceCodeTemplate = code;

    return node;
}


Node* GetGeometryBitangent()
{
    std::string code;
    code += "float3 %Output0 = geometry.Binormal;\n";

    auto node = new Node();
    node->Type = NodeType::Function;
    node->Tag = "Geometry Bitangent";
    node->AddOutput3("bitangent");
    node->SourceCodeTemplate = code;

    return node;
}