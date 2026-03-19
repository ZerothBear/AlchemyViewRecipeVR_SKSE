#pragma once

#include "RE/Skyrim.h"

#include <string_view>

namespace ARV
{
	class CraftingMenuWatcher final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static CraftingMenuWatcher* GetSingleton();
		static void Install();
		[[nodiscard]] bool ProbeCurrentCraftingMenuState(
			std::string_view a_reason,
			bool a_allowDeferredQueue = false) const;

		RE::BSEventNotifyControl ProcessEvent(
			const RE::MenuOpenCloseEvent* a_event,
			RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;

	private:
		void QueueCurrentCraftingMenuStateLog(std::string_view a_reason, std::uint32_t a_attempt = 1) const;
		[[nodiscard]] bool LogCurrentCraftingMenuState(
			std::string_view a_reason,
			std::uint32_t a_attempt,
			bool a_allowDeferredQueue) const;

		bool installed_{ false };
	};
}
