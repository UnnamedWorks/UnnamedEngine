#pragma once

namespace Unnamed {
	class UiElement;

	/// @brief 画面座標に対するヒットテスト結果です
	struct UiHitTestResult final {
		// 最前面でヒットした要素です。
		UiElement* target = nullptr;

		// RootからTargetまでの要素経路です。
		std::vector<UiElement*> path;

		// 実際に走査した要素数です。
		uint32_t visitedElementCount = 0;

		// ヒットテスト形状を再生成した要素数です。
		uint32_t rebuiltGeometryCount = 0;

		// ヒットテスト形状のキャッシュを再利用した要素数です。
		uint32_t reusedGeometryCount = 0;

		// 非表示のため省略した部分木数です。
		uint32_t skippedInvisibleSubtreeCount = 0;

		// WorldRect未計算のため自身の判定を省略した要素数です。
		uint32_t skippedUntransformedElementCount = 0;

		// 要素がヒットしたか判定します。
		[[nodiscard]] bool HasHit() const noexcept;
	};
}
