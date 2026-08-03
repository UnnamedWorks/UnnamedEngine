#pragma once

namespace Unnamed {
	struct UiTransformPassResult final {
		// 走査した要素数
		uint32_t mVisitedElementCount = 0;

		// ワールド矩形を再計算した要素数
		uint32_t mUpdatedElementCount = 0;

		// ワールド矩形が実際に変化した要素数
		uint32_t mChangedElementCount = 0;

		// Arrangeされていなかったため処理を省略した要素数
		uint32_t mSkippedUnarrangedCount = 0;
	};
}
