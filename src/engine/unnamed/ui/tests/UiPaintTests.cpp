#include "pch.h"
#include "UiPaintTests.h"

#include "engine/unnamed/ui/core/UiElement.h"
#include "engine/unnamed/ui/core/UiRoot.h"
#include "engine/unnamed/ui/core/UiTree.h"
#include "engine/unnamed/ui/layout/UiAbsolutePanel.h"
#include "engine/unnamed/ui/paint/UiPaintContext.h"

namespace Unnamed {
	namespace {
		constexpr float kTestEpsilon = 0.0001f;

		[[nodiscard]]
		bool IsNearlyEqual(const float first, const float second) noexcept {
			return std::abs(first - second) <= kTestEpsilon;
		}

		class TestPaintElement final : public UiElement {
		public:
			void SetColorForTest(const UiColor& color) {
				mColor = color;

				Invalidate(UI_INVALIDATION::PAINT);
			}

			void SetRoundedForTest(const bool rounded) {
				if (mRounded == rounded) {
					return;
				}

				mRounded = rounded;

				Invalidate(UI_INVALIDATION::PAINT);
			}

			void SetInvalidateDuringPaintForTest(
				const bool value
			) noexcept {
				mInvalidateDuringPaint = value;
			}

			[[nodiscard]]
			int GetPaintCallCount() const noexcept {
				return mPaintCallCount;
			}

		protected:
			void OnPaint(UiPaintContext& context) override {
				++mPaintCallCount;

				if (mRounded) {
					context.DrawSolidRoundedRect(
						UiRoundedRect{
							.rect  = GetWorldRect(),
							.radii = UiCornerRadii::Uniform(20.0f)
						},
						mColor
					);
				} else {
					context.DrawSolidRect(GetWorldRect(), mColor);
				}

				if (mInvalidateDuringPaint) {
					Invalidate(UI_INVALIDATION::PAINT);
				}
			}

		private:
			UiColor mColor{
				.mRed   = 1.0f,
				.mGreen = 0.0f,
				.mBlue  = 0.0f,
				.mAlpha = 1.0f
			};

			int mPaintCallCount = 0;

			bool mRounded               = false;
			bool mInvalidateDuringPaint = false;
		};

		class TestPaintPanel final : public UiAbsolutePanel {
		public:
			void SetColorForTest(const UiColor& color) {
				mColor = color;
				Invalidate(UI_INVALIDATION::PAINT);
			}

			[[nodiscard]]
			int GetPaintCallCount() const noexcept {
				return mPaintCallCount;
			}

		protected:
			void OnPaint(
				UiPaintContext& context
			) override {
				++mPaintCallCount;

				context.DrawSolidRect(GetWorldRect(), mColor);
			}

		private:
			UiColor mColor{
				.mRed   = 0.0f,
				.mGreen = 0.0f,
				.mBlue  = 1.0f,
				.mAlpha = 1.0f
			};

			int mPaintCallCount = 0;
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

		void PrepareUi(UiRoot& root) {
			root.SetViewportSize(Vec2(1280.0f, 720.0f));
			const UiLayoutPassResult layoutResult = root.PerformLayout();

			assert(layoutResult.stabilized);

			static_cast<void>(UiTree::UpdateTransforms(root));
		}

		[[nodiscard]]
		const UiSolidRectPaintCommand&
		GetSolidRectCommand(
			const UiPaintCommandList& commandList,
			const std::size_t         index
		) {
			const auto commands = commandList.GetCommands();

			assert(index < commands.size());

			const auto* command = std::get_if<UiSolidRectPaintCommand>(
				&commands[index]
			);

			assert(command != nullptr);

			return *command;
		}

		void TestCommandListBasics() {
			UiPaintCommandList commandList;

			assert(commandList.IsEmpty());

			commandList.AddSolidRect(
				UiRect{
					.pos  = Vec2(10.0f, 20.0f),
					.size = Vec2(100.0f, 50.0f)
				},
				UiColor{
					.mRed   = 1.0f,
					.mGreen = 0.0f,
					.mBlue  = 0.0f,
					.mAlpha = 1.0f
				}
			);

			assert(!commandList.IsEmpty());
			assert(commandList.GetCommandCount() == 1);

			commandList.Clear();

			assert(commandList.IsEmpty());
		}

