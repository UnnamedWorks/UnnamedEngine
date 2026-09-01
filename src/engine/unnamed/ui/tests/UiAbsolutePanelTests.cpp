#include "pch.h"
#include "UiAbsolutePanelTests.h"

#include "../../../../core/UnnamedMacro.h"

#include "../core/UiElement.h"

#include "../layout/UiAbsolutePanel.h"

namespace Unnamed {
	namespace {
		class TestLayoutElement final
			: public UiElement {
		public:
			void SetDesiredSizeForTest(
				const Vec2& desiredSize
			) noexcept {
				mDesiredSizeForTest =
					desiredSize;

				Invalidate(
					UI_INVALIDATION::MEASURE
				);
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

			[[nodiscard]]
			const UiRect& GetLastArrangeRect() const noexcept {
				return mLastArrangeRect;
			}

		protected:
			[[nodiscard]]
			Vec2 OnMeasure(
				const Vec2& availableSize
			) override {
				++mMeasureCallCount;

				mLastAvailableSize =
					availableSize;

				return mDesiredSizeForTest;
			}

			void OnArrange(
				const UiRect& finalRect
			) override {
				++mArrangeCallCount;

				mLastArrangeRect =
					finalRect;
			}

		private:
			Vec2 mDesiredSizeForTest{};
			Vec2 mLastAvailableSize{};

			UiRect mLastArrangeRect{};

			int mMeasureCallCount = 0;
			int mArrangeCallCount = 0;
		};

		class TestAbsolutePanel final
			: public UiAbsolutePanel {
		public:
			void ClearForTest(
				const UI_INVALIDATION invalidation
			) noexcept {
				ClearInvalidation(invalidation);
			}
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

		void TestDefaultSlotCreated() {
			TestAbsolutePanel panel;

			const TestLayoutElement& child =
				panel.CreateChild<
					TestLayoutElement
				>();

			const UiAbsoluteSlot* slot =
				panel.GetChildSlot(child);

			UASSERT(slot != nullptr);

			UASSERT(
				slot->rect.pos.x ==
				0.0f
			);

			UASSERT(
				slot->rect.pos.y ==
				0.0f
			);

			UASSERT(
				slot->rect.size.x ==
				0.0f
			);

			UASSERT(
				slot->rect.size.y ==
				0.0f
			);
		}

		void TestCreateChildAt() {
			TestAbsolutePanel panel;

			const TestLayoutElement& child =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						10.0f,
						20.0f,
						100.0f,
						50.0f
					)
				);

			const UiAbsoluteSlot* slot =
				panel.GetChildSlot(child);

			UASSERT(slot != nullptr);

			UASSERT(
				slot->rect.pos.x ==
				10.0f
			);

			UASSERT(
				slot->rect.pos.y ==
				20.0f
			);

			UASSERT(
				slot->rect.size.x ==
				100.0f
			);

			UASSERT(
				slot->rect.size.y ==
				50.0f
			);
		}

		void TestMeasureChildren() {
			TestAbsolutePanel panel;

			TestLayoutElement& first =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						10.0f,
						20.0f,
						100.0f,
						50.0f
					)
				);

