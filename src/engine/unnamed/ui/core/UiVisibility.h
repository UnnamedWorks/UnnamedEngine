#pragma once

namespace Unnamed {
	/// @brief UI要素の表示状態を表します。
	enum class UI_VISIBILITY : uint8_t {
		// レイアウト・表示・入力の対象
		VISIBLE = 0,

		// レイアウトに影響しますが、描画・入力の対象外
		HIDDEN    = 1,

		// レイアウト・描画・入力の対象外
		COLLAPSED = 2,
	};
}
