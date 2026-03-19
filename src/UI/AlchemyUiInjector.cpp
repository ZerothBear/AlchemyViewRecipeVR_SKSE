#include "UI/AlchemyUiInjector.h"

#include "PCH/PCH.h"

namespace
{
	constexpr auto kShimMovieName = "AlchemyRecipeViewVR.swf"sv;
	constexpr auto kArvNamespace = "arv"sv;
}

namespace ARV::UI
{
	AlchemyUiInjector& AlchemyUiInjector::GetSingleton()
	{
		static AlchemyUiInjector singleton;
		return singleton;
	}

	bool AlchemyUiInjector::ResolveMenuRoot(RE::GFxMovieView* a_movie, RE::GFxValue& a_root, RE::GFxValue& a_menu) const
	{
		if (!a_movie) {
			return false;
		}

		if (!a_movie->GetVariable(std::addressof(a_root), "_root")) {
			return false;
		}

		if (!a_movie->GetVariable(std::addressof(a_menu), "_root.Menu")) {
			return false;
		}

		return a_root.IsDisplayObject() && a_menu.IsDisplayObject();
	}

	bool AlchemyUiInjector::IsAlchemyMovie(RE::GFxMovieView* a_movie) const
	{
		RE::GFxValue root;
		RE::GFxValue menu;
		if (!ResolveMenuRoot(a_movie, root, menu)) {
			return false;
		}

		RE::GFxValue subtype;
		if (!menu.GetMember("_subtypeName", std::addressof(subtype)) || !subtype.IsString()) {
			return false;
		}

		const auto* subtypeName = subtype.GetString();
		return subtypeName && std::string_view(subtypeName) == "Alchemy";
	}

	void AlchemyUiInjector::SyncRootState(
		RE::GFxMovieView* a_movie,
		bool a_enabled,
		std::int32_t a_toggleKey,
		bool a_showNavButton) const
	{
		RE::GFxValue root;
		RE::GFxValue menu;
		if (!ResolveMenuRoot(a_movie, root, menu)) {
			return;
		}

		RE::GFxValue arvState;
		a_movie->CreateObject(std::addressof(arvState));
		arvState.SetMember("enabled", RE::GFxValue(a_enabled));
		arvState.SetMember("toggleKey", RE::GFxValue(a_toggleKey));
		arvState.SetMember("showNavButton", RE::GFxValue(a_showNavButton));
		root.SetMember(kArvNamespace.data(), arvState);
	}

	void AlchemyUiInjector::InjectButtonShim(RE::GFxMovieView* a_movie, std::int32_t a_toggleKey) const
	{
		RE::GFxValue root;
		RE::GFxValue menu;
		if (!ResolveMenuRoot(a_movie, root, menu)) {
			return;
		}

		const auto instanceName = std::format("AlchemyHelperUI_{}", a_toggleKey);

		RE::GFxValue existingClip;
		if (menu.GetMember(instanceName.c_str(), std::addressof(existingClip)) && existingClip.IsDisplayObject()) {
			return;
		}

		RE::GFxValue shimClip;
		if (!menu.CreateEmptyMovieClip(std::addressof(shimClip), instanceName.c_str(), 8543)) {
			spdlog::error("AlchemyUiInjector: failed to create shim movie clip '{}'", instanceName);
			return;
		}

		std::array args{ RE::GFxValue(kShimMovieName) };
		if (!shimClip.Invoke("loadMovie", nullptr, args.data(), static_cast<RE::UPInt>(args.size()))) {
			spdlog::error("AlchemyUiInjector: failed to load {}", kShimMovieName);
			return;
		}

		spdlog::info("AlchemyUiInjector: injected {} into '{}'", kShimMovieName, instanceName);
	}
}
