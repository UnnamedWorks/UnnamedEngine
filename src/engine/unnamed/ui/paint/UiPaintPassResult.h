#pragma once

namespace Unnamed {
	/// @brief UI描画コマンド生成パスの実行結果です。
	struct UiPaintPassResult final {
		// 実際に走査した要素数
		uint32_t mVisitedElementCount = 0;

		// 描画キャッシュを再生成した要素数
		uint32_t mRebuiltCacheCount = 0;

		// 既存の描画キャッシュを再利用した要素数
		uint32_t mReusedCacheCount = 0;

		// 非表示のため省略した部分木の数
		uint32_t mSkippedInvisibleSubtreeCount = 0;

		// WorldRect未経産のため省略した部分木の数
		uint32_t mSkippedUntransformedSubtreeCount = 0;

		// 最終コマンドリストへ出力されたコマンド数
		uint32_t mEmittedCommandCount = 0;
	};
}
