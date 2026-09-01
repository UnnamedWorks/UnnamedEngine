#include "pch.h"
#include "UiInvalidationTests.h"

#include "../core/UiContainer.h"
#include "../core/UiElement.h"

namespace Unnamed {
	namespace {
		class TestInvalidationElement final
			: public UiElement {
		public:
			void ClearForTest(const UI_INVALIDATION invalidation) noexcept {
				ClearInvalidation(invalidation);
			}

			[[nodiscard]]
			UI_INVALIDATION GetLastNewInvalidation() const noexcept {
				return mLastNewInvalidation;
			}

			[[nodiscard]]
			int GetInvalidatedCount() const noexcept {
				return mInvalidatedCount;
			}

			void ResetNotificationForTest() noexcept {
				mLastNewInvalidation = UI_INVALIDATION::NONE;

				mInvalidatedCount = 0;
			}

		protected:
			void OnInvalidated(const UI_INVALIDATION newInvalidation) override {
				mLastNewInvalidation = newInvalidation;

				++mInvalidatedCount;
			}

		private:
			UI_INVALIDATION mLastNewInvalidation = UI_INVALIDATION::NONE;

			int mInvalidatedCount = 0;
		};

		class TestInvalidationContainer final : public UiContainer {
		public:
			void ClearForTest(const UI_INVALIDATION invalidation) noexcept {
				ClearInvalidation(invalidation);
			}

			[[nodiscard]] int GetChildrenChangedCount() const noexcept {
				return mChildrenChangedCount;
			}

		protected:
			void OnChildrenChanged() noexcept override {
				++mChildrenChangedCount;
			}

		private:
			int mChildrenChangedCount = 0;
		};

		void TestInitialInvalidation() {
			const TestInvalidationElement element;

			UASSERT(
				HasAllInvalidation( element.GetInvalidation(), UI_INVALIDATION::
					ALL )
			);

			UASSERT(element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::PAINT ));
		}

		void TestClearInvalidation() {
			TestInvalidationElement element;
			element.ClearForTest(
				UI_INVALIDATION::MEASURE | UI_INVALIDATION::PAINT
			);

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(!element.IsInvalidated (UI_INVALIDATION::PAINT));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::ARRANGE ));
		}

		void TestOpacityInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);

			element.ResetNotificationForTest();

			element.SetOpacity(0.5f);

			UASSERT(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			UASSERT(element.GetInvalidatedCount() == 1);

			UASSERT(element.GetLastNewInvalidation() == UI_INVALIDATION::PAINT);

			// 同じ値を設定しても追加通知されない。
			element.SetOpacity(0.5f);

			UASSERT(element.GetInvalidatedCount() == 1);
		}

		void TestEnabledInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);

			element.SetEnabled(false);

			UASSERT(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));
		}

		void TestHitTestVisibleInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);

			element.SetHitTestVisible(false);

			UASSERT(element.GetInvalidation() == UI_INVALIDATION::HIT_TEST);
		}

		void TestHiddenVisibilityInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);
			element.SetVisibility(UI_VISIBILITY::HIDDEN);

			UASSERT(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(!element.IsInvalidated( UI_INVALIDATION::ARRANGE ));
		}

		void TestCollapsedVisibilityInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);
			element.SetVisibility(UI_VISIBILITY::COLLAPSED);

			UASSERT(element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));
		}

		void TestMeasurePropagation() {
			TestInvalidationContainer  root;
			TestInvalidationContainer& parent =
				root.CreateChild<TestInvalidationContainer>();

			TestInvalidationElement& child =
				parent.CreateChild<TestInvalidationElement>();

			root.ClearForTest(UI_INVALIDATION::ALL);
			parent.ClearForTest(UI_INVALIDATION::ALL);
			child.ClearForTest(UI_INVALIDATION::ALL);

			child.Invalidate(UI_INVALIDATION::MEASURE);

			UASSERT(child.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(parent.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(parent.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			UASSERT(root.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(root.IsInvalidated( UI_INVALIDATION::ARRANGE ));
		}

		void TestPaintDoesNotPropagate() {
			TestInvalidationContainer root;

			TestInvalidationElement& child =
				root.CreateChild<TestInvalidationElement>();

			root.ClearForTest(UI_INVALIDATION::ALL);
			child.ClearForTest(UI_INVALIDATION::ALL);

			child.Invalidate(UI_INVALIDATION::PAINT);

			UASSERT(child.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(!root.IsInvalidated( UI_INVALIDATION::PAINT ));
		}

		void TestAddChildInvalidation() {
			TestInvalidationContainer root;

			root.ClearForTest(UI_INVALIDATION::ALL);

			root.CreateChild<TestInvalidationElement>();

			UASSERT(root.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(root.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			UASSERT(root.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(root.IsInvalidated( UI_INVALIDATION::HIT_TEST ));

			UASSERT(root.IsInvalidated( UI_INVALIDATION::CHILDREN ));

			UASSERT(root.GetChildrenChangedCount() == 1);
		}

		void TestClearChildrenNotification() {
			TestInvalidationContainer root;

			root.CreateChild<TestInvalidationElement>();
			root.CreateChild<TestInvalidationElement>();

			const int countBeforeClear = root.GetChildrenChangedCount();

			root.ClearForTest(UI_INVALIDATION::ALL);

			root.ClearChildren();

			UASSERT(root.GetChildCount() == 0);

			UASSERT(root.GetChildrenChangedCount() == countBeforeClear + 1);

			UASSERT(root.IsInvalidated( UI_INVALIDATION::CHILDREN ));
		}

		void TestMeasureAlsoInvalidatesArrange() {
			TestInvalidationElement element;

			element.ClearForTest(
				UI_INVALIDATION::ALL
			);

			element.Invalidate(
				UI_INVALIDATION::MEASURE
			);

			UASSERT(
				element.IsInvalidated(
					UI_INVALIDATION::MEASURE
				)
			);

			UASSERT(
				element.IsInvalidated(
					UI_INVALIDATION::ARRANGE
				)
			);

			UASSERT(
				!element.IsInvalidated(
					UI_INVALIDATION::PAINT
				)
			);
		}
	}

	void RunUiInvalidationTests() {
		TestInitialInvalidation();
		TestClearInvalidation();
		TestOpacityInvalidation();
		TestEnabledInvalidation();
		TestHitTestVisibleInvalidation();
		TestHiddenVisibilityInvalidation();
		TestCollapsedVisibilityInvalidation();
		TestMeasurePropagation();
		TestPaintDoesNotPropagate();
		TestAddChildInvalidation();
		TestClearChildrenNotification();
		TestMeasureAlsoInvalidatesArrange();
	}
} // namespace Unnamed
