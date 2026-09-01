#include "pch.h"
#include "UiTransformTests.h"

#include "../core/UiElement.h"
#include "../core/UiRoot.h"
#include "../core/UiTransformPassResult.h"
#include "../core/UiTree.h"

#include "../layout/UiAbsoluteSlot.h"

#include "../utils/UiUtils.h"

namespace Unnamed {
	namespace {
		class TestTransformElement final
			: public UiElement {
		public:
			[[nodiscard]]
			int GetWorldRectChangedCount() const noexcept {
				return mWorldRectChangedCount;
			}

			[[nodiscard]]
			const UiRect& GetLastOldWorldRect() const noexcept {
				return mLastOldWorldRect;
			}

			[[nodiscard]]
			const UiRect& GetLastNewWorldRect() const noexcept {
				return mLastNewWorldRect;
			}

			[[nodiscard]]
			const UiRect& GetObservedWorldRect() const noexcept {
				return mObservedWorldRect;
			}

			void ClearForTest(
				const UI_INVALIDATION invalidation
			) noexcept {
				ClearInvalidation(invalidation);
			}

			void SetInvalidateTransformOnChangeForTest(
				const bool value
			) noexcept {
				mInvalidateTransformOnChange = value;
			}

		protected:
			void OnWorldRectChanged(
				const UiRect& oldWorldRect,
				const UiRect& newWorldRect
			) override {
				++mWorldRectChangedCount;

				mLastOldWorldRect = oldWorldRect;

				mLastNewWorldRect = newWorldRect;

				/*
				 * コールバック時点でGetWorldRect()が
				 * 新しい値になっていることを確認する。
				 */
				mObservedWorldRect = GetWorldRect();

				if (mInvalidateTransformOnChange) {
					Invalidate(UI_INVALIDATION::TRANSFORM);
				}
			}

		private:
			UiRect mLastOldWorldRect{};
			UiRect mLastNewWorldRect{};
			UiRect mObservedWorldRect{};

			int mWorldRectChangedCount = 0;

			bool mInvalidateTransformOnChange = false;
		};

		[[nodiscard]]
		UiAbsoluteSlot MakeSlot(
			const float x,
			const float y,
			const float width,
			const float height
		) {
			return UiAbsoluteSlot{
				.rect = UiRect{.pos = Vec2(x, y), .size = Vec2(width, height)}
			};
		}

		void PrepareLayout(UiRoot& root) {
			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			const UiLayoutPassResult result = root.PerformLayout();

			UASSERT(result.stabilized);
		}

		void TestNestedWorldCoordinates() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			const TestTransformElement& child = panel.CreateChildAt<
				TestTransformElement>(MakeSlot(10.0f, 20.0f, 120.0f, 40.0f));

			PrepareLayout(root);

			const UiTransformPassResult result = UiTree::UpdateTransforms(root);

			UASSERT(result.visitedElementCount == 3);
			UASSERT(result.updatedElementCount == 3);

			UASSERT(result.changedElementCount == 3);

			UASSERT(result.skippedUnarrangedCount == 0);

			UASSERT(root.HasWorldRect());
			UASSERT(panel.HasWorldRect());
			UASSERT(child.HasWorldRect());

			const UiRect& rootWorld = root.GetWorldRect();

			UASSERT(rootWorld.pos.x == 0.0f);

			UASSERT(rootWorld.pos.y == 0.0f);

			UASSERT(rootWorld.size.x == 1280.0f);

			UASSERT(rootWorld.size.y == 720.0f);

			const UiRect& panelWorld = panel.GetWorldRect();

			UASSERT(panelWorld.pos.x == 100.0f);

			UASSERT(panelWorld.pos.y == 50.0f);

			const UiRect& childWorld = child.GetWorldRect();

			UASSERT(childWorld.pos.x == 110.0f);
			UASSERT(childWorld.pos.y == 70.0f);
			UASSERT(childWorld.size.x == 120.0f);
			UASSERT(childWorld.size.y == 40.0f);

