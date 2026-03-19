#include "Events/CraftingMenuWatcher.h"

#include "Hooks/CraftingMenuHook.h"
#include "PCH/PCH.h"

#include <Windows.h>

namespace
{
	using AlchemyMenu = RE::CraftingSubMenus::CraftingSubMenus::AlchemyMenu;

	constexpr std::size_t kMaxLoggedIngredients = 8;
	constexpr std::uint32_t kMaxDeferredAttempts = 3;

	std::string SafeMenuName(const RE::MenuOpenCloseEvent* a_event)
	{
		if (!a_event || !a_event->menuName.c_str()) {
			return "<null>";
		}

		return a_event->menuName.c_str();
	}

	std::string SafeIngredientName(RE::InventoryEntryData* a_entry)
	{
		if (!a_entry) {
			return "<null>";
		}

		if (const auto* displayName = a_entry->GetDisplayName(); displayName && displayName[0] != '\0') {
			return displayName;
		}

		if (const auto* object = a_entry->object; object && object->GetName()) {
			return object->GetName();
		}

		return "<unnamed>";
	}

	bool IsAddressInsideMainModule(const void* a_address)
	{
		if (!a_address) {
			return false;
		}

		const auto moduleBase = reinterpret_cast<std::uintptr_t>(::GetModuleHandleW(nullptr));
		if (!moduleBase) {
			return false;
		}

		const auto* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(moduleBase);
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
			return false;
		}

		const auto* ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(moduleBase + dosHeader->e_lfanew);
		if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
			return false;
		}

		const auto moduleEnd = moduleBase + ntHeaders->OptionalHeader.SizeOfImage;
		const auto address = reinterpret_cast<std::uintptr_t>(a_address);
		return address >= moduleBase && address < moduleEnd;
	}

	void LogSelectedIndexes(const AlchemyMenu* a_menu, std::string_view a_reason)
	{
		if (!a_menu) {
			return;
		}

		if (a_menu->selectedIndexes.empty()) {
			spdlog::info("[{}] selectedIndexes=<empty>", a_reason);
			return;
		}

		std::string joined;
		const auto selectedCount = static_cast<std::uint32_t>(a_menu->selectedIndexes.size());
		for (std::uint32_t i = 0; i < selectedCount; ++i) {
			if (!joined.empty()) {
				joined += ", ";
			}
			joined += std::to_string(a_menu->selectedIndexes[i]);
		}

		spdlog::info("[{}] selectedIndexes=[{}]", a_reason, joined);
	}

	void LogIngredientEntries(const AlchemyMenu* a_menu, std::string_view a_reason)
	{
		if (!a_menu) {
			return;
		}

		const auto ingredientEntryCount = static_cast<std::uint32_t>(a_menu->ingredientEntries.size());
		const auto count = ingredientEntryCount < kMaxLoggedIngredients ?
			ingredientEntryCount :
			static_cast<std::uint32_t>(kMaxLoggedIngredients);
		for (std::uint32_t i = 0; i < count; ++i) {
			const auto& ingredientEntry = a_menu->ingredientEntries[i];
			auto* ingredient = ingredientEntry.ingredient;
			spdlog::info(
				"[{}] ingredient[{}] name='{}' countDelta={} selected={} usable={} effectFilters=[{}, {}, {}, {}]",
				a_reason,
				i,
				SafeIngredientName(ingredient),
				ingredient ? ingredient->countDelta : 0,
				ingredientEntry.isSelected != 0,
				ingredientEntry.isNotGreyed != 0,
				static_cast<std::uint32_t>(ingredientEntry.effect1FilterID),
				static_cast<std::uint32_t>(ingredientEntry.effect2FilterID),
				static_cast<std::uint32_t>(ingredientEntry.effect3FilterID),
				static_cast<std::uint32_t>(ingredientEntry.effect4FilterID));
		}

		if (ingredientEntryCount > count) {
			spdlog::info(
				"[{}] ... {} additional ingredient entries not logged",
				a_reason,
				ingredientEntryCount - count);
		}
	}
}

namespace ARV
{
	CraftingMenuWatcher* CraftingMenuWatcher::GetSingleton()
	{
		static CraftingMenuWatcher singleton;
		return std::addressof(singleton);
	}

	void CraftingMenuWatcher::Install()
	{
		auto* watcher = GetSingleton();
		if (watcher->installed_) {
			spdlog::info("CraftingMenuWatcher already installed");
			return;
		}

		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			spdlog::error("UI singleton unavailable; could not install CraftingMenuWatcher");
			return;
		}

