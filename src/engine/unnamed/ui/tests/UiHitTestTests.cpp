#include "pch.h"
#include "UiHitTestTests.h"

#include "engine/unnamed/ui/controls/UiPanel.h"
#include "engine/unnamed/ui/core/UiElement.h"
#include "engine/unnamed/ui/core/UiRoot.h"
#include "engine/unnamed/ui/core/UiTransformPassResult.h"
#include "engine/unnamed/ui/core/UiTree.h"
#include "engine/unnamed/ui/layout/UiAbsoluteSlot.h"

namespace Unnamed {
	namespace {
		class TestHitElement final : public UiElement {
		public:
			[[nodiscard]]
			int GetGeometryBuildCount() const noexcept {
				return mGeometryBuildCount;
			}

		protected:
			[[nodiscard]]
			UiHitTestGeometry
			OnBuildHitTestGeometry() override {
				++mGeometryBuildCount;
				return UiElement::OnBuildHitTestGeometry();
			}

		private:
			int mGeometryBuildCount = 0;
		};

		[[nodiscard]]
		UiAbsoluteSlot MakeSlot(
			const float x,
			const float y,
			const float width,
			const float height
		) {
			return UiAbsoluteSlot{
				.rect = UiRect{
					.pos  = Vec2(x, y),
					.size = Vec2(width, height)
				}
			};
		}

		void PrepareUi(UiRoot& root) {
			/*
			 * Root自身が空白領域を捕捉しないようにする。
			 * 子要素のHit Testには影響しない。
			 */
			root.SetHitTestVisible(false);

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			const UiLayoutPassResult layoutResult = root.PerformLayout();

			UASSERT(layoutResult.stabilized);

			const UiTransformPassResult transformResult =
					UiTree::UpdateTransforms(root);

			UASSERT(transformResult .skippedUnarrangedCount == 0);
		}

		void TestBasicHit() {
			UiRoot root;

			TestHitElement& child = root.CreateChildAt<TestHitElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			const UiHitTestResult result = UiTree::HitTest(
				root,
				Vec2(50.0f, 40.0f)
			);

			UASSERT(result.HasHit());
			UASSERT(result.target == &child);

			UASSERT(result.path.size() == 2);
			UASSERT(result.path[0] == &root);
			UASSERT(result.path[1] == &child);
		}

		void TestLaterSiblingIsOnTop() {
			UiRoot root;

			TestHitElement& first = root.CreateChildAt<TestHitElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			TestHitElement& second = root.CreateChildAt<TestHitElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			static_cast<void>(first);

			PrepareUi(root);

			const UiHitTestResult result = UiTree::HitTest(
				root,
				Vec2(50.0f, 40.0f)
			);

			UASSERT(result.target == &second);
		}

		void TestChildIsInFrontOfParent() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			TestHitElement& child = panel.CreateChildAt<TestHitElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			const UiHitTestResult result = UiTree::HitTest(
				root,
				Vec2(120.0f, 80.0f)
			);

			UASSERT(result.target == &child);

			UASSERT(result.path.size() == 3);
			UASSERT(result.path[0] == &root);
			UASSERT(result.path[1] == &panel);
			UASSERT(result.path[2] == &child);
		}

		void TestHitTestVisibleFalseStillAllowsChildren() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			panel.SetHitTestVisible(false);

			TestHitElement& child = panel.CreateChildAt<TestHitElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			const UiHitTestResult result = UiTree::HitTest(
				root,
				Vec2(120.0f, 80.0f)
			);

