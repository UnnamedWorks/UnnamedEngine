#pragma once
#include "UiPanelStyle.h"
#include "engine/unnamed/ui/layout/UiAbsolutePanel.h"

namespace Unnamed {
	class UiPaintContext;

	class UiPanel : public UiAbsolutePanel {
	public:
		UiPanel() noexcept           = default;
		~UiPanel() noexcept override = default;

		/// @brief パネルの視覚スタイルを設定します。
		/// @param style 設定するスタイル
		void SetStyle(const UiPanelStyle& style);

		/// @brief パネルの視覚スタイルを取得します。
		/// @return パネルの視覚スタイル
		[[nodiscard]] const UiPanelStyle& GetStyle() const noexcept;

		/// @brief パネルの背景色を設定します。
		/// @param color 設定する背景色
		void SetBackgroundColor(const UiColor& color);

		/// @brief 背景色を取得します。
		/// @return 背景色
		[[nodiscard]] const UiColor& GetBackgroundColor() const noexcept;

		/// @brief パネルのBorder色を設定します。
		/// @param color 設定するBorder色
		void SetBorderColor(const UiColor& color);

		/// @brief パネルのBorder色を取得します。
		/// @return Border色
		[[nodiscard]] const UiColor& GetBorderColor() const noexcept;

		/// @brief パネルのBorder幅を設定します。
		/// @param thickness 設定するBorder幅
		void SetBorderThickness(float thickness);

		/// @brief パネルのBorder幅を取得します。
		/// @return Border幅
		[[nodiscard]] float GetBorderThickness() const noexcept;

		/// @brief パネルの角丸半径を設定します。
		/// @param cornerRadii 設定する角丸半径
		void SetCornerRadii(const UiCornerRadii& cornerRadii);

		/// @brief パネルの角丸半径を取得します。
		/// @return 角丸半径
		[[nodiscard]] const UiCornerRadii& GetCornerRadii() const noexcept;

	protected:
		void OnPaint(UiPaintContext& context) override;

		[[nodiscard]] UiHitTestGeometry OnBuildHitTestGeometry() override;

	private:
		UiPanelStyle mStyle = {};
	};
}
