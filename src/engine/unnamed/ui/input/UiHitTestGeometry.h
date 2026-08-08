#pragma once
#include <variant>

#include "engine/unnamed/ui/geometry/UiGeometry.h"

namespace Unnamed {
	/// @brief UI要素のヒットテスト形状を保持します。
	class UiHitTestGeometry final {
	public:
		/// @brief 空のヒットテスト形状を生成します。
		[[nodiscard]] static UiHitTestGeometry Empty() noexcept;

		/// @brief 矩形のヒットテスト形状を生成します。
		/// @param rect ヒットテスト形状を生成する矩形
		[[nodiscard]] static UiHitTestGeometry FromRect(const UiRect& rect);

		/// @brief 角丸矩形のヒットテスト形状を生成します。
		/// @param roundedRect ヒットテスト形状を生成する角丸矩形
		[[nodiscard]] static UiHitTestGeometry FromRoundedRect(
			const UiRoundedRect& roundedRect
		);

		/// @brief 指定座標が形状内に存在するか判定します。
		/// @param point 判定する座標
		[[nodiscard]] bool Contains(const Vec2& point) const noexcept;

		/// @brief 形状が空か判定します。
		[[nodiscard]] bool IsEmpty() const noexcept;

	private:
		using Shape = std::variant<std::monostate, UiRect, UiRoundedRect>;

		explicit UiHitTestGeometry(const Shape& shape) noexcept;

	private:
		Shape mShape;
	};
}
