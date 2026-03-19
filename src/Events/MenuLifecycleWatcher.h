#pragma once

namespace ARV
{
	class MenuLifecycleWatcher final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static MenuLifecycleWatcher& GetSingleton();
		static void                  Install();

		RE::BSEventNotifyControl ProcessEvent(
			const RE::MenuOpenCloseEvent* a_event,
			RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override;

	private:
		bool installed_{ false };
	};
}
