#pragma once
#include "UiTransformPassResult.h"

namespace Unnamed {
	class UiElement;

	struct UiUpdateContext;

	/// @brief UI要素ツリーに対する共通処理を管理します。
	class UiTree {
	public:
		UiTree() = delete; // インスタンス化禁止

		/// @brief UI要素ツリーを更新します。
		/// @param root 更新対象のルート要素
		/// @param context 現在のフレーム情報
		static void Update(UiElement& root, const UiUpdateContext& context);

		/// @brief UI要素ツリーのワールド矩形を更新します。
		/// @param root 更新対象のルート要素
		/// @return ワールド矩形の更新結果
		[[nodiscard]] static UiTransformPassResult UpdateTransforms(
			UiElement& root
		);

	private:
		/// @brief UI要素ツリーを再帰的に更新します。
		/// @param element 更新対象の要素
		/// @param context 現在のフレーム情報
		static void UpdateRecursive(
			UiElement& element, const UiUpdateContext& context
		);

		/// @brief UI要素ツリーのワールド矩形を再帰的に更新します。
		/// @param element 更新対象の要素
		/// @param parentWorldPosition 親要素のワールド座標
		/// @param parentWorldPositionChanged 親要素のワールド座標が変化したか?
		/// @param result 更新結果を格納する構造体
		static void UpdateTransformsRecursive(
			UiElement&             element,
			const Vec2&            parentWorldPosition,
			bool                   parentWorldPositionChanged,
			UiTransformPassResult& result
		);
	};
} // namespace Unnamed
