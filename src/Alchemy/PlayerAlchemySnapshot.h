#pragma once

#include "Alchemy/IngredientRegistry.h"

namespace ARV::Alchemy
{
	class PlayerAlchemySnapshot
	{
	public:
		void Rebuild(const IngredientRegistry& a_registry);

		[[nodiscard]] std::int32_t Count(RE::FormID a_formID) const noexcept;
		[[nodiscard]] bool         HasKnownEffects(RE::FormID a_formID) const noexcept;

	private:
		std::unordered_map<RE::FormID, std::int32_t> ownedCounts_{};
		std::unordered_set<RE::FormID>               knownIngredients_{};
	};
}