		void TestEmptyAndTransparentCommandsAreSkipped() {
			UiPaintCommandList commandList;

			commandList.AddSolidRect(
				UiRect{
					.pos  = Vec2(0.0f, 0.0f),
					.size = Vec2(0.0f, 100.0f)
				},
				UiColor::White()
			);

			commandList.AddSolidRect(
				UiRect{
					.pos  = Vec2(0.0f, 0.0f),
					.size = Vec2(100.0f, 100.0f)
				},
				UiColor::Transparent()
			);

			assert(commandList.IsEmpty());
		}

		void TestPaintUsesWorldRect() {
			UiRoot root;

			TestPaintElement& child = root.CreateChildAt<TestPaintElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			UiPaintCommandList commandList;

			const UiPaintPassResult result = UiTree::BuildPaintCommands(
				root,
				commandList
			);

			assert(result.mEmittedCommandCount == 1);
			assert(commandList.GetCommandCount() == 1);
			assert(child.GetPaintCallCount() == 1);

			const auto& command = GetSolidRectCommand(commandList, 0);

			assert(command.mRect.pos.x == 10.0f);
			assert(command.mRect.pos.y == 20.0f);
			assert(command.mRect.size.x == 100.0f);
			assert(command.mRect.size.y == 50.0f);

			assert(!child.IsInvalidated( UI_INVALIDATION::PAINT ));
		}

		void TestParentPaintsBeforeChild() {
			UiRoot root;

			TestPaintPanel& panel = root.CreateChildAt<TestPaintPanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			panel.CreateChildAt<TestPaintElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			assert(commandList.GetCommandCount() == 2);

			const auto& panelCommand = GetSolidRectCommand(commandList, 0);

			const auto& childCommand = GetSolidRectCommand(commandList, 1);

			assert(panelCommand.mRect.pos.x == 100.0f);

			assert(panelCommand.mRect.pos.y == 50.0f);

			assert(childCommand.mRect.pos.x == 110.0f);

			assert(childCommand.mRect.pos.y == 70.0f);
		}

		void TestPaintCacheReuse() {
			UiRoot root;

			TestPaintElement& child = root.CreateChildAt<TestPaintElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			const int paintCallCount = child.GetPaintCallCount();

			const UiPaintPassResult result = UiTree::BuildPaintCommands(
				root,
				commandList
			);

			assert(commandList.GetCommandCount() == 1);

			assert(child.GetPaintCallCount() == paintCallCount);

			assert(result.mRebuiltCacheCount == 0);
			assert(result.mReusedCacheCount == 2);
		}

		void TestPaintInvalidationRebuildsOnlyElement() {
			UiRoot root;

			TestPaintElement& child = root.CreateChildAt<TestPaintElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			const int oldPaintCallCount = child.GetPaintCallCount();

			child.SetColorForTest(
				UiColor{
					.mRed   = 0.0f,
					.mGreen = 1.0f,
					.mBlue  = 0.0f,
					.mAlpha = 1.0f
				}
			);

			const UiPaintPassResult result = UiTree::BuildPaintCommands(
				root,
				commandList
			);

			assert(result.mRebuiltCacheCount == 1);
			assert(result.mReusedCacheCount == 1);

			assert(child.GetPaintCallCount() == oldPaintCallCount + 1);

			const auto& command = GetSolidRectCommand(commandList, 0);

			assert(command.mColor.mRed == 0.0f);
			assert(command.mColor.mGreen == 1.0f);
		}

		void TestEffectiveOpacity() {
			UiRoot root;

			UiAbsolutePanel& panel = root.CreateChildAt<UiAbsolutePanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			TestPaintElement& child = panel.CreateChildAt<TestPaintElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			panel.SetOpacity(0.5f);
			child.SetOpacity(0.5f);

			child.SetColorForTest(
				UiColor{
					.mRed   = 1.0f,
					.mGreen = 0.0f,
					.mBlue  = 0.0f,
					.mAlpha = 0.8f
				}
			);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			assert(commandList.GetCommandCount() == 1);

			const auto& command = GetSolidRectCommand(commandList, 0);

			assert(IsNearlyEqual( command.mColor.mAlpha, 0.2f ));
		}

