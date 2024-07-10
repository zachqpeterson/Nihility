module;

#include "Defines.hpp"

export module Networking:Steam;

export class NH_API Steam
{
public:
	static bool Initialize(U32 appID);
	static void Shutdown();

	static void Update();

private:
	static U32 appID;

	STATIC_CLASS(Steam);
};