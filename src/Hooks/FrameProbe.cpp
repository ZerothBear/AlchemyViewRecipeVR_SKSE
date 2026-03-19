#include "Hooks/FrameProbe.h"

#include "Events/CraftingMenuWatcher.h"
#include "PCH/PCH.h"
#include "RE/Offset.h"

namespace ARV
{
	namespace
	{
		FrameProbe& GetState()
		{
			static FrameProbe state;
			return state;
		}
	}

	void FrameProbe::Install()
	{
		auto& state = GetState();
		if (state.installed_) {
			spdlog::info("FrameProbe already installed");
			return;
		}

		auto& trampoline = SKSE::GetTrampoline();

		using DoFrame_t = void(RE::Main::*)();
		static REL::Relocation<DoFrame_t> doFrame;

		auto hook = +[](RE::Main* a_main)
		{
			doFrame(a_main);
			FrameProbe::OnFrame();
		};

		const auto base = REL::Relocation<std::uintptr_t>(RE::Offset::Main::OnIdle);
		const auto startAddr = base.address() + 0x3E;
		const auto endAddr = base.address() + 0x80;
		std::uintptr_t callSite = 0;

		for (auto addr = startAddr; addr < endAddr; ++addr) {
			if (*reinterpret_cast<const std::uint8_t*>(addr) == 0xE8) {
				callSite = addr;
				break;
			}
		}

		if (!callSite) {
			spdlog::error("FrameProbe: failed to find OnIdle callsite");
			return;
		}

		doFrame = trampoline.write_call<5>(callSite, hook);
		state.installed_ = true;
		spdlog::info("FrameProbe installed at 0x{:X}", callSite);
	}

	void FrameProbe::OnFrame()
	{
		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			return;
		}

		auto& state = GetState();
		const bool craftingMenuOpen = ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME);
		if (!craftingMenuOpen) {
			if (state.craftingMenuWasOpen_) {
				spdlog::info("FrameProbe detected Crafting Menu close");
			}

			state.craftingMenuWasOpen_ = false;
			state.loggedValidStateForThisOpen_ = false;
			state.framesSinceOpen_ = 0;
			state.nextProbeFrame_ = 1;
			return;
		}

		if (!state.craftingMenuWasOpen_) {
			state.craftingMenuWasOpen_ = true;
			state.loggedValidStateForThisOpen_ = false;
			state.framesSinceOpen_ = 0;
			state.nextProbeFrame_ = 1;
			spdlog::info("FrameProbe detected Crafting Menu open");
		}

		++state.framesSinceOpen_;
		if (state.loggedValidStateForThisOpen_ || state.framesSinceOpen_ < state.nextProbeFrame_) {
			return;
		}

		state.loggedValidStateForThisOpen_ =
			CraftingMenuWatcher::GetSingleton()->ProbeCurrentCraftingMenuState("FrameProbe");
		if (!state.loggedValidStateForThisOpen_) {
			state.nextProbeFrame_ = state.framesSinceOpen_ + 10;
		}
	}
}
