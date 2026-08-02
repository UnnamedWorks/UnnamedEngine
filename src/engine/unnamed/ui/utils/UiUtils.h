#pragma once

namespace Unnamed {
	struct UiRect;

	/// @brief 2つの Vec2 が正確に等しいかどうかを判定します。
	/// @param first 判定する Vec2
	/// @param second 判定する Vec2
	/// @return 等しい場合は true、そうでない場合は false
	[[nodiscard]] bool AreUiVectorsExactlyEqual(
		const Vec2& first, const Vec2& second
	) noexcept;

	/// @brief 2つの UiRect が正確に等しいかどうかを判定します。
	/// @param first 判定する UiRect
	/// @param second 判定する UiRect
	/// @return 等しい場合は true、そうでない場合は false
	[[nodiscard]] bool AreUiRectsExactlyEqual(
		const UiRect& first, const UiRect& second
	) noexcept;
}
