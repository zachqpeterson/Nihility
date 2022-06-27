#include "Camera.hpp"

Camera::Camera(F32 fov, F32 near, F32 far, const Vector3& position, const Vector3& rotation, const Vector4& ambientColor, ViewMatrixSource viewSource) :
    position{ position }, rotation{ rotation }, ambientColor{ ambientColor }, viewSource{ viewSource }, dirty{ false }, viewMatrix{ position, rotation }
{
    projection.SetPerspective(Math::DegToRad(fov), 1.7777777777f, near, far);
}

Camera::Camera(const Vector4& bounds, F32 near, F32 far, const Vector3& position, const Vector3& rotation, const Vector4& ambientColor, ViewMatrixSource viewSource) :
    position{ position }, rotation{ rotation }, ambientColor{ ambientColor }, viewSource{ viewSource }, dirty{ false }, viewMatrix{ position, rotation }
{
    projection.SetOrthographic(bounds.x, bounds.y, bounds.z, bounds.w, near, far);
}

const Vector3& Camera::Position() const
{
    return position;
}

void Camera::SetPosition(const Vector3& position)
{
    this->position = position;
    dirty = true;
}

void Camera::Translate(const Vector3& translation)
{
    position += translation;
    dirty = true;
}

const Vector3& Camera::Rotation() const
{
    return rotation;
}

void Camera::SetRotation(const Vector3& rotation)
{
    this->rotation = rotation;
    dirty = true;
}

void Camera::Rotate(const Vector3& rotation)
{
    this->rotation += rotation;
    dirty = true;
}

const Vector4& Camera::AmbientColor() const
{
    return ambientColor;
}

void Camera::SetAmbientColor(const Vector4& color)
{
    ambientColor = color;
}

const Matrix4& Camera::Projection() const
{
    return projection;
}

void Camera::ChangeProjection(F32 fov, F32 aspect, F32 near, F32 far)
{
    projection.SetPerspective(Math::DegToRad(fov), aspect, near, far);
}

void Camera::ChangeProjection(const Vector4& bounds, F32 near, F32 far)
{
    projection.SetOrthographic(bounds.x, bounds.y, bounds.z, bounds.w, near, far);
}

const Matrix4& Camera::View()
{
    if (dirty)
    {
        dirty = false;
        viewMatrix = Matrix4(position, rotation);
    }

    return viewMatrix;
}

Vector3 Camera::Forward()
{
    if (dirty)
    {
        dirty = false;
        viewMatrix = Matrix4(position, rotation);
    }

    return viewMatrix.Forward();
}

Vector3 Camera::Back()
{
    if (dirty)
    {
        dirty = false;
        viewMatrix = Matrix4(position, rotation);
    }

    return viewMatrix.Back();
}

Vector3 Camera::Right()
{
    if (dirty)
    {
        dirty = false;
        viewMatrix = Matrix4(position, rotation);
    }

    return viewMatrix.Right();
}

Vector3 Camera::Left()
{
    if (dirty)
    {
        dirty = false;
        viewMatrix = Matrix4(position, rotation);
    }

    return viewMatrix.Left();
}

Vector3 Camera::Up()
{
    if (dirty)
    {
        dirty = false;
        viewMatrix = Matrix4(position, rotation);
    }

    return viewMatrix.Up();
}

Vector3 Camera::Down()
{
    if (dirty)
    {
        dirty = false;
        viewMatrix = Matrix4(position, rotation);
    }

    return viewMatrix.Down();
}