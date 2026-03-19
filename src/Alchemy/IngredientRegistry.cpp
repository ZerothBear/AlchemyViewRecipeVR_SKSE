#include "Alchemy/IngredientRegistry.h"

#include "PCH/PCH.h"

namespace
{
	std::string ResolveDisplayName(const RE::IngredientItem* a_ingredient)
	{
		if (!a_ingredient) {
			return {};
		}

		if (const auto* name = a_ingredient->GetName(); name && name[0] != '\0') {
			return name;
		}

		if (const auto* fullName = a_ingredient->GetFullName(); fullName && fullName[0] != '\0') {
			return fullName;
		}

		return std::format("Ingredient_{:08X}", a_ingredient->GetFormID());
	}
}

namespace ARV::Alchemy
{
	IngredientRegistry& IngredientRegistry::GetSingleton()
	{
		static IngredientRegistry singleton;
		return singleton;
	}

	void IngredientRegistry::Build()
	{
		records_.clear();

		auto* dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler) {
			spdlog::error("IngredientRegistry: TESDataHandler unavailable");
			return;
		}

		auto& ingredients = dataHandler->GetFormArray<RE::IngredientItem>();
		records_.reserve(ingredients.size());

		for (auto* ingredient : ingredients) {
			if (!ingredient) {
				continue;
			}

			IngredientRecord record;
			record.formID = ingredient->GetFormID();
			record.ingredient = ingredient;
			record.displayName = ResolveDisplayName(ingredient);
			record.knownEffectMask = ingredient->gamedata.knownEffectFlags;
			record.effectFormIDs.fill(0);

			for (std::size_t i = 0; i < ingredient->effects.size() && i < record.effectFormIDs.size(); ++i) {
				const auto* effect = ingredient->effects[i];
				if (!effect || !effect->baseEffect) {
					continue;
				}

				record.effectFormIDs[i] = effect->baseEffect->GetFormID();
			}

			records_.emplace(record.formID, std::move(record));
		}

		spdlog::info("IngredientRegistry: indexed {} ingredients", records_.size());
	}

	const std::unordered_map<RE::FormID, IngredientRecord>& IngredientRegistry::Records() const noexcept
	{
		return records_;
	}

	const IngredientRecord* IngredientRegistry::Find(RE::FormID a_formID) const noexcept
	{
		const auto it = records_.find(a_formID);
		return it != records_.end() ? std::addressof(it->second) : nullptr;
	}
}
