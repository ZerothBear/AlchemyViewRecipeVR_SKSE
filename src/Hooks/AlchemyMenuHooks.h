#pragma once

namespace ARV
{
	class AlchemyMenuHooks final
	{
	public:
		using AlchemyMenu = RE::CraftingSubMenus::CraftingSubMenus::AlchemyMenu;

		static AlchemyMenuHooks& GetSingleton();
		static void              Install();

		void RememberCallback(std::string_view a_name, RE::FxDelegateHandler::CallbackFn* a_callback);
		void OnAccept(AlchemyMenu* a_menu);
		void OnProcessUserEvent(AlchemyMenu* a_menu, RE::BSFixedString* a_control);

		[[nodiscard]] RE::FxDelegateHandler::CallbackFn* CallbackFor(std::string_view a_name) const noexcept;

	private:
		std::unordered_map<std::string, RE::FxDelegateHandler::CallbackFn*> callbacks_{};
		bool                                                                installed_{ false };
	};
}
