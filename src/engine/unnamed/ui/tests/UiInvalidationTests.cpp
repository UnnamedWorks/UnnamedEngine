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

			assert(
				HasAllInvalidation( element.GetInvalidation(), UI_INVALIDATION::
					ALL )
			);

			assert(element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			assert(element.IsInvalidated( UI_INVALIDATION::PAINT ));
		}

		void TestClearInvalidation() {
			TestInvalidationElement element;
			element.ClearForTest(
				UI_INVALIDATION::MEASURE | UI_INVALIDATION::PAINT
			);

			assert(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			assert(!element.IsInvalidated (UI_INVALIDATION::PAINT));

			assert(element.IsInvalidated( UI_INVALIDATION::ARRANGE ));
		}

		void TestOpacityInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);

			element.ResetNotificationForTest();

			element.SetOpacity(0.5f);

			assert(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			assert(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			assert(!element.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			assert(element.GetInvalidatedCount() == 1);

			assert(element.GetLastNewInvalidation() == UI_INVALIDATION::PAINT);

			// 同じ値を設定しても追加通知されない。
			element.SetOpacity(0.5f);

			assert(element.GetInvalidatedCount() == 1);
		}

		void TestEnabledInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);

			element.SetEnabled(false);

			assert(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			assert(element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));

			assert(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));
		}

		void TestHitTestVisibleInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);

			element.SetHitTestVisible(false);

			assert(element.GetInvalidation() == UI_INVALIDATION::HIT_TEST);
		}

		void TestHiddenVisibilityInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);
			element.SetVisibility(UI_VISIBILITY::HIDDEN);

			assert(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			assert(element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));

			assert(!element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			assert(!element.IsInvalidated( UI_INVALIDATION::ARRANGE ));
		}

		void TestCollapsedVisibilityInvalidation() {
			TestInvalidationElement element;

			element.ClearForTest(UI_INVALIDATION::ALL);
			element.SetVisibility(UI_VISIBILITY::COLLAPSED);

			assert(element.IsInvalidated( UI_INVALIDATION::MEASURE ));

			assert(element.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			assert(element.IsInvalidated( UI_INVALIDATION::PAINT ));

			assert(element.IsInvalidated( UI_INVALIDATION::HIT_TEST ));
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

			assert(child.IsInvalidated( UI_INVALIDATION::MEASURE ));

			assert(parent.IsInvalidated( UI_INVALIDATION::MEASURE ));

			assert(parent.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			assert(root.IsInvalidated( UI_INVALIDATION::MEASURE ));

			assert(root.IsInvalidated( UI_INVALIDATION::ARRANGE ));
		}

		void TestPaintDoesNotPropagate() {
			TestInvalidationContainer root;

			TestInvalidationElement& child =
				root.CreateChild<TestInvalidationElement>();

			root.ClearForTest(UI_INVALIDATION::ALL);
			child.ClearForTest(UI_INVALIDATION::ALL);

			child.Invalidate(UI_INVALIDATION::PAINT);

			assert(child.IsInvalidated( UI_INVALIDATION::PAINT ));

			assert(!root.IsInvalidated( UI_INVALIDATION::PAINT ));
		}

		void TestAddChildInvalidation() {
			TestInvalidationContainer root;

			root.ClearForTest(UI_INVALIDATION::ALL);

			root.CreateChild<TestInvalidationElement>();

			assert(root.IsInvalidated( UI_INVALIDATION::MEASURE ));

			assert(root.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			assert(root.IsInvalidated( UI_INVALIDATION::PAINT ));

			assert(root.IsInvalidated( UI_INVALIDATION::HIT_TEST ));

			assert(root.IsInvalidated( UI_INVALIDATION::CHILDREN ));

			assert(root.GetChildrenChangedCount() == 1);
		}

		void TestClearChildrenNotification() {
			TestInvalidationContainer root;

			root.CreateChild<TestInvalidationElement>();
			root.CreateChild<TestInvalidationElement>();

			const int countBeforeClear = root.GetChildrenChangedCount();

			root.ClearForTest(UI_INVALIDATION::ALL);

			root.ClearChildren();

			assert(root.GetChildCount() == 0);

			assert(root.GetChildrenChangedCount() == countBeforeClear + 1);

			assert(root.IsInvalidated( UI_INVALIDATION::CHILDREN ));
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
	}
} // namespace Unnamed
