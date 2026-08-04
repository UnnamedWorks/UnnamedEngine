#pragma once
#include <span>

#include "UiPaintCommand.h"

namespace Unnamed {
	struct UiRect;
	struct UiRoundedRect;
	struct UiColor;

	class UiPaintCommandList final {
	public:
		/// @brief 描画コマンドリストをクリアします。
		void Clear() noexcept;

		/// @brief 描画コマンドリストの容量を確保します。
		/// @param commandCount 確保する描画コマンドの数
		void Reserve(size_t commandCount);

		/// @brief 塗りつぶし矩形の描画コマンドを追加します。
		/// @param rect 描画する矩形
		/// @param color 描画する色
		void AddSolidRect(const UiRect& rect, const UiColor& color);

		/// @brief 塗りつぶし角丸矩形の描画コマンドを追加します。
		/// @param roundedRect 描画する角丸矩形
		/// @param color 描画する色
		void AddSolidRoundedRect(
			const UiRoundedRect& roundedRect, const UiColor& color
		);

		/// @brief 描画コマンドリストを結合します。
		/// @param source 結合する描画コマンドリスト
		/// @param opacityMultiplier 結合する描画コマンドリストの不透明度の乗算値
		void Append(
			const UiPaintCommandList& source, float opacityMultiplier = 1.0f
		);

		/// @brief 描画コマンドの数を取得します。
		/// @return 描画コマンドの数
		[[nodiscard]] size_t GetCommandCount() const noexcept;

		/// @brief 描画コマンドリストが空かどうかを判定します。
		/// @return 描画コマンドリストが空の場合は true、そうでない場合は false
		[[nodiscard]] bool IsEmpty() const noexcept;

		/// @brief 描画コマンドリストの描画コマンドを取得します。
		/// @return 描画コマンドリストの描画コマンド
		[[nodiscard]]
		std::span<const UiPaintCommand> GetCommands() const noexcept;

	private:
		std::vector<UiPaintCommand> mCommands;
	};
}
