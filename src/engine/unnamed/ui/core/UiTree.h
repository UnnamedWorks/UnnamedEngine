#pragma once

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

	private:
		/// @brief UI要素ツリーを再帰的に更新します。
		/// @param element 更新対象の要素
		/// @param context 現在のフレーム情報
		static void	UpdateRecursive(
			UiElement& element, const UiUpdateContext& context
		);
	};
} // namespace Unnamed