			UASSERT(result.target == &child);
		}

		void TestHiddenSubtreeIsSkipped() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			panel.CreateChildAt<TestHitElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			panel.SetVisibility(UI_VISIBILITY::HIDDEN);

			PrepareUi(root);

			const UiHitTestResult result = UiTree::HitTest(
				root,
				Vec2(120.0f, 80.0f)
			);

			UASSERT(!result.HasHit());

			UASSERT(result .skippedInvisibleSubtreeCount == 1);
		}

		void TestRoundedPanelCorners() {
			UiRoot root;

			UiPanel& panel = root.CreateChildAt<UiPanel>(
				MakeSlot(0.0f, 0.0f, 100.0f, 40.0f)
			);

			panel.SetCornerRadii(UiCornerRadii::Uniform(20.0f));

			PrepareUi(root);

			/*
			 * 外側矩形内だが、角丸の円弧外。
			 */
			const UiHitTestResult cornerResult = UiTree::HitTest(
				root,
				Vec2(1.0f, 1.0f)
			);

			UASSERT(!cornerResult.HasHit());

			const UiHitTestResult centerResult = UiTree::HitTest(
				root,
				Vec2(50.0f, 20.0f)
			);
			UASSERT(centerResult.target == &panel);
		}

		void TestRightAndBottomEdgesAreExcluded() {
			UiRoot root;

			TestHitElement& child = root.CreateChildAt<TestHitElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			const UiHitTestResult rightEdge = UiTree::HitTest(
				root,
				Vec2(110.0f, 40.0f)
			);

			UASSERT(!rightEdge.HasHit());

			const UiHitTestResult bottomEdge = UiTree::HitTest(
				root,
				Vec2(50.0f, 70.0f)
			);

			UASSERT(!bottomEdge.HasHit());

			const UiHitTestResult inside = UiTree::HitTest(
				root,
				Vec2(109.9f, 69.9f)
			);

			UASSERT(inside.target == &child);
		}

		void TestChildOutsideParentCanBeHit() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(0.0f, 0.0f, 50.0f, 50.0f)
			);

			TestHitElement& child = panel.CreateChildAt<TestHitElement>(
				MakeSlot(100.0f, 100.0f, 20.0f, 20.0f)
			);

			PrepareUi(root);

			const UiHitTestResult result = UiTree::HitTest(
				root,
				Vec2(105.0f, 105.0f)
			);

			UASSERT(result.target == &child);
		}

		void TestDisabledAndTransparentElementCanBeHit() {
			UiRoot root;

			TestHitElement& child = root.CreateChildAt<TestHitElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			child.SetEnabled(false);
			child.SetOpacity(0.0f);

			PrepareUi(root);

			const UiHitTestResult result = UiTree::HitTest(
				root,
				Vec2(50.0f, 40.0f)
			);

			UASSERT(result.target == &child);
		}

		void TestHitTestGeometryCache() {
			UiRoot root;

			TestHitElement& child = root.CreateChildAt<TestHitElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			const UiHitTestResult firstResult = UiTree::HitTest(
				root,
				Vec2(50.0f, 40.0f)
			);

			UASSERT(firstResult.rebuiltGeometryCount == 1);

			UASSERT(child.GetGeometryBuildCount() == 1);

			const UiHitTestResult secondResult = UiTree::HitTest(
				root,
				Vec2(50.0f, 40.0f)
			);

			UASSERT(secondResult.rebuiltGeometryCount == 0);

			UASSERT(secondResult.reusedGeometryCount == 1);

			UASSERT(child.GetGeometryBuildCount() == 1);

			child.Invalidate(UI_INVALIDATION::HIT_TEST);

			const UiHitTestResult thirdResult = UiTree::HitTest(
				root,
				Vec2(50.0f, 40.0f)
			);

			UASSERT(thirdResult.rebuiltGeometryCount == 1);

			UASSERT(child.GetGeometryBuildCount() == 2);
		}

		void TestUntransformedElementIsSkipped() {
			UiElement element;

			const UiHitTestResult result = UiTree::HitTest(
				element,
				Vec2(0.0f, 0.0f)
			);

			UASSERT(!result.HasHit());

			UASSERT(result .skippedUntransformedElementCount == 1);

			UASSERT(element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));
		}
	}

	void RunUiHitTestTests() {
		TestBasicHit();
		TestLaterSiblingIsOnTop();
		TestChildIsInFrontOfParent();
		TestHitTestVisibleFalseStillAllowsChildren();
		TestHiddenSubtreeIsSkipped();
		TestRoundedPanelCorners();
		TestRightAndBottomEdgesAreExcluded();
		TestChildOutsideParentCanBeHit();
		TestDisabledAndTransparentElementCanBeHit();
		TestHitTestGeometryCache();
		TestUntransformedElementIsSkipped();
	}
}