			/*
			 * Transformを実行してもLocal LayoutRectは
			 * 変更されない。
			 */
			UASSERT(child.GetLayoutRect().pos.x == 10.0f);

			UASSERT(child.GetLayoutRect().pos.y == 20.0f);

			UASSERT(!root.IsInvalidated( UI_INVALIDATION::TRANSFORM ));

			UASSERT(!panel.IsInvalidated( UI_INVALIDATION::TRANSFORM ));

			UASSERT(!child.IsInvalidated( UI_INVALIDATION::TRANSFORM ));
		}

		void TestCleanTransformPassUsesCache() {
			UiRoot root;

			const TestTransformElement& child = root.CreateChildAt<
				TestTransformElement>(MakeSlot(10.0f, 20.0f, 100.0f, 50.0f));

			PrepareLayout(root);

			static_cast<void>(UiTree::UpdateTransforms(root));

			const int callbackCount = child.GetWorldRectChangedCount();

			const UiTransformPassResult result = UiTree::UpdateTransforms(root);

			UASSERT(result.visitedElementCount == 2);

			UASSERT(result.updatedElementCount == 0);

			UASSERT(result.changedElementCount == 0);

			UASSERT(child.GetWorldRectChangedCount() == callbackCount);
		}

		void TestParentMovementUpdatesDescendant() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			TestTransformElement& child = panel.CreateChildAt<
				TestTransformElement>(MakeSlot(10.0f, 20.0f, 120.0f, 40.0f));

			PrepareLayout(root);
			static_cast<void>(UiTree::UpdateTransforms(root));

			child.ClearForTest(
				UI_INVALIDATION::PAINT | UI_INVALIDATION::HIT_TEST
			);

			root.SetChildSlot(panel, MakeSlot(200.0f, 100.0f, 400.0f, 300.0f));

			static_cast<void>(root.PerformLayout());

			const UiTransformPassResult result = UiTree::UpdateTransforms(root);

			/*
			 * Panelは自身のTransform Dirtyで更新され、
			 * Childは親のWorld Position変更によって更新される。
			 */
			UASSERT(result.updatedElementCount == 2);

			UASSERT(result.changedElementCount == 2);

			UASSERT(panel.GetWorldRect().pos.x == 200.0f);

			UASSERT(panel.GetWorldRect().pos.y == 100.0f);

			UASSERT(child.GetWorldRect().pos.x == 210.0f);

			UASSERT(child.GetWorldRect().pos.y == 120.0f);

			/*
			 * Childのローカル位置は変わらない。
			 */
			UASSERT(child.GetLayoutRect().pos.x == 10.0f);

			UASSERT(child.GetLayoutRect().pos.y == 20.0f);

			UASSERT(child.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(child.IsInvalidated( UI_INVALIDATION::HIT_TEST ));
		}

		void TestChildLocalMovementUpdatesOnlyChild() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			TestTransformElement& child = panel.CreateChildAt<
				TestTransformElement>(MakeSlot(10.0f, 20.0f, 120.0f, 40.0f));

			PrepareLayout(root);
			static_cast<void>(UiTree::UpdateTransforms(root));

			panel.SetChildSlot(child, MakeSlot(30.0f, 40.0f, 120.0f, 40.0f));

			static_cast<void>(root.PerformLayout());

			const UiTransformPassResult result = UiTree::UpdateTransforms(root);

			UASSERT(result.updatedElementCount == 1);

			UASSERT(result.changedElementCount == 1);

			UASSERT(child.GetWorldRect().pos.x == 130.0f);

