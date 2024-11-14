#pragma once

#include "Manifold.hpp"


struct SensorBeginTouchEvent
{
	/// The id of the sensor shape
	I32 sensorShapeId;

	/// The id of the dynamic shape that began touching the sensor shape
	I32 visitorShapeId;
};

/// An end touch event is generated when a shape stops overlapping a sensor shape.
struct SensorEndTouchEvent
{
	/// The id of the sensor shape
	I32 sensorShapeId;

	/// The id of the dynamic shape that stopped touching the sensor shape
	I32 visitorShapeId;
};

/// Sensor events are buffered in the Box2D world and are available
///	as begin/end overlap event arrays after the time step is complete.
///	Note: these may become invalid if bodies and/or shapes are destroyed
struct SensorEvents
{
	/// Array of sensor begin touch events
	SensorBeginTouchEvent* beginEvents;

	/// Array of sensor end touch events
	SensorEndTouchEvent* endEvents;

	/// The number of begin touch events
	I32 beginCount;

	/// The number of end touch events
	I32 endCount;
};

/// A begin touch event is generated when two shapes begin touching.
struct ContactBeginTouchEvent
{
	/// Id of the first shape
	I32 shapeIdA;

	/// Id of the second shape
	I32 shapeIdB;

	/// The initial contact manifold
	Manifold manifold;
};

/// An end touch event is generated when two shapes stop touching.
struct ContactEndTouchEvent
{
	/// Id of the first shape
	I32 shapeIdA;

	/// Id of the second shape
	I32 shapeIdB;
};

/// A hit touch event is generated when two shapes collide with a speed faster than the hit speed threshold.
struct ContactHitEvent
{
	/// Id of the first shape
	I32 shapeIdA;

	/// Id of the second shape
	I32 shapeIdB;

	/// Point where the shapes hit
	Vector2 point;

	/// Normal vector pointing from shape A to shape B
	Vector2 normal;

	/// The speed the shapes are approaching. Always positive. Typically in meters per second.
	F32 approachSpeed;
};

/// Contact events are buffered in the Box2D world and are available
///	as event arrays after the time step is complete.
///	Note: these may become invalid if bodies and/or shapes are destroyed
struct ContactEvents
{
	/// Array of begin touch events
	ContactBeginTouchEvent* beginEvents;

	/// Array of end touch events
	ContactEndTouchEvent* endEvents;

	/// Array of hit events
	ContactHitEvent* hitEvents;

	/// Number of begin touch events
	I32 beginCount;

	/// Number of end touch events
	I32 endCount;

	/// Number of hit events
	I32 hitCount;
};

/// Body move events triggered when a body moves.
/// Triggered when a body moves due to simulation. Not reported for bodies moved by the user.
/// This also has a flag to indicate that the body went to sleep so the application can also
/// sleep that actor/entity/object associated with the body.
/// On the other hand if the flag does not indicate the body went to sleep then the application
/// can treat the actor/entity/object associated with the body as awake.
///	This is an efficient way for an application to update game object transforms rather than
///	calling functions such as b2Body_GetTransform() because this data is delivered as a contiguous array
///	and it is only populated with bodies that have moved.
///	@note If sleeping is disabled all dynamic and kinematic bodies will trigger move events.
struct BodyMoveEvent
{
	Transform2D transform;
	I32 bodyId;
	void* userData;
	bool fellAsleep;
};

/// Body events are buffered in the Box2D world and are available
///	as event arrays after the time step is complete.
///	Note: this data becomes invalid if bodies are destroyed
struct BodyEvents
{
	/// Array of move events
	BodyMoveEvent* moveEvents;

	/// Number of move events
	I32 moveCount;
};