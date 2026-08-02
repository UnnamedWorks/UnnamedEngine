#pragma once

namespace Unnamed {
	/// @brief UIレイアウトパスの実行結果です。
	struct UiLayoutPassResult final {
		// 実行されたMeasure/Arrangeパスの回数
		uint32_t passCount = 0;

		// レイアウトが規定回数以内に安定したか
		bool stabilized = true;

		// 実際にレイアウト処理が実行されたか
		bool performedLayout = false;
	};
}