		ui->AddEventSink<RE::MenuOpenCloseEvent>(watcher);
		watcher->installed_ = true;
		spdlog::info("Installed CraftingMenuWatcher");
	}

	RE::BSEventNotifyControl CraftingMenuWatcher::ProcessEvent(
		const RE::MenuOpenCloseEvent* a_event,
		RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		if (!a_event) {
			spdlog::warn("Received null MenuOpenCloseEvent");
			return RE::BSEventNotifyControl::kContinue;
		}

		if (SafeMenuName(a_event) != RE::CraftingMenu::MENU_NAME) {
			return RE::BSEventNotifyControl::kContinue;
		}

		spdlog::info("Crafting Menu {}", a_event->opening ? "opened" : "closed");
		if (a_event->opening) {
			CraftingMenuHook::GetSingleton()->OnCraftingMenuOpened();
		} else {
			CraftingMenuHook::GetSingleton()->OnCraftingMenuClosed();
		}

		return RE::BSEventNotifyControl::kContinue;
	}

	void CraftingMenuWatcher::QueueCurrentCraftingMenuStateLog(std::string_view a_reason, std::uint32_t a_attempt) const
	{
		auto* taskInterface = SKSE::GetTaskInterface();
		if (!taskInterface) {
			spdlog::error("[{}] SKSE task interface unavailable", a_reason);
			return;
		}

		taskInterface->AddUITask([reason = std::string(a_reason), a_attempt]()
			{
				(void) CraftingMenuWatcher::GetSingleton()->LogCurrentCraftingMenuState(reason, a_attempt, true);
			});
	}

	bool CraftingMenuWatcher::ProbeCurrentCraftingMenuState(std::string_view a_reason, bool a_allowDeferredQueue) const
	{
		return LogCurrentCraftingMenuState(a_reason, 0, a_allowDeferredQueue);
	}

	bool CraftingMenuWatcher::LogCurrentCraftingMenuState(
		std::string_view a_reason,
		std::uint32_t a_attempt,
		bool a_allowDeferredQueue) const
	{
		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			spdlog::error("[{}] UI singleton unavailable", a_reason);
			return false;
		}

		auto craftingMenuPtr = ui->GetMenu<RE::CraftingMenu>();
		if (!craftingMenuPtr) {
			spdlog::warn("[{}] CraftingMenu instance unavailable", a_reason);
			return false;
		}

		auto* craftingMenu = craftingMenuPtr.get();
		auto* subMenu = craftingMenu->GetCraftingSubMenu();
		spdlog::info(
			"[{}] attempt={} CraftingMenu={} craftingSubMenu={}",
			a_reason,
			a_attempt,
			static_cast<const void*>(craftingMenu),
			static_cast<const void*>(subMenu));

		if (!subMenu) {
			spdlog::warn("[{}] CraftingMenu has no active submenu yet", a_reason);
			if (a_allowDeferredQueue && a_attempt < kMaxDeferredAttempts) {
				QueueCurrentCraftingMenuStateLog(a_reason, a_attempt + 1);
			}
			return false;
		}

		if (IsAddressInsideMainModule(subMenu)) {
			spdlog::warn(
				"[{}] submenu pointer {} still points inside SkyrimVR.exe image; deferring inspection",
				a_reason,
				static_cast<const void*>(subMenu));
			if (a_allowDeferredQueue && a_attempt < kMaxDeferredAttempts) {
				QueueCurrentCraftingMenuStateLog(a_reason, a_attempt + 1);
			}
			return false;
		}

		auto* alchemyMenu = skyrim_cast<AlchemyMenu*>(subMenu);
		if (!alchemyMenu) {
			spdlog::info("[{}] Active crafting submenu is not alchemy", a_reason);
			return false;
		}

		spdlog::info(
			"[{}] AlchemyMenu ingredientEntries={} selectedIndexes={} currentIngredientIdx={} resultPotionEntry={} resultPotion={} unknownPotion={}",
			a_reason,
			alchemyMenu->ingredientEntries.size(),
			alchemyMenu->selectedIndexes.size(),
			alchemyMenu->currentIngredientIdx,
			static_cast<const void*>(alchemyMenu->resultPotionEntry),
			static_cast<const void*>(alchemyMenu->resultPotion),
			static_cast<const void*>(alchemyMenu->unknownPotion));

		LogSelectedIndexes(alchemyMenu, a_reason);
		LogIngredientEntries(alchemyMenu, a_reason);
		return true;
	}
}
