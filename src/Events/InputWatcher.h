#pragma once

namespace ARV
{
	class InputWatcher final : public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		static InputWatcher& GetSingleton();
		static void          Install();

		RE::BSEventNotifyControl ProcessEvent(
			RE::InputEvent* const* a_event,
			RE::BSTEventSource<RE::InputEvent*>* a_source) override;

	private:
		bool installed_{ false };
	};
}
