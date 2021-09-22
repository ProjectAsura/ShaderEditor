#pragma once

#include <EditData.h>


Node* Sample1D();
Node* Sample2D();
//Node* Sample3D();
//Node* SampleCube();
//Node* Sample1DArray();
//Node* Sample2DArray();
//Node* SampleCubeArray();

Node* ConstantValue(DataType type);
Node* Color3();
Node* Color4();

Node* FromFloat2();
Node* FromFloat3();
Node* FromFloat4();

Node* ToFloat2();
Node* ToFloat3();
Node* ToFloat4();

Node* OpAdd(DataType type);
Node* OpSub(DataType type);
Node* OpMul(DataType type);
Node* OpDiv(DataType type);

Node* GetTexCoord0();
Node* GetTexCoord1();
Node* GetTexCoord2();
Node* GetTexCoord3();

Node* GetGeometryNormal();
Node* GetGeometryTangent();
Node* GetGeometryBitangent();
