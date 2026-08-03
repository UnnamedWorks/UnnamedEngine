#include "pch.h"
#include "UiRootTests.h"

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

			assert(!root.HasViewportSize());

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			assert(root.HasViewportSize());

			assert(root.GetViewportSize().x == 1280.0f);

			assert(root.GetViewportSize().y == 720.0f);

			assert(root.IsLayoutInvalidated());
		}

		void TestFirstLayout() {
			UiRoot root;

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			const UiLayoutPassResult result = root.PerformLayout();

			assert(result.performedLayout);
			assert(result.stabilized);
			assert(result.passCount == 1);

			const UiRect& rootRect = root.GetLayoutRect();

			assert(rootRect.pos.x == 0.0f);

			assert(rootRect.pos.y == 0.0f);

			assert(rootRect.size.x == 1280.0f);

			assert(rootRect.size.y == 720.0f);

			assert(!root.IsLayoutInvalidated());
		}

		void TestCleanLayoutDoesNoWork() {
			UiRoot root;

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			static_cast<void>(root.PerformLayout());

			const UiLayoutPassResult result = root.PerformLayout();

			assert(!result.performedLayout);
			assert(result.stabilized);
			assert(result.passCount == 0);
		}

		void TestRootLaysOutChild() {
			UiRoot root;

			const TestRootChild& child =
				root.CreateChildAt<TestRootChild>(
					MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
				);

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			const UiLayoutPassResult result = root.PerformLayout();

			assert(result.stabilized);

			assert(child.GetMeasureCallCount() == 1);

			assert(child.GetArrangeCallCount() == 1);

			assert(child.GetLastAvailableSize().x == 100.0f);

			assert(child.GetLastAvailableSize().y == 50.0f);

			const UiRect& childRect = child.GetLayoutRect();

			assert(childRect.pos.x == 10.0f);

			assert(childRect.pos.y == 20.0f);

			assert(childRect.size.x == 100.0f);

			assert(childRect.size.y == 50.0f);
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

			assert(result.performedLayout);
			assert(result.stabilized);

			assert(root.GetLayoutRect().size.x == 1920.0f);

			assert(root.GetLayoutRect().size.y == 1080.0f);

			/*
			 * Absolute Slotのサイズは変わっていないため、
			 * 子自身のMeasure/Arrangeキャッシュは再利用される。
			 */
			assert(child.GetMeasureCallCount() == childMeasureCount);

			assert(child.GetArrangeCallCount() == childArrangeCount);
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

			assert(root.IsLayoutInvalidated());

			const UiLayoutPassResult result = root.PerformLayout();

			assert(result.performedLayout);
			assert(result.stabilized);

			assert(child.GetMeasureCallCount() == oldMeasureCount + 1);
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

			assert(result.performedLayout);
			assert(result.stabilized);
			assert(result.passCount == 2);

			assert(child.GetMeasureCallCount() == 2);
		}

		void TestSameViewportDoesNotInvalidate() {
			UiRoot root;

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			static_cast<void>(root.PerformLayout());

			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			assert(!root.IsLayoutInvalidated());

			const UiLayoutPassResult result = root.PerformLayout();

			assert(!result.performedLayout);
			assert(result.passCount == 0);
		}

		void TestZeroSizeViewport() {
			UiRoot root;

			root.SetViewportSize(Vec2(0.0f, 0.0f));

			const UiLayoutPassResult result = root.PerformLayout();

			assert(result.stabilized);

			assert(root.GetLayoutRect().size.x == 0.0f);

			assert(root.GetLayoutRect().size.y == 0.0f);
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