			UASSERT(child.GetWorldRect().pos.y == 90.0f);
		}

		void TestParentSizeChangeDoesNotUpdateChildWorld() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			const TestTransformElement& child = panel.CreateChildAt<
				TestTransformElement>(MakeSlot(10.0f, 20.0f, 120.0f, 40.0f));

			PrepareLayout(root);
			static_cast<void>(UiTree::UpdateTransforms(root));

			const int childCallbackCount = child.GetWorldRectChangedCount();

			root.SetChildSlot(panel, MakeSlot(100.0f, 50.0f, 800.0f, 600.0f));

			static_cast<void>(root.PerformLayout());

			const UiTransformPassResult result = UiTree::UpdateTransforms(root);

			/*
			 * PanelのWorldRectサイズだけが変わる。
			 * PanelのWorld Positionは同じなのでChildは更新不要。
			 */
			UASSERT(result.updatedElementCount == 1);

			UASSERT(result.changedElementCount == 1);

			UASSERT(child.GetWorldRectChangedCount() == childCallbackCount);

			UASSERT(child.GetWorldRect().pos.x == 110.0f);

			UASSERT(child.GetWorldRect().pos.y == 70.0f);
		}

		void TestCollapsedWorldRect() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			TestTransformElement& child =
				panel.CreateChildAt<TestTransformElement>(
					MakeSlot(10.0f, 20.0f, 120.0f, 40.0f)
				);

			child.SetVisibility(UI_VISIBILITY::COLLAPSED);

			PrepareLayout(root);
			static_cast<void>(UiTree::UpdateTransforms(root));

			const UiRect& worldRect = child.GetWorldRect();

			UASSERT(worldRect.pos.x == 110.0f);

			UASSERT(worldRect.pos.y == 70.0f);

			UASSERT(worldRect.size.x == 0.0f);

			UASSERT(worldRect.size.y == 0.0f);
		}

		void TestWorldRectUpdatedBeforeCallback() {
			UiRoot root;

			const TestTransformElement& child =
				root.CreateChildAt<TestTransformElement>(
					MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
				);

			PrepareLayout(root);
			static_cast<void>(UiTree::UpdateTransforms(root));

			const UiRect& observed = child.GetObservedWorldRect();

			const UiRect& notified = child.GetLastNewWorldRect();

			UASSERT(AreUiRectsExactlyEqual( observed, notified ));

			UASSERT(AreUiRectsExactlyEqual( child.GetWorldRect(), notified ));
		}

		void TestTransformInvalidationDuringCallbackRemains() {
			UiRoot root;

			TestTransformElement& child =
				root.CreateChildAt<TestTransformElement>(
					MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
				);

			child.SetInvalidateTransformOnChangeForTest(true);

			PrepareLayout(root);

			static_cast<void>(UiTree::UpdateTransforms(root));

			UASSERT(child.IsInvalidated(UI_INVALIDATION::TRANSFORM));

			const int callbackCount = child.GetWorldRectChangedCount();

			/*
			 * 2回目は再計算されるがWorldRectは同じなので、
			 * OnWorldRectChanged()は呼ばれない。
			 */
			const UiTransformPassResult result = UiTree::UpdateTransforms(root);

			UASSERT(result.updatedElementCount == 1);

			UASSERT(result.changedElementCount == 0);

			UASSERT(child.GetWorldRectChangedCount() == callbackCount);

			UASSERT(!child.IsInvalidated( UI_INVALIDATION::TRANSFORM ));
		}

		void TestUnarrangedElementIsSkipped() {
			UiElement element;

			const UiTransformPassResult result = UiTree::UpdateTransforms(
				element
			);

			UASSERT(result.visitedElementCount == 1);

			UASSERT(result.updatedElementCount == 0);

			UASSERT(result.changedElementCount == 0);

			UASSERT(result.skippedUnarrangedCount == 1);

			UASSERT(!element.HasWorldRect());

			UASSERT(element.IsInvalidated( UI_INVALIDATION::TRANSFORM ));
		}
	} // namespace

	void RunUiTransformTests() {
		TestNestedWorldCoordinates();
		TestCleanTransformPassUsesCache();
		TestParentMovementUpdatesDescendant();
		TestChildLocalMovementUpdatesOnlyChild();
		TestParentSizeChangeDoesNotUpdateChildWorld();
		TestCollapsedWorldRect();
		TestWorldRectUpdatedBeforeCallback();
		TestTransformInvalidationDuringCallbackRemains();
		TestUnarrangedElementIsSkipped();
	}
}
