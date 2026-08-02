#pragma once
#include "UiAbsoluteSlot.h"

#include "../core/UiContainer.h"

namespace Unnamed {
	class UiAbsolutePanel : public UiContainer {
	public:
		UiAbsolutePanel()           = default;
		~UiAbsolutePanel() override = default;

		/// @brief 指定スロットへ新しい子要素を生成します。
		template <std::derived_from<UiElement> T, typename... Args>
		T& CreateChildAt(
			const UiAbsoluteSlot& slot,
			Args&&...             args
		) {
			T& child = UiContainer::CreateChild<T>(std::forward<Args>(args)...);

			const bool succeeded = SetChildSlot(child, slot);

			UASSERT(succeeded);

			return child;
		}

		/// @brief 子要素の配置Slotを変更します。
		/// @return 子要素がこのPanelに所属している場合はtrue。
		bool SetChildSlot(UiElement& child, const UiAbsoluteSlot& slot);

		/// @brief 子要素の配置スロットを取得します。
		/// @return 子要素がこのPanelに所属していない場合はnullptrです。
		[[nodiscard]] const UiAbsoluteSlot* GetChildSlot(
			const UiElement& child
		) const noexcept;

	protected:
		[[nodiscard]] Vec2 OnMeasure(const Vec2& availableSize) override;

		void OnArrange(const UiRect& finalRect) override;

		void OnChildAdded(
			UiElement& child, size_t index
		) noexcept override;

		void OnChildRemoving(
			UiElement& child, size_t index
		) noexcept override;

	private:
		[[nodiscard]] std::optional<size_t> FindChildIndex(
			const UiElement& child
		) const noexcept;

		std::vector<UiAbsoluteSlot> mSlots;
	};
}
