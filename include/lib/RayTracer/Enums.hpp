#pragma once
namespace EvoEngine {
    enum class MaterialType {
        Default,
        VertexColor,
        CompressedBTF
    };

    enum class RendererType {
        Default,
        Instanced,
        Skinned,
        Curve
    };

    enum class PrimitiveType {
        Custom,
        QuadraticBSpline,
        CubicBSpline,
        Linear,
        CatmullRom,
        Triangle
    };
}