#pragma once

#include "ResourceDefines.hpp"

#include "Containers/String.hpp"

struct NH_API AnimationChannel
{
public:
	const String& TargetNodeName() const { return nodeName; }

	F32 MaxTime()
	{
		F32 maxTranslationTime = translationTimings[translationTimings.Size() - 1];
		F32 maxRotationTime = rotationTimings[rotationTimings.Size() - 1];
		F32 maxScaleTime = scaleTimings[scaleTimings.Size() - 1];

		return Math::Max(maxRotationTime, maxTranslationTime, maxScaleTime);
	}

	Matrix4 TRSMatrix(F32 time) const { return { Translation(time), Rotation(time), Scaling(time) }; }

	Vector3 Translation(F32 time) const
	{
		if (translations.Empty()) { return Vector3::Zero; }

		switch (preState)
		{
		case AnimationBehaviour::Default:
			if (time < translationTimings[0]) { return Vector3::Zero; }
			break;
		case AnimationBehaviour::Constant:
			if (time < translationTimings[0]) { return translations[0]; }
			break;
		default:
			break;
		}

		switch (postState)
		{
		case AnimationBehaviour::Default:
			if (time > translationTimings[translationTimings.Size() - 1]) { return Vector3::Zero; }
			break;
		case AnimationBehaviour::Constant:
			if (time >= translationTimings[translationTimings.Size() - 1]) { return translations[translationTimings.Size() - 1]; }
			break;
		default:
			break;
		}

		F32* t = translationTimings.Find([time](F32* t) { return *t >= time; });
		I64 timeIndex = Math::Max((t - translationTimings.begin()) - 1, 0ll);
		F32 interpolatedTime = (time - translationTimings[timeIndex]) * inverseTranslationTimeDiffs[timeIndex];

		return Math::Mix(translations[timeIndex], translations[timeIndex + 1], interpolatedTime);
	}

	Vector3 Scaling(F32 time) const
	{
		if (scalings.Empty()) { return Vector3::One; }

		switch (preState)
		{
		case AnimationBehaviour::Default:
			if (time < scaleTimings[0]) { return Vector3::Zero; }
			break;
		case AnimationBehaviour::Constant:
			if (time < scaleTimings[0]) { return scalings[0]; }
			break;
		default:
			break;
		}

		switch (postState)
		{
		case AnimationBehaviour::Default:
			if (time > scaleTimings[scalings.Size() - 1]) { return Vector3::Zero; }
			break;
		case AnimationBehaviour::Constant:
			if (time >= scaleTimings[scalings.Size() - 1]) { return scalings[scalings.Size() - 1]; }
			break;
		default:
			break;
		}

		F32* t = scaleTimings.Find([time](F32* t) { return *t >= time; });
		I64 timeIndex = Math::Max((t - scaleTimings.begin()) - 1, 0ll);
		F32 interpolatedTime = (time - scaleTimings[timeIndex]) * inverseScaleTimeDiffs[timeIndex];

		return Math::Mix(scalings[timeIndex], scalings[timeIndex + 1], interpolatedTime);
	}

	Quaternion3 Rotation(F32 time) const
	{
		if (rotations.Empty()) { return Quaternion3::Identity; }

		switch (preState)
		{
		case AnimationBehaviour::Default:
			if (time < rotationTimings[0]) { return Quaternion3::Identity; }
			break;
		case AnimationBehaviour::Constant:
			if (time < rotationTimings[0]) { return rotations[0]; }
			break;
		default:
			break;
		}

		switch (postState)
		{
		case AnimationBehaviour::Default:
			if (time > rotationTimings[rotationTimings.Size() - 1]) { return Quaternion3::Identity; }
			break;
		case AnimationBehaviour::Constant:
			if (time >= rotationTimings[rotationTimings.Size() - 1]) { return rotations[rotationTimings.Size() - 1]; }
			break;
		default:
			break;
		}

		F32* t = rotationTimings.Find([time](F32* t) { return *t >= time; });
		I64 timeIndex = Math::Max((t - rotationTimings.begin()) - 1, 0ll);
		F32 interpolatedTime = (time - rotationTimings[timeIndex]) * inverseRotationTimeDiffs[timeIndex];

		return rotations[timeIndex].Slerp(rotations[timeIndex + 1], interpolatedTime).Normalize();
	}

private:
	String nodeName;

	Vector<F32> translationTimings{};
	Vector<F32> inverseTranslationTimeDiffs{};
	Vector<F32> rotationTimings{};
	Vector<F32> inverseRotationTimeDiffs{};
	Vector<F32> scaleTimings{};
	Vector<F32> inverseScaleTimeDiffs{};

	Vector<Vector3> translations{};
	Vector<Vector3> scalings{};
	Vector<Quaternion3> rotations{};

	AnimationBehaviour preState = AnimationBehaviour::Default;
	AnimationBehaviour postState = AnimationBehaviour::Default;

	friend class Resources;
};

struct NH_API Animation3D
{
public:
	const Vector<AnimationChannel>& Channels() const { return animChannels; }

	const String& ClipName() const { return clipName; }
	const F32& ClipDuration() const { return clipDuration; }
	const F32& ClipTicksPerSecond() const { return clipTicksPerSecond; }

	void SetClipName(const String& name) { clipName = name; }

private:
	String clipName;
	F32 clipDuration = 0.0f;
	F32 clipTicksPerSecond = 0.0f;

	Vector<AnimationChannel> animChannels{};

	friend class Resources;
};