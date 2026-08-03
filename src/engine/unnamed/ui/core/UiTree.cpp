#include "pch.h"
#include "UiTree.h"

#include "UiElement.h"

#include "../utils/UiUtils.h"

namespace Unnamed {
	void UiTree::Update(UiElement& root, const UiUpdateContext& context) {
		UpdateRecursive(root, context);
	}

	UiTransformPassResult UiTree::UpdateTransforms(UiElement& root) {
		UiTransformPassResult result = {};

		UpdateTransformsRecursive(
			root,
			Vec2::zero,
			false,
			result
		);

		return result;
	}

	void UiTree::UpdateRecursive(
		UiElement& element, const UiUpdateContext& context
	) {
		// 親を先に更新する
		element.OnUpdate(context);

		const size_t childCount = element.GetTraversalChildCount();

		for (size_t index = 0; index < childCount; ++index) {
			UiElement& child = element.GetTraversalChild(index);

			UpdateRecursive(child, context);
		}
	}

	void UiTree::UpdateTransformsRecursive(
		UiElement& element, const Vec2& parentWorldPosition,
		bool       parentWorldPositionChanged, UiTransformPassResult& result
	) {
		++result.mVisitedElementCount;

		// Arrangeされていない要素は、ローカル矩形がまだ確定していないためTransformを更新できない。Transformフラグは解除せず、次回へ持ち越す。
		if (!element.mHasArrangeResult) {
			++result.mSkippedUnarrangedCount;
			return;
		}

		const bool shouldUpdate =
			!element.mHasWorldRect ||
			parentWorldPositionChanged ||
			element.IsInvalidated(UI_INVALIDATION::TRANSFORM);

		bool worldPositionChanged = false;

		if (shouldUpdate) {
			++result.mUpdatedElementCount;

			UiRect newWorldRect = element.mLayoutRect;

			newWorldRect.pos += parentWorldPosition;

			const UiRect oldWorldRect = element.mWorldRect;

			const bool hadWorldRect = element.mHasWorldRect;

			const bool worldRectChanged =
				!hadWorldRect ||
				!AreUiRectsExactlyEqual(
					oldWorldRect,
					newWorldRect
				);

			worldPositionChanged =
				!hadWorldRect ||
				!AreUiVectorsExactlyEqual(
					oldWorldRect.pos,
					newWorldRect.pos
				);

			// OnWorldRectChanged()中にTransformが再Invalidationされた場合、そのDirtyを残すために先に解除する
			element.ClearInvalidation(UI_INVALIDATION::TRANSFORM);

			element.mWorldRect = newWorldRect;

			element.mHasWorldRect = true;

			if (worldRectChanged) {
				++result.mChangedElementCount;

				// 画面上の位置とサイズが変わったため、描画と入力領域を再生成する
				element.Invalidate(
					UI_INVALIDATION::PAINT | UI_INVALIDATION::HIT_TEST
				);

				element.OnWorldRectChanged(oldWorldRect, newWorldRect);
			}
		}

		const Vec2 childParentWorldPosition = element.mWorldRect.pos;

		const size_t childCount = element.GetTraversalChildCount();

		for (size_t index = 0; index < childCount; ++index) {
			UiElement& child = element.GetTraversalChild(index);

			UpdateTransformsRecursive(
				child,
				childParentWorldPosition,
				worldPositionChanged,
				result
			);
		}
	}
} // namespace Unnamed
