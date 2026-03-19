#include "Alchemy/PlayerAlchemySnapshot.h"

#include "PCH/PCH.h"

namespace
{
	bool IsIngredient(RE::TESBoundObject& a_object)
	{
		return a_object.Is(RE::FormType::Ingredient);
	}
}

namespace ARV::Alchemy
{
	void PlayerAlchemySnapshot::Rebuild(const IngredientRegistry& a_registry)
	{
		ownedCounts_.clear();
		knownIngredients_.clear();

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			spdlog::error("PlayerAlchemySnapshot: PlayerCharacter unavailable");
			return;
		}

		const auto counts = player->GetInventoryCounts(IsIngredient);
		ownedCounts_.reserve(counts.size());

		for (const auto& [object, count] : counts) {
			if (!object || count <= 0) {
				continue;
			}

			ownedCounts_.emplace(object->GetFormID(), count);
		}

		for (const auto& [formID, record] : a_registry.Records()) {
			if (record.ingredient && record.ingredient->gamedata.knownEffectFlags != 0) {
				knownIngredients_.insert(formID);
			}
		}

		spdlog::info(
			"PlayerAlchemySnapshot: owned={} known={}",
			ownedCounts_.size(),
			knownIngredients_.size());
	}

	std::int32_t PlayerAlchemySnapshot::Count(RE::FormID a_formID) const noexcept
	{
		const auto it = ownedCounts_.find(a_formID);
		return it != ownedCounts_.end() ? it->second : 0;
	}

	bool PlayerAlchemySnapshot::HasKnownEffects(RE::FormID a_formID) const noexcept
	{
		return knownIngredients_.contains(a_formID);
	}
}
