#include "Hooks/CraftingMenuHook.h"

#include "Events/CraftingMenuWatcher.h"
#include "PCH/PCH.h"

namespace ARV
{
	namespace
	{
		constexpr std::uint32_t kProbeInterval = 30;
		constexpr std::uint32_t kMaxProbeAttempts = 300;
	}

	CraftingMenuHook* CraftingMenuHook::GetSingleton()
	{
		static CraftingMenuHook singleton;
		return std::addressof(singleton);
	}

	void CraftingMenuHook::Install()
	{
		auto* hook = GetSingleton();
		if (hook->installed_) {
			spdlog::info("CraftingMenuHook already installed");
			return;
		}

		auto vtable = REL::Relocation<std::uintptr_t>(RE::CraftingMenu::VTABLE[0]);
		_postDisplay = vtable.write_vfunc(
			6,
			+[](RE::CraftingMenu* a_menu)
			{
				_postDisplay(a_menu);
				CraftingMenuHook::GetSingleton()->HandlePostDisplay(a_menu);
			});

		hook->installed_ = true;
		spdlog::info("CraftingMenuHook installed");
	}

	void CraftingMenuHook::OnCraftingMenuOpened()
	{
		menuOpen_ = true;
		loggedValidStateForCurrentOpen_ = false;
		loggedProbeBudgetExhaustedForCurrentOpen_ = false;
		probeAttempts_ = 0;
	}

	void CraftingMenuHook::OnCraftingMenuClosed()
	{
		menuOpen_ = false;
		loggedValidStateForCurrentOpen_ = false;
		loggedProbeBudgetExhaustedForCurrentOpen_ = false;
		probeAttempts_ = 0;
	}

	void CraftingMenuHook::HandlePostDisplay(RE::CraftingMenu* a_menu)
	{
		if (!menuOpen_ || loggedValidStateForCurrentOpen_ || !a_menu) {
			return;
		}

		++probeAttempts_;
		if (probeAttempts_ > kMaxProbeAttempts) {
			if (!loggedProbeBudgetExhaustedForCurrentOpen_) {
				spdlog::warn(
					"PostDisplay probe budget exhausted after {} attempts on CraftingMenu={}",
					kMaxProbeAttempts,
					static_cast<void*>(a_menu));
				loggedProbeBudgetExhaustedForCurrentOpen_ = true;
			}
			return;
		}

		if (probeAttempts_ != 1 && probeAttempts_ % kProbeInterval != 0) {
			return;
		}

		if (probeAttempts_ == 1 || probeAttempts_ % kProbeInterval == 0) {
			spdlog::info("PostDisplay probe attempt {} on CraftingMenu={}", probeAttempts_, static_cast<void*>(a_menu));
		}

		if (CraftingMenuWatcher::GetSingleton()->ProbeCurrentCraftingMenuState("PostDisplay", false)) {
			loggedValidStateForCurrentOpen_ = true;
			spdlog::info("PostDisplay probe succeeded on attempt {}", probeAttempts_);
		}
	}
}
