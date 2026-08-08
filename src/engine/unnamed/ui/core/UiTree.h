#pragma once
#include "UiTransformPassResult.h"
#include "../paint/UiPaintPassResult.h"
#include "engine/unnamed/ui/input/UiHitTestResult.h"

namespace Unnamed {
	class UiElement;
	class UiPaintCommandList;

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

		/// @brief UI要素ツリーの描画コマンドを構築します。
		/// @param root 描画コマンドを構築するルート要素
		/// @param outputCommands 描画コマンドの出力先
		/// @return 描画コマンドの構築結果
		[[nodiscard]] static UiPaintPassResult BuildPaintCommands(
			UiElement& root, UiPaintCommandList& outputCommands
		);

		/// @brief 指定された画面座標から最前面のUI要素を検索します。
		/// @param root 検索対象のルート要素
		[[nodiscard]] static UiHitTestResult HitTest(
			UiElement& root, const Vec2& worldPoint
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

		/// @brief UI要素ツリーの描画コマンドを再帰的に構築します。
		/// @param element 描画コマンドを構築する要素
		/// @param parentOpacity 親要素の不透明度
		/// @param outputCommands 描画コマンドの出力先
		/// @param result 描画コマンドの構築結果を格納する構造体
		static void BuildPaintCommandsRecursive(
			UiElement&          element,
			float               parentOpacity,
			UiPaintCommandList& outputCommands,
			UiPaintPassResult&  result
		);

		/// @brief UI要素ツリーを再帰的にヒットテストします。
		/// @param element ヒットテスト対象の要素
		/// @param worldPoint ヒットテストするワールド座標
		/// @param result ヒットテスト結果を格納する構造体
		/// @return ヒットした要素が存在する場合は true、存在しない場合は false
		[[nodiscard]] static bool HitTestRecursive(
			UiElement& element, const Vec2& worldPoint, UiHitTestResult& result
		);
	};
} // namespace Unnamed
