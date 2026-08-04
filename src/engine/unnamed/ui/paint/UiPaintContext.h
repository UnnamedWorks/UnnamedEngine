#pragma once
#include "UiPaintCommandList.h"

namespace Unnamed {
	class UiPaintContext final {
	public:
		explicit UiPaintContext(
			UiPaintCommandList& commandList
		) noexcept :
			mCommandList(commandList) {
		}

		/// @brief 単色の矩形を描画します。
		/// @param rect 描画する矩形
		/// @param color 描画する色
		void DrawSolidRect(const UiRect& rect, const UiColor& color) const;

		/// @brief 単色の角丸矩形を描画します。
		/// @param roundedRect 描画する角丸矩形
		/// @param color 描画する色
		void DrawSolidRoundedRect(
			const UiRoundedRect& roundedRect, const UiColor& color
		) const;

		/// @brief 単色の内側ボーダーを描画します。
		/// @param roundedRect 描画する角丸矩形
		/// @param thickness ボーダーの太さ
		/// @param color 描画する色
		void DrawSolidBorder(
			const UiRoundedRect& roundedRect, const float thickness,
			const UiColor&       color
		) const;

	private:
		UiPaintCommandList& mCommandList;
	};
}
