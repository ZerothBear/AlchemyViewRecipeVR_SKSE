#pragma once

namespace ARV::Alchemy
{
	struct IngredientRecord
	{
		RE::FormID                formID{ 0 };
		RE::IngredientItem*       ingredient{ nullptr };
		std::string               displayName;
		std::array<RE::FormID, 4> effectFormIDs{};
		std::uint16_t             knownEffectMask{ 0 };
	};

	class IngredientRegistry final
	{
	public:
		static IngredientRegistry& GetSingleton();

		void Build();

		[[nodiscard]] const std::unordered_map<RE::FormID, IngredientRecord>& Records() const noexcept;
		[[nodiscard]] const IngredientRecord*                                 Find(RE::FormID a_formID) const noexcept;

	private:
		std::unordered_map<RE::FormID, IngredientRecord> records_{};
	};
}
