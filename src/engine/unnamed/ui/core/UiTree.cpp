#include "pch.h"
#include "UiTree.h"

#include "UiElement.h"

#include "../utils/UiUtils.h"
#include "engine/unnamed/ui/paint/UiPaintContext.h"

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

	UiPaintPassResult UiTree::BuildPaintCommands(
		UiElement& root, UiPaintCommandList& outputCommands
	) {
		outputCommands.Clear();

		UiPaintPassResult result = {};

		BuildPaintCommandsRecursive(
			root,
			1.0f,
			outputCommands,
			result
		);

		result.mEmittedCommandCount =
				static_cast<uint32_t>(outputCommands.GetCommandCount());

		return result;
	}

	UiHitTestResult UiTree::HitTest(UiElement& root, const Vec2& worldPoint) {
		UASSERT(
			worldPoint.IsValid() &&
			"UI Hit Test point must be finite."
		);

		UiHitTestResult result = {};

		if (!worldPoint.IsValid()) {
			return result;
		}

		static_cast<void>(HitTestRecursive(root, worldPoint, result));

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
		++result.visitedElementCount;

		// Arrangeされていない要素は、ローカル矩形がまだ確定していないためTransformを更新できない。Transformフラグは解除せず、次回へ持ち越す。
		if (!element.mHasArrangeResult) {
			++result.skippedUnarrangedCount;
			return;
		}

		const bool shouldUpdate =
				!element.mHasWorldRect ||
				parentWorldPositionChanged ||
				element.IsInvalidated(UI_INVALIDATION::TRANSFORM);

		bool worldPositionChanged = false;

		if (shouldUpdate) {
			++result.updatedElementCount;

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
				++result.changedElementCount;

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

	void UiTree::BuildPaintCommandsRecursive(
		UiElement&          element, float                     parentOpacity,
		UiPaintCommandList& outputCommands, UiPaintPassResult& result
	) {
		++result.mVisitedElementCount;

		// HIDDENまたはCOLLAPSEDな親の子は個別にVISIBLEでも画面へ表示しない。
		if (element.GetVisibility() != UI_VISIBILITY::VISIBLE) {
			++result.mSkippedInvisibleSubtreeCount;
			return;
		}

		// ワールド座標が確定していない要素は描画できない。PaintDirtyは解除せず、次回へ持ち越す。
		if (!element.mHasWorldRect) {
			++result.mSkippedUntransformedSubtreeCount;
			return;
		}

		const float effectiveOpacity = std::clamp(
			parentOpacity * element.GetOpacity(),
			0.0f,
			1.0f
		);

		const bool shouldRebuildCache =
				!element.mHasPaintCommandCache ||
				element.IsInvalidated(UI_INVALIDATION::PAINT);

		if (shouldRebuildCache) {
			++result.mRebuiltCacheCount;

			// OnPaint()中にPaintが再Invalidationされた場合、そのDirtyを残すために先に解除する
			element.ClearInvalidation(UI_INVALIDATION::PAINT);

			element.mPaintCommandCache.Clear();

			UiPaintContext paintContext(element.mPaintCommandCache);

			element.OnPaint(paintContext);

			element.mHasPaintCommandCache = true;
		} else {
			++result.mReusedCacheCount;
		}

		// キャッシュ内の色自体は要素固有のAlphaだけを保持し、親を含む実効Opacityは出力時に適用する。
		outputCommands.Append(element.mPaintCommandCache, effectiveOpacity);

		const size_t childCount = element.GetTraversalChildCount();

		for (size_t index = 0; index < childCount; ++index) {
			UiElement& child = element.GetTraversalChild(index);

			BuildPaintCommandsRecursive(
				child,
				effectiveOpacity,
				outputCommands,
				result
			);
		}
	}

	bool UiTree::HitTestRecursive(
		UiElement& element, const Vec2& worldPoint, UiHitTestResult& result
	) {
		++result.visitedElementCount;

		if (element.GetVisibility() != UI_VISIBILITY::VISIBLE) {
			++result.skippedInvisibleSubtreeCount;
			return false;
		}

		result.path.emplace_back(&element);

		// 描画順と逆順に検索する。あとから追加された子ほど手前に描画されるため、ヒットテストでも先に調べる。
		const size_t childCount = element.GetTraversalChildCount();

		for (size_t count = childCount; count > 0; --count) {
			const size_t index = count - 1;

			UiElement& child = element.GetTraversalChild(index);

			if (HitTestRecursive(child, worldPoint, result)) {
				return true;
			}
		}

		// HitTestVisibleは自身にのみ作用する。falseでも子要素の検索は上で実効済み
		if (element.IsHitTestVisible()) {
			if (!element.mHasWorldRect) {
				++result.skippedUntransformedElementCount;
			} else {
				const bool shouldRebuildGeometry =
						!element.mHasHitTestGeometry ||
						element.IsInvalidated(UI_INVALIDATION::HIT_TEST);

				if (shouldRebuildGeometry) {
					++result.rebuiltGeometryCount;

					// OnBuildHitTestGeometry()中の再Invalidationを残すため、呼び出し前に解除する
					element.ClearInvalidation(UI_INVALIDATION::HIT_TEST);

					element.mHitTestGeometry = element.OnBuildHitTestGeometry();

					element.mHasHitTestGeometry = true;
				} else {
					++result.reusedGeometryCount;
				}

				if (element.mHitTestGeometry.Contains(worldPoint)) {
					result.target = &element;
					return true;
				}
			}
		}
		result.path.pop_back();

		return false;
	}
} // namespace Unnamed
