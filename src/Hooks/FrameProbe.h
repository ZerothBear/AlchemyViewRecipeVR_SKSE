#pragma once

namespace ARV
{
	class FrameProbe
	{
	public:
		static void Install();
		static void OnFrame();

	private:
		bool installed_{ false };
		bool craftingMenuWasOpen_{ false };
		bool loggedValidStateForThisOpen_{ false };
		std::uint32_t framesSinceOpen_{ 0 };
		std::uint32_t nextProbeFrame_{ 1 };
	};
}
