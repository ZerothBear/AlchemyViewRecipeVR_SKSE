#pragma once

#include "RE/Skyrim.h"

namespace ARV
{
	class CraftingMenuHook
	{
	public:
		static CraftingMenuHook* GetSingleton();
		static void Install();

		void OnCraftingMenuOpened();
		void OnCraftingMenuClosed();

	private:
		void HandlePostDisplay(RE::CraftingMenu* a_menu);

		inline static REL::Relocation<decltype(&RE::CraftingMenu::PostDisplay)> _postDisplay;

		bool installed_{ false };
		bool menuOpen_{ false };
		bool loggedValidStateForCurrentOpen_{ false };
		bool loggedProbeBudgetExhaustedForCurrentOpen_{ false };
		std::uint32_t probeAttempts_{ 0 };
	};
}
