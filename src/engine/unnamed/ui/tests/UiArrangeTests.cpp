#include "pch.h"
#include "UiArrangeTests.h"

#include "../core/UiElement.h"

#include "../geometry/UiGeometry.h"

namespace Unnamed {
	namespace {
		constexpr float kTestEpsilon = 0.0001f;

		[[nodiscard]] bool IsNearlyEqual(
			const float first, const float second
		) noexcept {
			return std::abs(first - second) <= kTestEpsilon;
		}

		[[nodiscard]]
		bool AreRectsNearlyEqual(
			const UiRect& first, const UiRect& second
		) noexcept {
			return
				IsNearlyEqual(first.pos.x, second.pos.x) &&
				IsNearlyEqual(first.pos.y, second.pos.y) &&
				IsNearlyEqual(first.size.x, second.size.x) &&
				IsNearlyEqual(first.size.y, second.size.y);
		}

		class TestArrangeElement final : public UiElement {
		public:
			[[nodiscard]] int GetArrangeCallCount() const noexcept {
				return mArrangeCallCount;
			}

			[[nodiscard]] const UiRect& GetLastOnArrangeRect() const noexcept {
				return mLastOnArrangeRect;
			}

			[[nodiscard]] const UiRect& GetObservedLayoutRect() const noexcept {
				return mObservedLayoutRect;
			}

			void SetInvalidateDuringArrangeForTest(
				const bool invalidate
			) noexcept {
				mInvalidateDuringArrange = invalidate;
			}

			void ClearForTest(const UI_INVALIDATION invalidation) noexcept {
				ClearInvalidation(invalidation);
			}

		protected:
			void OnArrange(const UiRect& finalRect) override {
				++mArrangeCallCount;

				mLastOnArrangeRect = finalRect;

				mObservedLayoutRect = GetLayoutRect();

				if (mInvalidateDuringArrange) {
					Invalidate(UI_INVALIDATION::ARRANGE);
				}
			}

		private:
			UiRect mLastOnArrangeRect{};
			UiRect mObservedLayoutRect{};

			int mArrangeCallCount = 0;

			bool mInvalidateDuringArrange = false;
		};

		void TestDefaultArrange() {
			TestArrangeElement element;

			constexpr UiRect finalRect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 1);

			UASSERT(AreRectsNearlyEqual( element.GetLayoutRect(), finalRect ));

			UASSERT(
				AreRectsNearlyEqual( element.GetLastOnArrangeRect(), finalRect )
			);

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::ARRANGE ));
		}

		void TestLayoutRectIsUpdatedBeforeOnArrange() {
			TestArrangeElement element;

			constexpr UiRect finalRect{
				.pos  = Vec2(30.0f, 40.0f),
				.size = Vec2(200.0f, 80.0f)
			};

			element.Arrange(finalRect);

			UASSERT(
				AreRectsNearlyEqual( element.GetObservedLayoutRect(), finalRect
				)
			);
		}

		void TestArrangeCache() {
			TestArrangeElement element;

			constexpr UiRect finalRect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			element.Arrange(finalRect);
			element.Arrange(finalRect);
			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 1);
		}

		void TestRectChangeRearranges() {
			TestArrangeElement element;

			element.Arrange(
				UiRect{
					.pos  = Vec2(10.0f, 20.0f),
					.size = Vec2(100.0f, 50.0f)
				}
			);

			element.Arrange(
				UiRect{
					.pos  = Vec2(20.0f, 20.0f),
					.size = Vec2(100.0f, 50.0f)
				}
			);

			UASSERT(element.GetArrangeCallCount() == 2);

			element.Arrange(
				UiRect{
					.pos  = Vec2(20.0f, 20.0f),
					.size = Vec2(200.0f, 50.0f)
				}
			);

			UASSERT(element.GetArrangeCallCount() == 3);
		}

		void TestArrangeInvalidationReruns() {
			TestArrangeElement element;

			constexpr UiRect finalRect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 1);

			element.Invalidate(UI_INVALIDATION::ARRANGE);

			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 2);
		}

		void TestLayoutChangeInvalidation() {
			TestArrangeElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);

			element.Arrange(
				UiRect{
					.pos  = Vec2(10.0f, 20.0f),
					.size = Vec2(100.0f, 50.0f)
				}
			);

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::TRANSFORM ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));
		}

		void TestSameRectArrangeDoesNotDirtyGeometry() {
			TestArrangeElement element;

			constexpr UiRect finalRect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			element.Arrange(finalRect);

			element.ClearForTest(UI_INVALIDATION::ALL);

			element.Invalidate(UI_INVALIDATION::ARRANGE);

			element.Arrange(finalRect);

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::TRANSFORM ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));
		}

		void TestHiddenStillArranges() {
			TestArrangeElement element;

			element.SetVisibility(UI_VISIBILITY::HIDDEN);

			constexpr UiRect finalRect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 1);

			UASSERT(AreRectsNearlyEqual( element.GetLayoutRect(), finalRect ));
		}

		void TestCollapsedSkipsOnArrange() {
			TestArrangeElement element;

			element.SetVisibility(UI_VISIBILITY::COLLAPSED);

			constexpr UiRect finalRect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 0);

			const UiRect& layoutRect = element.GetLayoutRect();

			UASSERT(IsNearlyEqual( layoutRect.pos.x, 10.0f ));

			UASSERT(IsNearlyEqual( layoutRect.pos.y, 20.0f ));

			UASSERT(IsNearlyEqual( layoutRect.size.x, 0.0f ));

			UASSERT(IsNearlyEqual( layoutRect.size.y, 0.0f ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::ARRANGE ));
		}

		void TestCollapsedToVisibleRearranges() {
			TestArrangeElement element;

			constexpr UiRect finalRect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			element.SetVisibility(UI_VISIBILITY::COLLAPSED);

			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 0);

			element.SetVisibility(UI_VISIBILITY::VISIBLE);

			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 1);

			UASSERT(AreRectsNearlyEqual( element.GetLayoutRect(), finalRect ));
		}

		void TestInvalidationDuringArrangeRemains() {
			TestArrangeElement element;

			element.SetInvalidateDuringArrangeForTest(true);

			constexpr UiRect finalRect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(100.0f, 50.0f)
			};

			element.Arrange(finalRect);

			UASSERT(element.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			UASSERT(element.GetArrangeCallCount() == 1);

			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 2);
		}

		void TestZeroSizeArrange() {
			TestArrangeElement element;

			constexpr UiRect finalRect{
				.pos  = Vec2(10.0f, 20.0f),
				.size = Vec2(0.0f, 0.0f)
			};

			element.Arrange(finalRect);

			UASSERT(element.GetArrangeCallCount() == 1);

			UASSERT(AreRectsNearlyEqual( element.GetLayoutRect(), finalRect ));
		}
	}

	void RunUiArrangeTests() {
		TestDefaultArrange();
		TestLayoutRectIsUpdatedBeforeOnArrange();
		TestArrangeCache();
		TestRectChangeRearranges();
		TestArrangeInvalidationReruns();
		TestLayoutChangeInvalidation();
		TestSameRectArrangeDoesNotDirtyGeometry();
		TestHiddenStillArranges();
		TestCollapsedSkipsOnArrange();
		TestCollapsedToVisibleRearranges();
		TestInvalidationDuringArrangeRemains();
		TestZeroSizeArrange();
	}
} // namespace Unnamed
