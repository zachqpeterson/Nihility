#include "Discord.hpp"

#include "Core\Logger.hpp"

#include <time.h>

discord::User Discord::currentUser;
discord::Core* Discord::core;
discord::Activity Discord::activity;

bool Discord::Initialize(CSTR applicationName)
{
	discord::Result result = discord::Core::Create(1200965397206274118, DiscordCreateFlags_Default, &core);
	if (result != discord::Result::Ok)
	{
		Logger::Error("Failed To Instantiate Discord Core!");
		return false;
	}

	core->SetLogHook(discord::LogLevel::Debug, Log);

	activity.SetType(discord::ActivityType::Playing);
	activity.SetName("Nihility");
	activity.SetDetails(applicationName);
	activity.GetTimestamps().SetStart(time(nullptr));
	activity.SetInstance(false);
	activity.SetApplicationId(1200965397206274118);
	discord::ActivityAssets& assets = activity.GetAssets();
	assets.SetLargeImage("nihility_logo_large");

	core->ActivityManager().UpdateActivity(activity, Callback);

	return true;
}

void Discord::Shutdown()
{

}

void Discord::Update()
{
	if (core) { core->RunCallbacks(); }
}

void Discord::Log(discord::LogLevel level, const char* message)
{
	switch (level)
	{
	case discord::LogLevel::Debug: { Logger::Debug(message); } break;
	case discord::LogLevel::Info: { Logger::Info(message); } break;
	case discord::LogLevel::Warn: { Logger::Warn(message); } break;
	case discord::LogLevel::Error: { Logger::Error(message); } break;
	}
}

void Discord::Callback(discord::Result result)
{

}