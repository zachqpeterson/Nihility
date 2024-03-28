#include "Physics.hpp"

#include "Core\Time.hpp"
#include "Core\Logger.hpp"

bool Physics::Initialize()
{
	Logger::Trace("Initializing Physics...");

	return true;
}

void Physics::Shutdown()
{
	Logger::Trace("Shutting Down Physics...");
}

void Physics::Update(F64 step)
{
	//for each rigid body

	//F64 a = 1.4;
	//F64 b = 0.7;
	//F64 c = 2.1;
	//F32 v = a * b * c;
	//Vector3 i0 = v * Vector3{ b * b + c * c, a * a + c * c, a * a + b * b } / 12.0f;
	//
	//RigidBody rb;
	//rb.invI0 = 1.0 / i0;
	//
	//Matrix3 r = rb.rotation.ToMatrix3();
	//
	//Vector3 omega = r * rb.invI0 * r.Transpose() * rb.angularMomentum;
	//
	//rb.position += rb.velocity * (F32)Time::DeltaTime();
	//rb.rotation = Quaternion3{ omega * (F32)(step / 2.0) } * rb.rotation;
}