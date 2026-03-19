#include "Config/Settings.h"

#include "PCH/PCH.h"

#include <Windows.h>

namespace
{
	constexpr auto kConfigPath = "Data/SKSE/Plugins/AlchemyRecipeViewVR.ini"sv;

	bool ReadBoolSetting(const char* a_section, const char* a_key, bool a_defaultValue)
	{
		char buffer[32]{};
		::GetPrivateProfileStringA(
			a_section,
			a_key,
			a_defaultValue ? "true" : "false",
			buffer,
			static_cast<DWORD>(std::size(buffer)),
			kConfigPath.data());

		std::string value = buffer;
		std::ranges::transform(value, value.begin(), [](unsigned char c) {
			return static_cast<char>(std::tolower(c));
		});

		if (value == "1" || value == "true" || value == "yes" || value == "on") {
			return true;
		}
		if (value == "0" || value == "false" || value == "no" || value == "off") {
			return false;
		}

		return a_defaultValue;
	}
}

namespace ARV::Config
{
	Settings& Settings::GetSingleton()
	{
		static Settings singleton;
		return singleton;
	}

	void Settings::Load()
	{
		const auto attributes = ::GetFileAttributesA(kConfigPath.data());
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			spdlog::warn("Settings: {} not found, using defaults", kConfigPath);
			return;
		}

		data_.enable = ReadBoolSetting("Main", "bEnable", data_.enable);
		data_.debugLogging = ReadBoolSetting("Main", "bDebugLogging", data_.debugLogging);
		data_.toggleKey = static_cast<std::int32_t>(::GetPrivateProfileIntA("Main", "iToggleKey", data_.toggleKey, kConfigPath.data()));
		data_.showNavButton = ReadBoolSetting("Main", "bShowNavButton", data_.showNavButton);

		spdlog::info(
			"Settings: enable={} debugLogging={} toggleKey={} showNavButton={}",
			data_.enable,
			data_.debugLogging,
			data_.toggleKey,
			data_.showNavButton);
	}

	const RuntimeSettings& Settings::Get() const noexcept
	{
		return data_;
	}

	bool Settings::Enabled() const noexcept
	{
		return data_.enable;
	}

	bool Settings::DebugLogging() const noexcept
	{
		return data_.debugLogging;
	}

	std::int32_t Settings::ToggleKey() const noexcept
	{
		return data_.toggleKey;
	}

	bool Settings::ShowNavButton() const noexcept
	{
		return data_.showNavButton;
	}
}