		void TestHiddenElementSkipsSubtree() {
			UiRoot root;

			TestPaintPanel& panel = root.CreateChildAt<TestPaintPanel>(
				MakeSlot(100.0f, 50.0f, 400.0f, 300.0f)
			);

			TestPaintElement& child = panel.CreateChildAt<TestPaintElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			const int panelPaintCount = panel.GetPaintCallCount();

			const int childPaintCount = child.GetPaintCallCount();

			panel.SetVisibility(UI_VISIBILITY::HIDDEN);

			static_cast<void>(root.PerformLayout());
			static_cast<void>(UiTree::UpdateTransforms(root));

			const UiPaintPassResult result = UiTree::BuildPaintCommands(
				root,
				commandList
			);

			assert(commandList.IsEmpty());

			assert(result .mSkippedInvisibleSubtreeCount == 1);

			assert(panel.GetPaintCallCount() == panelPaintCount);

			assert(child.GetPaintCallCount() == childPaintCount);
		}

		void TestVisibleAfterHiddenPaintsAgain() {
			UiRoot root;

			TestPaintElement& child = root.CreateChildAt<TestPaintElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			PrepareUi(root);

			UiPaintCommandList commandList;

			child.SetVisibility(UI_VISIBILITY::HIDDEN);

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			assert(commandList.IsEmpty());

			child.SetVisibility(UI_VISIBILITY::VISIBLE);

			const UiPaintPassResult result = UiTree::BuildPaintCommands(
				root,
				commandList
			);

			assert(commandList.GetCommandCount() == 1);
			assert(result.mRebuiltCacheCount == 1);
		}

		void TestRoundedRectIsNormalized() {
			UiRoot root;

			TestPaintElement& child = root.CreateChildAt<TestPaintElement>(
				MakeSlot(0.0f, 0.0f, 100.0f, 40.0f)
			);

			child.SetRoundedForTest(true);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			const auto commands = commandList.GetCommands();

			assert(commands.size() == 1);

			const auto* command = std::get_if<UiSolidRoundedRectPaintCommand>(
				&commands[0]
			);

			assert(command != nullptr);

			assert(command ->mRoundedRect .radii .topLeft == 20.0f);
		}

		void TestPaintInvalidationDuringPaintRemains() {
			UiRoot root;

			TestPaintElement& child = root.CreateChildAt<TestPaintElement>(
				MakeSlot(10.0f, 20.0f, 100.0f, 50.0f)
			);

			child.SetInvalidateDuringPaintForTest(true);

			PrepareUi(root);

			UiPaintCommandList commandList;

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			assert(child.IsInvalidated( UI_INVALIDATION::PAINT ));

			const int oldPaintCount = child.GetPaintCallCount();

			child.SetInvalidateDuringPaintForTest(false);

			static_cast<void>(UiTree::BuildPaintCommands(root, commandList));

			assert(child.GetPaintCallCount() == oldPaintCount + 1);

			assert(!child.IsInvalidated( UI_INVALIDATION::PAINT ));
		}

		void TestUntransformedElementIsSkipped() {
			UiElement          element;
			UiPaintCommandList commandList;

			const UiPaintPassResult result = UiTree::BuildPaintCommands(
				element,
				commandList
			);

			assert(commandList.IsEmpty());

			assert(result.mVisitedElementCount == 1);

			assert(result .mSkippedUntransformedSubtreeCount == 1);

			assert(element.IsInvalidated( UI_INVALIDATION::PAINT ));
		}
	}

	void RunUiPaintTests() {
		TestCommandListBasics();
		TestEmptyAndTransparentCommandsAreSkipped();
		TestPaintUsesWorldRect();
		TestParentPaintsBeforeChild();
		TestPaintCacheReuse();
		TestPaintInvalidationRebuildsOnlyElement();
		TestEffectiveOpacity();
		TestHiddenElementSkipsSubtree();
		TestVisibleAfterHiddenPaintsAgain();
		TestRoundedRectIsNormalized();
		TestPaintInvalidationDuringPaintRemains();
		TestUntransformedElementIsSkipped();
	}
} // namespace Unnamed
