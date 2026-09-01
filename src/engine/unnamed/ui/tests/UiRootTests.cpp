#include "pch.h"
#include "UiRootTests.h"

#include "../../../../core/UnnamedMacro.h"

#include "../core/UiElement.h"
#include "../core/UiRoot.h"

#include "../layout/UiLayoutPassResult.h"

namespace Unnamed {
	namespace {
		class TestRootChild final : public UiElement {
		public:
			void InvalidateMeasureOnce() {
				mInvalidateMeasureOnce = true;

				Invalidate(UI_INVALIDATION::MEASURE);
			}

			[[nodiscard]]
			int GetMeasureCallCount() const noexcept {
				return mMeasureCallCount;
			}

			[[nodiscard]]
			int GetArrangeCallCount() const noexcept {
				return mArrangeCallCount;
			}

			[[nodiscard]]
			const Vec2& GetLastAvailableSize() const noexcept {
				return mLastAvailableSize;
			}

		protected:
			[[nodiscard]]
			Vec2 OnMeasure(
				const Vec2& availableSize
			) override {
				++mMeasureCallCount;

				mLastAvailableSize = availableSize;

				if (mInvalidateMeasureOnce) {
					mInvalidateMeasureOnce = false;

					Invalidate(UI_INVALIDATION::MEASURE);
				}

				return availableSize;
			}

			void OnArrange(
				const UiRect&


			) override {
				++mArrangeCallCount;
			}

		private:
			Vec2 mLastAvailableSize{};

			int mMeasureCallCount = 0;
			int mArrangeCallCount = 0;

			bool mInvalidateMeasureOnce = false;
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

		void TestViewportSize() {
			UiRoot root;

			UASSERT(!root.HasViewportSize());

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			UASSERT(root.HasViewportSize());

			UASSERT(root.GetViewportSize().x == 1280.0f);

			UASSERT(root.GetViewportSize().y == 720.0f);

			UASSERT(root.IsLayoutInvalidated());
		}

		void TestFirstLayout() {
			UiRoot root;

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			const UiLayoutPassResult result = root.PerformLayout();

			UASSERT(result.performedLayout);
			UASSERT(result.stabilized);
			UASSERT(result.passCount == 1);

			const UiRect& rootRect = root.GetLayoutRect();

			UASSERT(rootRect.pos.x == 0.0f);

			UASSERT(rootRect.pos.y == 0.0f);

			UASSERT(rootRect.size.x == 1280.0f);

			UASSERT(rootRect.size.y == 720.0f);

			UASSERT(!root.IsLayoutInvalidated());
		}

		void TestCleanLayoutDoesNoWork() {
			UiRoot root;

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			static_cast<void>(root.PerformLayout());

			const UiLayoutPassResult result = root.PerformLayout();

			UASSERT(!result.performedLayout);
			UASSERT(result.stabilized);
			UASSERT(result.passCount == 0);
		}

		void TestRootLaysOutChild() {
			UiRoot root;

			const TestRootChild& child =
				root.CreateChildAt<TestRootChild>(
					MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
				);

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			const UiLayoutPassResult result = root.PerformLayout();

			UASSERT(result.stabilized);

			UASSERT(child.GetMeasureCallCount() == 1);

			UASSERT(child.GetArrangeCallCount() == 1);

			UASSERT(child.GetLastAvailableSize().x == 100.0f);

			UASSERT(child.GetLastAvailableSize().y == 50.0f);

			const UiRect& childRect = child.GetLayoutRect();

			UASSERT(childRect.pos.x == 10.0f);

			UASSERT(childRect.pos.y == 20.0f);

			UASSERT(childRect.size.x == 100.0f);

			UASSERT(childRect.size.y == 50.0f);
		}

		void TestViewportResize() {
			UiRoot root;

			const TestRootChild& child =
				root.CreateChildAt<TestRootChild>(
					MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
				);

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			static_cast<void>(root.PerformLayout());

			const int childMeasureCount = child.GetMeasureCallCount();

			const int childArrangeCount = child.GetArrangeCallCount();

			root.SetViewportSize(Vec2(1920.0f, 1080.0f));

			const UiLayoutPassResult result = root.PerformLayout();

			UASSERT(result.performedLayout);
			UASSERT(result.stabilized);

			UASSERT(root.GetLayoutRect().size.x == 1920.0f);

			UASSERT(root.GetLayoutRect().size.y == 1080.0f);

			/*
			 * Absolute Slotのサイズは変わっていないため、
			 * 子自身のMeasure/Arrangeキャッシュは再利用される。
			 */
			UASSERT(child.GetMeasureCallCount() == childMeasureCount);

			UASSERT(child.GetArrangeCallCount() == childArrangeCount);
		}

		void TestChildMeasureInvalidation() {
			UiRoot root;

			TestRootChild& child =
				root.CreateChildAt<TestRootChild>(
					MakeSlot(0.0f, 0.0f, 100.0f, 50.0f)
				);

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			static_cast<void>(root.PerformLayout());

			const int oldMeasureCount = child.GetMeasureCallCount();

			child.Invalidate(UI_INVALIDATION::MEASURE);

			UASSERT(root.IsLayoutInvalidated());

			const UiLayoutPassResult result = root.PerformLayout();

			UASSERT(result.performedLayout);
			UASSERT(result.stabilized);

			UASSERT(child.GetMeasureCallCount() == oldMeasureCount + 1);
		}

		void TestLayoutCanRequireSecondPass() {
			UiRoot root;

			TestRootChild& child = root.CreateChildAt<TestRootChild>(
				MakeSlot(0.0f, 0.0f, 100.0f, 50.0f)
			);

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			/*
			 * 最初のOnMeasure中に、一度だけMeasureを
			 * 再Invalidationする。
			 */
			child.InvalidateMeasureOnce();

			const UiLayoutPassResult result = root.PerformLayout();

			UASSERT(result.performedLayout);
			UASSERT(result.stabilized);
			UASSERT(result.passCount == 2);

			UASSERT(child.GetMeasureCallCount() == 2);
		}

		void TestSameViewportDoesNotInvalidate() {
			UiRoot root;

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			static_cast<void>(root.PerformLayout());

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			UASSERT(!root.IsLayoutInvalidated());

			const UiLayoutPassResult result = root.PerformLayout();

			UASSERT(!result.performedLayout);
			UASSERT(result.passCount == 0);
		}

		void TestZeroSizeViewport() {
			UiRoot root;

			root.SetViewportSize(Vec2(0.0f, 0.0f));

			const UiLayoutPassResult result = root.PerformLayout();

			UASSERT(result.stabilized);

			UASSERT(root.GetLayoutRect().size.x == 0.0f);

			UASSERT(root.GetLayoutRect().size.y == 0.0f);
		}
	}

	void RunUiRootTests() {
		TestViewportSize();
		TestFirstLayout();
		TestCleanLayoutDoesNoWork();
		TestRootLaysOutChild();
		TestViewportResize();
		TestChildMeasureInvalidation();
		TestLayoutCanRequireSecondPass();
		TestSameViewportDoesNotInvalidate();
		TestZeroSizeViewport();
	}
} // namespace Unnamed
