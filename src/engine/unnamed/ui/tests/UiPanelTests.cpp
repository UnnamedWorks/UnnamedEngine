#include "pch.h"
#include "UiPanelTests.h"

#include "engine/unnamed/ui/controls/UiPanel.h"
#include "engine/unnamed/ui/core/UiRoot.h"
#include "engine/unnamed/ui/core/UiTransformPassResult.h"
#include "engine/unnamed/ui/core/UiTree.h"

namespace Unnamed {
	namespace {
		constexpr float kTestEpsilon = 0.0001f;

		[[nodiscard]]
		bool IsNearlyEqual(const float first, const float second) noexcept {
			return std::abs(first - second) <= kTestEpsilon;
		}

		class TestUiPanel final : public UiPanel {
		public:
			void ClearForTest(const UI_INVALIDATION invalidation) noexcept {
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

		void PrepareUi(UiRoot& root) {
			root.SetViewportSize(Vec2(1280.0f, 720.0f));

			const UiLayoutPassResult layoutResult = root.PerformLayout();

			UASSERT(layoutResult.stabilized);

			const UiTransformPassResult transformResult =
					UiTree::UpdateTransforms(root);

			UASSERT(transformResult .mSkippedUnarrangedCount == 0);
		}

		void TestDefaultPanelProducesNoCommands() {
			UiRoot root;

			root.CreateChildAt<TestUiPanel>(
				MakeSlot(
					10.0f,
					20.0f,
					100.0f,
					50.0f
				)
			);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			UASSERT(commandList.IsEmpty());
		}

		void TestBackgroundProducesSolidRect() {
			UiRoot root;

			TestUiPanel& panel = root.CreateChildAt<TestUiPanel>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			panel.SetBackgroundColor(
				UiColor{
					.mRed   = 1.0f,
					.mGreen = 0.0f,
					.mBlue  = 0.0f,
					.mAlpha = 1.0f
				}
			);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			UASSERT(commandList.GetCommandCount() == 1);

			const auto commands = commandList.GetCommands();

			const auto* command = std::get_if<UiSolidRectPaintCommand>(
				&commands[0]
			);

			UASSERT(command != nullptr);

			UASSERT(command->mRect.pos.x == 10.0f);
			UASSERT(command->mRect.pos.y == 20.0f);
			UASSERT(command->mRect.size.x == 100.0f);
			UASSERT(command->mRect.size.y == 50.0f);
		}

		void TestRoundedBackground() {
			UiRoot root;

			TestUiPanel& panel =
					root.CreateChildAt<TestUiPanel>(
						MakeSlot(
							0.0f,
							0.0f,
							100.0f,
							40.0f
						)
					);

			panel.SetBackgroundColor(UiColor::White());

			panel.SetCornerRadii(UiCornerRadii::Uniform(30.0f));

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			const auto commands = commandList.GetCommands();

			UASSERT(commands.size() == 1);

			const auto* command = std::get_if<UiSolidRoundedRectPaintCommand>(
				&commands[0]
			);

			UASSERT(command != nullptr);

			/*
			 * 100 × 40なので最大半径は20。
			 */
			UASSERT(command ->mRoundedRect .radii .topLeft == 20.0f);
		}

		void TestBorderOnlyProducesBorderCommand() {
			UiRoot root;

			TestUiPanel& panel =
					root.CreateChildAt<TestUiPanel>(
						MakeSlot(
							0.0f,
							0.0f,
							100.0f,
							40.0f
						)
					);

			panel.SetBorderColor(
				UiColor{
					.mRed   = 0.0f,
					.mGreen = 1.0f,
					.mBlue  = 0.0f,
					.mAlpha = 1.0f
				}
			);

			panel.SetBorderThickness(4.0f);

			panel.SetCornerRadii(UiCornerRadii::Uniform(12.0f));

			PrepareUi(root);

			UiPaintCommandList commandList;

			auto result = UiTree::BuildPaintCommands(root, commandList);

			const auto commands = commandList.GetCommands();

			UASSERT(commands.size() == 1);

			const auto* command = std::get_if<UiSolidBorderPaintCommand>(
				&commands[0]
			);

			UASSERT(command != nullptr);

			UASSERT(command->thickness == 4.0f);

			UASSERT(command->roundedRect .radii .topLeft == 12.0f);

			UASSERT(command->mColor.mGreen == 1.0f);
		}

		void TestBackgroundAndBorderOrder() {
			UiRoot root;

			TestUiPanel& panel =
					root.CreateChildAt<TestUiPanel>(
						MakeSlot(
							0.0f,
							0.0f,
							100.0f,
							40.0f
						)
					);

			panel.SetBackgroundColor(UiColor::White());

			panel.SetBorderColor(
				UiColor{
					.mRed   = 0.0f,
					.mGreen = 0.0f,
					.mBlue  = 0.0f,
					.mAlpha = 1.0f
				}
			);

			panel.SetBorderThickness(2.0f);

			panel.SetCornerRadii(UiCornerRadii::Uniform(8.0f));

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			const auto commands = commandList.GetCommands();

			UASSERT(commands.size() == 2);

			UASSERT(
				std::holds_alternative< UiSolidRoundedRectPaintCommand >(
					commands[0])
			);

			UASSERT(
				std::holds_alternative< UiSolidBorderPaintCommand >(commands[1])
			);
		}

		void TestStyleChangeInvalidatesOnlyPaint() {
			TestUiPanel panel;

			panel.ClearForTest(UI_INVALIDATION::ALL);

			panel.SetBackgroundColor(
				UiColor{
					.mRed   = 1.0f,
					.mGreen = 0.0f,
					.mBlue  = 0.0f,
					.mAlpha = 1.0f
				}
			);

			UASSERT(panel.IsInvalidated( UI_INVALIDATION::PAINT ));

			UASSERT(!panel.IsInvalidated( UI_INVALIDATION::MEASURE ));

			UASSERT(!panel.IsInvalidated( UI_INVALIDATION::ARRANGE ));

			UASSERT(!panel.IsInvalidated( UI_INVALIDATION::TRANSFORM ));
		}

		void TestSameStyleDoesNotInvalidate() {
			TestUiPanel panel;

			const UiColor color{
				.mRed   = 1.0f,
				.mGreen = 0.0f,
				.mBlue  = 0.0f,
				.mAlpha = 1.0f
			};

			panel.SetBackgroundColor(color);

			panel.ClearForTest(UI_INVALIDATION::ALL);

			panel.SetBackgroundColor(color);

			UASSERT(panel.GetInvalidation() == UI_INVALIDATION::NONE);
		}

		void TestBorderThicknessIsClampedAtPaintTime() {
			UiRoot root;

			TestUiPanel& panel =
					root.CreateChildAt<TestUiPanel>(
						MakeSlot(
							0.0f,
							0.0f,
							100.0f,
							40.0f
						)
					);

			panel.SetBorderColor(UiColor::White());

			panel.SetBorderThickness(100.0f);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			const auto commands = commandList.GetCommands();

			const auto* command = std::get_if<UiSolidBorderPaintCommand>(
				&commands[0]
			);

			UASSERT(command != nullptr);

			/*
			 * 短辺40の半分なので20へ制限される。
			 */
			UASSERT(command->thickness == 20.0f);
		}

		void TestOpacityAppliesToBorder() {
			UiRoot root;

			TestUiPanel& panel =
					root.CreateChildAt<TestUiPanel>(
						MakeSlot(
							0.0f,
							0.0f,
							100.0f,
							40.0f
						)
					);

			panel.SetOpacity(0.5f);

			panel.SetBorderColor(
				UiColor{
					.mRed   = 1.0f,
					.mGreen = 1.0f,
					.mBlue  = 1.0f,
					.mAlpha = 0.8f
				}
			);

			panel.SetBorderThickness(2.0f);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			const auto commands = commandList.GetCommands();

			const auto* command = std::get_if<UiSolidBorderPaintCommand>(
				&commands[0]
			);

			UASSERT(command != nullptr);

			UASSERT(IsNearlyEqual( command->mColor.mAlpha, 0.4f ));
		}

		void TestPanelPaintCacheReuse() {
			UiRoot root;

			TestUiPanel& panel =
					root.CreateChildAt<TestUiPanel>(
						MakeSlot(
							0.0f,
							0.0f,
							100.0f,
							40.0f
						)
					);

			panel.SetBackgroundColor(UiColor::White());

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			const UiPaintPassResult result =
					UiTree::BuildPaintCommands(
						root,
						commandList
					);

			UASSERT(result.mRebuiltCacheCount == 0);

			UASSERT(result.mReusedCacheCount == 2);

			UASSERT(commandList.GetCommandCount() == 1);
		}
	}

	void RunUiPanelTests() {
		TestDefaultPanelProducesNoCommands();
		TestBackgroundProducesSolidRect();
		TestRoundedBackground();
		TestBorderOnlyProducesBorderCommand();
		TestBackgroundAndBorderOrder();
		TestStyleChangeInvalidatesOnlyPaint();
		TestSameStyleDoesNotInvalidate();
		TestBorderThicknessIsClampedAtPaintTime();
		TestOpacityAppliesToBorder();
		TestPanelPaintCacheReuse();
	}
} // namespace Unnamed
