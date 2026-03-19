#pragma once

#include "Alchemy/IngredientRegistry.h"
#include "Alchemy/PlayerAlchemySnapshot.h"

namespace ARV
{
	class RecipeModeSession final
	{
	public:
		using AlchemyMenu = RE::CraftingSubMenus::CraftingSubMenus::AlchemyMenu;
		using MenuIngredientEntry = AlchemyMenu::MenuIngredientEntry;

		static RecipeModeSession& GetSingleton();

		void OnCraftingMenuOpened(RE::GFxMovieView* a_movie);
		void OnCraftingMenuClosed();
		void BindAlchemyMenu(AlchemyMenu* a_menu);
		void OnSelectionChanged();
		void OnUserEvent(const RE::BSFixedString* a_control);

		void Toggle();
		void DisableRecipeMode();

		[[nodiscard]] bool IsMenuOpen() const noexcept;
		[[nodiscard]] bool IsEnabled() const noexcept;
		[[nodiscard]] bool HasAlchemyMenu() const noexcept;
		[[nodiscard]] bool ShouldBlockCraft() const noexcept;
		[[nodiscard]] bool IsSyntheticEntry(const RE::InventoryEntryData* a_entryData) const noexcept;
		[[nodiscard]] bool IsSyntheticForm(RE::FormID a_formID) const noexcept;
		[[nodiscard]] bool IsCurrentMovieAlchemy() const noexcept;

	private:
		void EnableRecipeMode();
		void CaptureVanillaIngredientEntries();
		void BuildSyntheticIngredientEntries();
		void ApplySyntheticIngredientEntries();
		void RestoreVanillaIngredientEntries();
		void ClearSyntheticEntries();
		void ApplyUiState();
		void RefreshMenu(bool a_fullRebuild);
		void SyncRootState();

		RE::GFxMovieView* movie_{ nullptr };
		AlchemyMenu*      alchemyMenu_{ nullptr };

		bool menuOpen_{ false };
		bool enabled_{ false };
		bool uiInjected_{ false };
		bool vanillaCaptured_{ false };

		Alchemy::PlayerAlchemySnapshot                         playerSnapshot_{};
		std::vector<MenuIngredientEntry>                       vanillaIngredientEntries_{};
		std::vector<std::unique_ptr<RE::InventoryEntryData>>  syntheticInventoryEntries_{};
		std::vector<MenuIngredientEntry>                       syntheticIngredientEntries_{};
		std::unordered_set<RE::FormID>                         syntheticFormIDs_{};
		std::unordered_set<const RE::InventoryEntryData*>      syntheticEntryPointers_{};
	};
}
