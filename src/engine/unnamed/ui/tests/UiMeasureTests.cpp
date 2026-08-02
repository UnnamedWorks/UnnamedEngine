#include "pch.h"
#include "UiMeasureTests.h"

#include "../core/UiElement.h"

namespace Unnamed {
	namespace {
		class TestMeasureElement final : public UiElement {
		public:
			void SetMeasureResultForTest(const Vec2& measureResult) noexcept {
				mMeasureResult = measureResult;
			}

			void SetInvalidateDuringMeasureForTest(
				const bool invalidate
			) noexcept {
				mInvalidateDuringMeasure =
					invalidate;
			}

			[[nodiscard]]
			int GetMeasureCallCount() const noexcept {
				return mMeasureCallCount;
			}

			[[nodiscard]]
			const Vec2& GetLastAvailableSize() const noexcept {
				return mLastAvailableSize;
			}

			void ClearForTest(
				const UI_INVALIDATION invalidation
			) noexcept {
				ClearInvalidation(invalidation);
			}

		protected:
			[[nodiscard]] Vec2 OnMeasure(const Vec2& availableSize) override {
				++mMeasureCallCount;

				mLastAvailableSize =
					availableSize;

				if (mInvalidateDuringMeasure) {
					Invalidate(
						UI_INVALIDATION::MEASURE
					);
				}

				return mMeasureResult;
			}

		private:
			Vec2 mMeasureResult{};
			Vec2 mLastAvailableSize{};

			int mMeasureCallCount = 0;

			bool mInvalidateDuringMeasure = false;
		};

		void TestDefaultMeasure() {
			UiElement element;

			const Vec2 desiredSize = element.Measure(Vec2(100.0f, 100.0f));

			UASSERT(desiredSize.x == 0.0f);
			UASSERT(desiredSize.y == 0.0f);

			UASSERT(element.GetDesiredSize().x == 0.0f);

			UASSERT(element.GetDesiredSize().y == 0.0f);

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));
		}

		void TestMeasureResult() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(
				Vec2(80.0f, 40.0f)
			);

			const Vec2 desiredSize = element.Measure(Vec2(100.0f, 100.0f));

			UASSERT(desiredSize.x == 80.0f);
			UASSERT(desiredSize.y == 40.0f);

			UASSERT(element.GetMeasureCallCount() == 1);

			UASSERT(element.GetLastAvailableSize().x == 100.0f);

			UASSERT(element.GetLastAvailableSize().y == 100.0f);
		}

		void TestMeasureCache() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(Vec2(80.0f, 40.0f));

			constexpr Vec2 availableSize(100.0f, 100.0f);

			(void)element.Measure(availableSize);
			(void)element.Measure(availableSize);
			(void)element.Measure(availableSize);

			UASSERT(element.GetMeasureCallCount() == 1);
		}

		void TestConstraintChangeRemeasures() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(Vec2(80.0f, 40.0f));

			(void)element.Measure(Vec2(100.0f, 100.0f));

			(void)element.Measure(Vec2(200.0f, 100.0f));

			UASSERT(element.GetMeasureCallCount() == 2);

			(void)element.Measure(Vec2(200.0f, 50.0f));

			UASSERT(element.GetMeasureCallCount() == 3);
		}

		void TestInvalidationRemeasures() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(Vec2(80.0f, 40.0f));

			constexpr Vec2 availableSize(100.0f, 100.0f);

			(void)element.Measure(availableSize);

			UASSERT(element.GetMeasureCallCount() == 1);

			element.Invalidate(UI_INVALIDATION::MEASURE);

			(void)element.Measure(availableSize);

			UASSERT(element.GetMeasureCallCount() == 2);
		}

		void TestCollapsedSkipsMeasure() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(Vec2(80.0f, 40.0f));

			element.SetVisibility(UI_VISIBILITY::COLLAPSED);

			const Vec2 desiredSize = element.Measure(Vec2(100.0f, 100.0f));

			UASSERT(desiredSize.x == 0.0f);
			UASSERT(desiredSize.y == 0.0f);

			UASSERT(element.GetMeasureCallCount() == 0);

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));
		}

		void TestHiddenStillMeasures() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(Vec2(80.0f, 40.0f));

			element.SetVisibility(UI_VISIBILITY::HIDDEN);

			const Vec2 desiredSize = element.Measure(Vec2(100.0f, 100.0f));

			UASSERT(desiredSize.x == 80.0f);
			UASSERT(desiredSize.y == 40.0f);

			UASSERT(element.GetMeasureCallCount() == 1);
		}

		void TestDesiredSizeCanExceedConstraint() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(Vec2(300.0f, 200.0f));

			const Vec2 desiredSize = element.Measure(Vec2(100.0f, 50.0f));

			UASSERT(desiredSize.x == 300.0f);
			UASSERT(desiredSize.y == 200.0f);
		}

		void TestInfiniteConstraint() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(Vec2(500.0f, 30.0f));

			constexpr float infinity = std::numeric_limits<float>::infinity();

			const Vec2 desiredSize = element.Measure(Vec2(infinity, 100.0f));

			UASSERT(desiredSize.x == 500.0f);
			UASSERT(desiredSize.y == 30.0f);

			UASSERT(Math::Equal(element.GetLastAvailableSize().x, infinity));

			UASSERT(Math::Equal(element.GetLastAvailableSize().y, 100.0f));
		}

		void TestMeasureClearsOnlyMeasureFlag() {
			TestMeasureElement element;

			(void)element.Measure(Vec2(100.0f, 100.0f));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));
		}

		void TestInvalidationDuringMeasureRemains() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(Vec2(80.0f, 40.0f));

			element.SetInvalidateDuringMeasureForTest(true);

			(void)element.Measure(Vec2(100.0f, 100.0f));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(element.GetMeasureCallCount() == 1);

			(void)element.Measure(Vec2(100.0f, 100.0f));

			UASSERT(element.GetMeasureCallCount() == 2);
		}

		void TestCollapsedToVisibleRemeasures() {
			TestMeasureElement element;

			element.SetMeasureResultForTest(Vec2(80.0f, 40.0f));

			element.SetVisibility(UI_VISIBILITY::COLLAPSED);

			(void)element.Measure(Vec2(100.0f, 100.0f));

			UASSERT(element.GetMeasureCallCount() == 0);

			element.SetVisibility(UI_VISIBILITY::VISIBLE);

			const Vec2 desiredSize = element.Measure(Vec2(100.0f, 100.0f));

			UASSERT(desiredSize.x == 80.0f);
			UASSERT(desiredSize.y == 40.0f);

			UASSERT(element.GetMeasureCallCount() == 1);
		}
	}

	void RunUiMeasureTests() {
		TestDefaultMeasure();
		TestMeasureResult();
		TestMeasureCache();
		TestConstraintChangeRemeasures();
		TestInvalidationRemeasures();
		TestCollapsedSkipsMeasure();
		TestHiddenStillMeasures();
		TestDesiredSizeCanExceedConstraint();
		TestInfiniteConstraint();
		TestMeasureClearsOnlyMeasureFlag();
		TestInvalidationDuringMeasureRemains();
		TestCollapsedToVisibleRemeasures();
	}
} // namespace Unnamed