			TestLayoutElement& second =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						200.0f,
						5.0f,
						40.0f,
						30.0f
					)
				);

			first.SetDesiredSizeForTest(
				Vec2(300.0f, 100.0f)
			);

			second.SetDesiredSizeForTest(
				Vec2(20.0f, 10.0f)
			);

			const Vec2 desiredSize =
				panel.Measure(
					Vec2(1000.0f, 1000.0f)
				);

			UASSERT(
				first.GetMeasureCallCount() == 1
			);

			UASSERT(
				second.GetMeasureCallCount() == 1
			);

			UASSERT(
				first.GetLastAvailableSize().x ==
				100.0f
			);

			UASSERT(
				first.GetLastAvailableSize().y ==
				50.0f
			);

			UASSERT(
				second.GetLastAvailableSize().x ==
				40.0f
			);

			UASSERT(
				second.GetLastAvailableSize().y ==
				30.0f
			);

			// max(10 + 100, 200 + 40)
			UASSERT(desiredSize.x == 240.0f);

			// max(20 + 50, 5 + 30)
			UASSERT(desiredSize.y == 70.0f);
		}

		void TestCollapsedChildDoesNotContribute() {
			TestAbsolutePanel panel;

			panel.CreateChildAt<
				TestLayoutElement
			>(
				MakeSlot(
					10.0f,
					10.0f,
					50.0f,
					20.0f
				)
			);

			TestLayoutElement& collapsed =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						1000.0f,
						1000.0f,
						500.0f,
						500.0f
					)
				);

			collapsed.SetVisibility(
				UI_VISIBILITY::COLLAPSED
			);

			const Vec2 desiredSize =
				panel.Measure(
					Vec2(2000.0f, 2000.0f)
				);

			UASSERT(desiredSize.x == 60.0f);
			UASSERT(desiredSize.y == 30.0f);

			UASSERT(
				collapsed.GetMeasureCallCount() ==
				0
			);
		}

		void TestHiddenChildStillContributes() {
			TestAbsolutePanel panel;

			TestLayoutElement& hidden =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						100.0f,
						50.0f,
						200.0f,
						80.0f
					)
				);

			hidden.SetVisibility(
				UI_VISIBILITY::HIDDEN
			);

			const Vec2 desiredSize =
				panel.Measure(
					Vec2(1000.0f, 1000.0f)
				);

			UASSERT(desiredSize.x == 300.0f);
			UASSERT(desiredSize.y == 130.0f);

			UASSERT(
				hidden.GetMeasureCallCount() == 1
			);
		}

		void TestArrangeUsesLocalCoordinates() {
			TestAbsolutePanel panel;

			const TestLayoutElement& child =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						10.0f,
						20.0f,
						100.0f,
						50.0f
					)
				);

			static_cast<void>(panel.Measure(Vec2(1000.0f, 1000.0f)));

			panel.Arrange(
				UiRect{
					.pos =
					Vec2(500.0f, 300.0f),

					.size =
					Vec2(400.0f, 200.0f)
				}
			);

			const UiRect& childRect =
				child.GetLayoutRect();

			/*
			 * 親の(500, 300)は加算されない。
			 * Child LayoutRectは親ローカル座標。
			 */
			UASSERT(
				childRect.pos.x ==
				10.0f
			);

			UASSERT(
				childRect.pos.y ==
				20.0f
			);

			UASSERT(
				childRect.size.x ==
				100.0f
			);

			UASSERT(
				childRect.size.y ==
				50.0f
			);

			UASSERT(
				child.GetArrangeCallCount() == 1
			);
		}

		void TestSlotChangeInvalidatesPanel() {
			TestAbsolutePanel panel;

			TestLayoutElement& child =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						10.0f,
						20.0f,
						100.0f,
						50.0f
					)
				);

			panel.ClearForTest(
				UI_INVALIDATION::ALL
			);

			const bool succeeded =
				panel.SetChildSlot(
					child,
					MakeSlot(
						30.0f,
						40.0f,
						200.0f,
						80.0f
					)
				);

			UASSERT(succeeded);

			UASSERT(
				panel.IsInvalidated(
					UI_INVALIDATION::MEASURE
				)
			);

			UASSERT(
				panel.IsInvalidated(
					UI_INVALIDATION::ARRANGE
				)
			);
		}

		void TestSameSlotDoesNotInvalidate() {
			TestAbsolutePanel panel;

			const UiAbsoluteSlot slot =
				MakeSlot(
					10.0f,
					20.0f,
					100.0f,
					50.0f
				);

			TestLayoutElement& child =
				panel.CreateChildAt<
					TestLayoutElement
				>(slot);

			panel.ClearForTest(
				UI_INVALIDATION::ALL
			);

			const bool succeeded =
				panel.SetChildSlot(
					child,
					slot
				);

			UASSERT(succeeded);

			UASSERT(
				panel.GetInvalidation() ==
				UI_INVALIDATION::NONE
			);
		}

		void TestSlotSizeChangeRemeasuresChild() {
			TestAbsolutePanel panel;

			TestLayoutElement& child =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						0.0f,
						0.0f,
						100.0f,
						50.0f
					)
				);

			static_cast<void>(panel.Measure(Vec2(1000.0f, 1000.0f)));

			UASSERT(
				child.GetMeasureCallCount() == 1
			);

			panel.SetChildSlot(
				child,
				MakeSlot(
					0.0f,
					0.0f,
					200.0f,
					80.0f
				)
			);

			static_cast<void>(panel.Measure(Vec2(1000.0f, 1000.0f)));

			UASSERT(
				child.GetMeasureCallCount() == 2
			);

			UASSERT(
				child.GetLastAvailableSize().x ==
				200.0f
			);

			UASSERT(
				child.GetLastAvailableSize().y ==
				80.0f
			);
		}

		void TestRemoveChildMaintainsSlotMapping() {
			TestAbsolutePanel panel;

			TestLayoutElement& first =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						10.0f,
						20.0f,
						100.0f,
						50.0f
					)
				);

			const TestLayoutElement& second =
				panel.CreateChildAt<
					TestLayoutElement
				>(
					MakeSlot(
						200.0f,
						300.0f,
						400.0f,
						500.0f
					)
				);

			const std::unique_ptr<UiElement> removed =
				panel.RemoveChild(first);

			UASSERT(removed != nullptr);
			UASSERT(panel.GetChildCount() == 1);

			const UiAbsoluteSlot* secondSlot =
				panel.GetChildSlot(second);

			UASSERT(secondSlot != nullptr);

			UASSERT(
				secondSlot->rect.pos.x ==
				200.0f
			);

			UASSERT(
				secondSlot->rect.pos.y ==
				300.0f
			);

			UASSERT(
				secondSlot->rect.size.x ==
				400.0f
			);

			UASSERT(
				secondSlot->rect.size.y ==
				500.0f
			);
		}

		void TestUnknownChildSlotFails() {
			TestAbsolutePanel panel;
			TestLayoutElement unrelated;

			const bool succeeded =
				panel.SetChildSlot(
					unrelated,
					MakeSlot(
						10.0f,
						20.0f,
						100.0f,
						50.0f
					)
				);

			UASSERT(!succeeded);

			UASSERT(
				panel.GetChildSlot(unrelated) ==
				nullptr
			);
		}
	}

	void RunUiAbsolutePanelTests() {
		TestDefaultSlotCreated();
		TestCreateChildAt();
		TestMeasureChildren();
		TestCollapsedChildDoesNotContribute();
		TestHiddenChildStillContributes();
		TestArrangeUsesLocalCoordinates();
		TestSlotChangeInvalidatesPanel();
		TestSameSlotDoesNotInvalidate();
		TestSlotSizeChangeRemeasuresChild();
		TestRemoveChildMaintainsSlotMapping();
		TestUnknownChildSlotFails();
	}
} // namespace Unnamed
