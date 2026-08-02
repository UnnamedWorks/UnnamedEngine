#pragma once

namespace Unnamed {
	/// @brief UI要素の更新処理へ渡されるフレーム情報です。
	struct UiUpdateContext final {
		// 前フレームからの経過時間
		float deltaTime = 0.0f;

		uint64_t frameIndex = 0;
	};
}
