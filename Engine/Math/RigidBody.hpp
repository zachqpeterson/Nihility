#pragma once

#include "Defines.hpp"
#include "Shape.hpp"
#include "Math.hpp"
#include "Resources\Scene.hpp"

enum NH_API BodyType
{
	BODY_TYPE_STATIC,
	BODY_TYPE_KINEMATIC,
	BODY_TYPE_DYNAMIC,

	BODY_TYPE_COUNT
};

struct RigidBody2DDef
{
	/// The body type: static, kinematic, or dynamic.
	BodyType type = BODY_TYPE_STATIC;

	/// The initial world position of the body. Bodies should be created with the desired position.
	/// @note Creating bodies at the origin and then moving them nearly doubles the cost of body creation, especially
	///	if the body is moved after shapes have been added.
	Vector2 position = Vector2Zero;

	/// The initial world rotation of the body. Use b2MakeRot() if you have an angle.
	Quaternion2 rotation = Quaternion2Identity;

	/// The initial linear velocity of the body's origin. Typically in meters per second.
	Vector2 linearVelocity = Vector2Zero;

	/// The initial angular velocity of the body. Radians per second.
	F32 angularVelocity = 0.0f;

	/// Linear damping is use to reduce the linear velocity. The damping parameter
	/// can be larger than 1 but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	///	Generally linear damping is undesirable because it makes objects move slowly
	///	as if they are floating.
	F32 linearDamping = 0.0f;

	/// Angular damping is use to reduce the angular velocity. The damping parameter
	/// can be larger than 1.0f but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	///	Angular damping can be use slow down rotating bodies.
	F32 angularDamping = 0.0f;

	/// Scale the gravity applied to this body. Non-dimensional.
	F32 gravityScale = 1.0f;

	/// Sleep velocity threshold, default is 0.05 meter per second
	F32 sleepThreshold = 0.05f;

	/// Use this to store application specific body data.
	void* userData = nullptr;

	/// Set this flag to false if this body should never fall asleep.
	bool enableSleep = true;

	/// Is this body initially awake or sleeping?
	bool isAwake = true;

	/// Should this body be prevented from rotating? Useful for characters.
	bool fixedRotation = false;

	/// Treat this body as high speed object that performs continuous collision detection
	/// against dynamic and kinematic bodies, but not other bullet bodies.
	///	@warning Bullets should be used sparingly. They are not a solution for general dynamic-versus-dynamic
	///	continuous collision. They may interfere with joint constraints.
	bool isBullet = false;

	/// Used to disable a body. A disabled body does not move or collide.
	bool isEnabled = true;

	/// Automatically compute mass and related properties on this body from shapes.
	/// Triggers whenever a shape is add/removed/changed. Default is true.
	bool automaticMass = true;

	/// This allows this body to bypass rotational speed limits. Should only be used
	///	for circular objects, like wheels.
	bool allowFastRotation = false;
};

struct NH_API RigidBody2D : public Component<RigidBody2D>
{
	RigidBody2D(const RigidBody2DDef& def);
	RigidBody2D(const RigidBody2D& other);
	RigidBody2D(RigidBody2D&& other);

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;
	virtual void Cleanup(Scene* scene) final;

	I32 AddCollider(const ShapeDef& shapeDef, const Capsule& capsule);
	I32 AddCollider(const ShapeDef& shapeDef, const Circle& circle);
	I32 AddCollider(const ShapeDef& shapeDef, const ConvexPolygon& polygon);
	I32 AddCollider(const ShapeDef& shapeDef, const Segment& segment);
	I32 AddCollider(const ShapeDef& shapeDef, const ChainSegment& chainSegment);

	void SetPosition(const Vector2& position);
	void SetRotation(const Quaternion2& rotation);

private:
	void DestroyBodyContacts(bool wakeBodies);
	void UpdateMassData();

	void* userData;
	I32 setIndex;
	I32 localIndex;
	I32 headContactKey;
	I32 contactCount;
	I32 headShapeId;
	I32 shapeCount;
	I32 headChainId;
	I32 headJointKey;
	I32 jointCount;
	I32 islandId;
	I32 islandPrev;
	I32 islandNext;
	F32 sleepThreshold;
	F32 sleepTime;
	I32 bodyMoveIndex;
	I32 id;

	BodyType type{ BODY_TYPE_DYNAMIC };

	U16 revision;

	bool enableSleep;
	bool fixedRotation;
	bool isSpeedCapped;
	bool isMarked;
	bool automaticMass;

	friend class Physics;
	friend struct ConstraintGraph;
};