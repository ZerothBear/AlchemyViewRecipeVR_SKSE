#pragma once

namespace ARV::UI
{
	class AlchemyUiInjector final
	{
	public:
		static AlchemyUiInjector& GetSingleton();

		[[nodiscard]] bool IsAlchemyMovie(RE::GFxMovieView* a_movie) const;

		void SyncRootState(RE::GFxMovieView* a_movie, bool a_enabled, std::int32_t a_toggleKey, bool a_showNavButton) const;
		void InjectButtonShim(RE::GFxMovieView* a_movie, std::int32_t a_toggleKey) const;

	private:
		[[nodiscard]] bool ResolveMenuRoot(RE::GFxMovieView* a_movie, RE::GFxValue& a_root, RE::GFxValue& a_menu) const;
	};
}
