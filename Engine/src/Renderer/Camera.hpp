#pragma once

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"

enum ViewMatrixSource
{
    VIEW_MATRIX_SOURCE_SCENE_CAMERA = 0x01,
    VIEW_MATRIX_SOURCE_UI_CAMERA = 0x02,
    VIEW_MATRIX_SOURCE_LIGHT_CAMERA = 0x03,
};

struct NH_API Camera
{
    Camera(F32 fov = 45.0f, F32 near = 0.1f, F32 far = 1000.0f, const Vector3 & position = Vector3::ZERO,
        const Vector3& rotation = Vector3::ZERO, const Vector3& ambientColor = Vector3::ONE, ViewMatrixSource viewSource = VIEW_MATRIX_SOURCE_SCENE_CAMERA);
    Camera(const Vector4& bounds, F32 near = 0.1f, F32 far = 1000.0f, const Vector3& position = Vector3::ZERO,
        const Vector3& rotation = Vector3::ZERO, const Vector3& ambientColor = Vector3::ONE, ViewMatrixSource viewSource = VIEW_MATRIX_SOURCE_SCENE_CAMERA);

    void* operator new(U64 size) { return Memory::Allocate(sizeof(Camera), MEMORY_TAG_RENDERER); }
    void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Camera), MEMORY_TAG_RENDERER); }
    
    void Update();

    const Vector3& Position() const;
    void SetPosition(const Vector3& position);
    void Translate(const Vector3& translation);
    const Vector3& Rotation() const;
    void SetRotation(const Vector3& rotation);
    void Rotate(const Vector3& rotation);
    const Vector4& AmbientColor() const;
    void SetAmbientColor(const Vector3& color);
    const Matrix4& Projection() const;
    void ChangeProjection(F32 fov, F32 aspect, F32 near, F32 far);
    void ChangeProjection(const Vector4& bounds, F32 near, F32 far);
    const Matrix4& View();
    void SetTarget(Transform2D* target);

    Vector3 Forward();
    Vector3 Back();
    Vector3 Right();
    Vector3 Left();
    Vector3 Up();
    Vector3 Down();

private:
    //TODO: Maybe have a transform
    Vector3 position;
    Vector3 rotation;
    Vector3 ambientColor; //TODO: Color struct
    ViewMatrixSource viewSource;
    bool dirty;
    Matrix4 projection;
    Matrix4 viewMatrix;
    Transform2D* target;
};
