#pragma once

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"

enum ViewMatrixSource
{
    VIEW_MATRIX_SOURCE_SCENE_CAMERA = 0x01,
    VIEW_MATRIX_SOURCE_UI_CAMERA = 0x02,
    VIEW_MATRIX_SOURCE_LIGHT_CAMERA = 0x03,
};

struct Camera
{
    NH_API Camera(F32 fov = 45.0f, F32 near = 0.1f, F32 far = 1000.0f, const Vector3 & position = Vector3::ZERO,
        const Vector3& rotation = Vector3::ZERO, const Vector4& ambientColor = Vector4::ONE, ViewMatrixSource viewSource = VIEW_MATRIX_SOURCE_SCENE_CAMERA);
    NH_API Camera(const Vector4& bounds, F32 near = 0.1f, F32 far = 1000.0f, const Vector3& position = Vector3::ZERO,
        const Vector3& rotation = Vector3::ZERO, const Vector4& ambientColor = Vector4::ONE, ViewMatrixSource viewSource = VIEW_MATRIX_SOURCE_SCENE_CAMERA);

    void* operator new(U64 size) { return Memory::Allocate(sizeof(Camera), MEMORY_TAG_ENTITY); }
    void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Camera), MEMORY_TAG_ENTITY); }

    NH_API const Vector3& Position() const;
    NH_API void SetPosition(const Vector3& position);
    NH_API void Translate(const Vector3& translation);
    NH_API const Vector3& Rotation() const;
    NH_API void SetRotation(const Vector3& rotation);
    NH_API void Rotate(const Vector3& rotation);
    NH_API const Vector4& AmbientColor() const;
    NH_API void SetAmbientColor(const Vector4& color);
    NH_API const Matrix4& Projection() const;
    NH_API void ChangeProjection(F32 fov, F32 aspect, F32 near, F32 far);
    NH_API void ChangeProjection(const Vector4& bounds, F32 near, F32 far);
    NH_API const Matrix4& View();

    NH_API Vector3 Forward();
    NH_API Vector3 Back();
    NH_API Vector3 Right();
    NH_API Vector3 Left();
    NH_API Vector3 Up();
    NH_API Vector3 Down();

private:
    //TODO: Maybe have a transform
    Vector3 position;
    Vector3 rotation;
    Vector4 ambientColor; //TODO: Color struct
    ViewMatrixSource viewSource;
    bool dirty;
    Matrix4 projection;
    Matrix4 viewMatrix;
};
