module;

#include "Defines.hpp"

#include "Discord\discord.h"

module Networking:Discord;

import Core;
import Containers;

discord::User Discord::currentUser;
discord::Core* Discord::core;
discord::Activity Discord::activity;
U64 Discord::discordAppId;

void Discord::Initialize(U64 discordAppId_)
{
	Logger::Trace("Initializing Discord Integration...");

	discordAppId = discordAppId_;

#ifdef NH_DEBUG
	if (discordAppId == 0) { discordAppId = 1200965397206274118; }
#endif
	
	if (discordAppId != 0) { TryCreateCore(); }
}

bool Discord::TryCreateCore()
{
	if (discord::Core::Create(discordAppId, DiscordCreateFlags_NoRequireDiscord, &core) != discord::Result::Ok) { return false; }

#ifdef NH_DEBUG
	core->SetLogHook(discord::LogLevel::Error, Log);
#endif

	return true;
}

void Discord::Shutdown()
{
	Logger::Trace("Shutting Down Discord Integration...");
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

void Discord::SetActivity(const Activity& newActivity)
{
	activity.SetType(discord::ActivityType::Playing);
	activity.SetName(newActivity.name.Data());
	activity.SetDetails(newActivity.details.Data());
	activity.SetState(newActivity.state.Data());
	activity.GetTimestamps().SetStart(Time::SecondsSinceEpoch());
	activity.SetInstance(false);
	activity.SetApplicationId(discordAppId);
	discord::ActivityAssets& assets = activity.GetAssets();
	assets.SetLargeImage(newActivity.largeImage.Data());
	assets.SetLargeText(newActivity.largeImageText.Data());
	assets.SetSmallImage(newActivity.smallImage.Data());
	assets.SetSmallText(newActivity.smallImageText.Data());
	
	if (core || TryCreateCore()) { core->ActivityManager().UpdateActivity(activity, Callback); }
}

void Discord::ChangeActivityName(const StringView& name)
{
	activity.SetName(name.Data());

	if (core || TryCreateCore()) { core->ActivityManager().UpdateActivity(activity, Callback); }
}

void Discord::ChangeActivityDetails(const StringView& details)
{
	activity.SetDetails(details.Data());

	if (core || TryCreateCore()) { core->ActivityManager().UpdateActivity(activity, Callback); }
}

void Discord::ChangeActivityState(const StringView& state)
{
	activity.SetState(state.Data());

	if (core || TryCreateCore()) { core->ActivityManager().UpdateActivity(activity, Callback); }
}

void Discord::ChangeActivityLargeImage(const StringView& largeImage)
{
	discord::ActivityAssets& assets = activity.GetAssets();
	assets.SetLargeImage(largeImage.Data());

	if (core || TryCreateCore()) { core->ActivityManager().UpdateActivity(activity, Callback); }
}

void Discord::ChangeActivityLargeImageText(const StringView& largeImageText)
{
	discord::ActivityAssets& assets = activity.GetAssets();
	assets.SetLargeText(largeImageText.Data());

	if (core || TryCreateCore()) { core->ActivityManager().UpdateActivity(activity, Callback); }
}

void Discord::ChangeActivitySmallImage(const StringView& smallImage)
{
	discord::ActivityAssets& assets = activity.GetAssets();
	assets.SetSmallImage(smallImage.Data());

	if (core || TryCreateCore()) { core->ActivityManager().UpdateActivity(activity, Callback); }
}

void Discord::ChangeActivitySmallImageText(const StringView& smallImageText)
{
	discord::ActivityAssets& assets = activity.GetAssets();
	assets.SetSmallText(smallImageText.Data());

	if (core || TryCreateCore()) { core->ActivityManager().UpdateActivity(activity, Callback); }
}
