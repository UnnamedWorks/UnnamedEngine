#pragma once
#include "../layout/UiAbsolutePanel.h"
#include "../layout/UiLayoutPassResult.h"

namespace Unnamed {
	/// @brief UIツリーの最上位要素とレイアウト入口を管理します。
	class UiRoot final : public UiAbsolutePanel {
	public:
		UiRoot()           = default;
		~UiRoot() override = default;

		/// @brief UI描画領域のサイズを設定します。
		/// @param viewportSize UI描画領域のサイズ
		void SetViewportSize(const Vec2& viewportSize);

		/// @brief 現在のUI描画領域サイズを取得します。
		/// @return UI描画領域のサイズ
		[[nodiscard]] const Vec2& GetViewportSize() const noexcept;

		/// @brief Viewportサイズが設定済みかを判定します。
		/// @return Viewportサイズが設定済みの場合true
		[[nodiscard]] bool HasViewportSize() const noexcept;

		/// @brief MeasureまたはArrangeが必要か判定します。
		/// @return MeasureまたはArrangeが必要な場合true
		[[nodiscard]] bool IsLayoutInvalidated() const noexcept;

		/// @brief UIツリーのレイアウトを必要に応じて実行します。
		/// @return レイアウト結果
		[[nodiscard]] UiLayoutPassResult PerformLayout();

		/// @brief 1フレーム内で許可する最大レイアウトパス数。
		static constexpr uint32_t kMaxLayoutPassCount = 8;

	private:
		Vec2 mViewportSize = {};

		bool mHasViewportSize    = false;
		bool mIsPerformingLayout = false;
	};
}
