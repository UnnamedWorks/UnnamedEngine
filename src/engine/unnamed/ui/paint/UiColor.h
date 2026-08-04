#pragma once

namespace Unnamed {
	/// @brief UI描画で使用する色構造体です。
	struct UiColor final {
		float mRed, mGreen, mBlue, mAlpha = 1.0f;

		/// @brief 透明な色を返します。
		/// @return 透明な色
		[[nodiscard]] static constexpr UiColor Transparent() noexcept {
			return UiColor{
				.mRed   = 0.0f,
				.mGreen = 0.0f,
				.mBlue  = 0.0f,
				.mAlpha = 0.0f
			};
		}

		/// @brief 白色を返します。
		/// @return 白色
		[[nodiscard]] static constexpr UiColor White() noexcept {
			return UiColor{
				.mRed   = 1.0f,
				.mGreen = 1.0f,
				.mBlue  = 1.0f,
				.mAlpha = 1.0f
			};
		}

		/// @brief 色が有効かどうかを返します。
		/// @return 色が有効な場合はtrue、そうでない場合はfalse
		[[nodiscard]] bool IsValid() const noexcept;

		/// @brief 色を正規化して返します。無効な色の場合は、各成分を0.0fにクランプします。
		/// @return 正規化された色
		[[nodiscard]] UiColor GetSanitized() const noexcept;

		/// @brief 親要素の不透明度を考慮した色を返します。
		/// @param opacityMultiplier 親要素の不透明度の乗算値
		/// @return 親要素の不透明度を考慮した色s
		[[nodiscard]] UiColor WithOpacityMultiplier(
			float opacityMultiplier
		) const noexcept;
	};
}
