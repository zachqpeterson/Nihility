#pragma once

#include "ResourceDefines.hpp"

#include "Model.hpp"

#include "Core/Time.hpp"

struct NH_API ModelInstance
{
public:
	ResourceRef<Model> ModelRef() const { return model; }
	Matrix4 WorldTransformMatrix() const { return localTranslationMatrix; }

	void SetTranslation(const Vector3& position) { this->position = position; }
	void SetRotation(const Vector3& rotation) { this->rotation = rotation; }
	void SetScale(const Vector3& scale) { this->scale = scale; }
	void SetSwapYZAxis(bool value) { this->swapYZAxis = value; }

	const Vector3& Translation() const { return position; }
	const Vector3& Rotation() const { return rotation; }
	const Vector3& Scale() const { return scale; }
	bool SwapYZAxis() const { return swapYZAxis; }

	const Vector<Matrix4>& BoneMatrices() const { return boneMatrices; }

	void UpdateModelRootMatrix()
	{
		localScaleMatrix = Matrix4::Scale(scale);

		if (swapYZAxis)
		{
			Matrix4 flipMatrix = Matrix4::Rotate(Matrix4::Identity, -90.0f, Vector3::Forward);
			localSwapAxisMatrix = Matrix4::Rotate(flipMatrix, -90.0f, Vector3::Up);
		}
		else
		{
			localSwapAxisMatrix = Matrix4::Identity;
		}

		localRotationMatrix = Matrix4::Rotate(rotation);

		localTranslationMatrix = Matrix4::Translate(position);

		localTransformMatrix = localTranslationMatrix * localRotationMatrix * localSwapAxisMatrix * localScaleMatrix;
	}

	void UpdateAnimation()
	{
		F32 deltaTime = (F32)Time::DeltaTime();

		const Animation3D& animation = model->Animations()[animClipNr];

		animPlayTimePos += deltaTime * animation.ClipTicksPerSecond() * animSpeedFactor;
		animPlayTimePos = Math::Mod(animPlayTimePos, animation.ClipDuration());

		const Vector<AnimationChannel>& animChannels = animation.Channels();

		for (const AnimationChannel& channel : animChannels)
		{
			const String& nodeNameToAnimate = channel.TargetNodeName();
			Node* node = *model->NodeMap()[nodeNameToAnimate];

			node->SetRotation(channel.Rotation(animPlayTimePos));
			node->SetScaling(channel.Scaling(animPlayTimePos));
			node->SetTranslation(channel.Translation(animPlayTimePos));
		}

		model->RootNode()->SetRootTransformMatrix(localTransformMatrix * model->RootTranformationMatrix());

		boneMatrices.Clear();
		for (Node* node : model->NodeList())
		{
			String nodeName = node->NodeName();

			node->UpdateTRSMatrix();

			const Matrix4* matrix = model->BoneOffsetMatrices()[nodeName];

			if (matrix) { boneMatrices.Emplace((*model->NodeMap()[nodeName])->TRSMatrix() * *matrix); }
		}
	}

private:
	ResourceRef<Model> model = nullptr;

	Vector3 position = Vector3::Zero;
	Vector3 rotation = Vector3::Zero;
	Vector3 scale = Vector3::One;
	bool swapYZAxis = false;

	U32 animClipNr = 0;
	F32 animPlayTimePos = 0.0f;
	F32 animSpeedFactor = 1.0f;

	Matrix4 localTranslationMatrix = Matrix4::Identity;
	Matrix4 localRotationMatrix = Matrix4::Identity;
	Matrix4 localScaleMatrix = Matrix4::Identity;
	Matrix4 localSwapAxisMatrix = Matrix4::Identity;

	Matrix4 localTransformMatrix = Matrix4::Identity;

	Vector<Matrix4> boneMatrices{};

	friend class Resources;
};