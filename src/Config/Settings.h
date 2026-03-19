#pragma once

namespace ARV::Config
{
	struct RuntimeSettings
	{
		bool         enable{ true };
		bool         debugLogging{ false };
		std::int32_t toggleKey{ 43 };
		bool         showNavButton{ true };
	};

	class Settings final
	{
	public:
		static Settings& GetSingleton();

		void Load();

		[[nodiscard]] const RuntimeSettings& Get() const noexcept;
		[[nodiscard]] bool                   Enabled() const noexcept;
		[[nodiscard]] bool                   DebugLogging() const noexcept;
		[[nodiscard]] std::int32_t           ToggleKey() const noexcept;
		[[nodiscard]] bool                   ShowNavButton() const noexcept;

	private:
		RuntimeSettings data_{};
	};
}
