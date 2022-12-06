#include "TimeSlip.hpp"

#include <Engine.hpp>

int main()
{
	Engine::Initialize("Time-Slip", TimeSlip::Initialize, TimeSlip::Update, TimeSlip::Shutdown);

	return 0;
}
