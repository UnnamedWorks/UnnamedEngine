#include "pch.h"
#include "UiTreeTests.h"

#include "../core/UiContainer.h"
#include "../core/UiElement.h"
#include "../core/UiTree.h"
#include "../core/UiUpdateContext.h"

namespace Unnamed {
	namespace {
		class TestUpdateElement final : public UiElement {
		public:
			TestUpdateElement(
				const int id, std::vector<int>& updateOrder
			) : mUpdateOrder(updateOrder), mId(id) {
			}

			[[nodiscard]]
			int GetUpdateCount() const noexcept {
				return mUpdateCount;
			}

			[[nodiscard]]
			const UiUpdateContext* GetLastUpdateContext() const noexcept {
				return mLastUpdateContext;
			}

		protected:
			void OnUpdate(
				const UiUpdateContext& context
			) override {
				mUpdateOrder.push_back(mId);

				++mUpdateCount;
				mLastUpdateContext = &context;
			}

		private:
			std::vector<int>&      mUpdateOrder;
			const UiUpdateContext* mLastUpdateContext = nullptr;

			int mId          = 0;
			int mUpdateCount = 0;
		};

		class TestUpdateContainer final : public UiContainer {
		public:
			TestUpdateContainer(
				const int         id,
				std::vector<int>& updateOrder
			) : mUpdateOrder(updateOrder), mId(id) {
			}

			[[nodiscard]] int GetUpdateCount() const noexcept {
				return mUpdateCount;
			}

			[[nodiscard]]
			const UiUpdateContext* GetLastUpdateContext() const noexcept {
				return mLastUpdateContext;
			}

		protected:
			void OnUpdate(const UiUpdateContext& context) override {
				mUpdateOrder.push_back(mId);

				++mUpdateCount;
				mLastUpdateContext = &context;
			}

		private:
			std::vector<int>&      mUpdateOrder;
			const UiUpdateContext* mLastUpdateContext = nullptr;

			int mId          = 0;
			int mUpdateCount = 0;
		};

		void TestEmptyTreeUpdate() {
			std::vector<int> updateOrder;

			TestUpdateContainer root(
				0,
				updateOrder
			);

			constexpr UiUpdateContext context{
				.deltaTime  = 1.0f / 60.0f,
				.frameIndex = 100
			};

			UiTree::Update(root, context);

			UASSERT(updateOrder.size() == 1);
			UASSERT(updateOrder[0] == 0);

			UASSERT(root.GetUpdateCount() == 1);
			UASSERT(
				root.GetLastUpdateContext() ==
				&context
			);
		}

		void TestDepthFirstUpdateOrder() {
			std::vector<int> updateOrder;

			TestUpdateContainer root(
				0,
				updateOrder
			);

			TestUpdateContainer& panelA =
				root.CreateChild<TestUpdateContainer>(
					1,
					updateOrder
				);

			const TestUpdateElement& elementA =
				panelA.CreateChild<TestUpdateElement>(2, updateOrder);

			TestUpdateContainer& panelB =
				panelA.CreateChild<TestUpdateContainer>(3, updateOrder);

			const TestUpdateElement& elementB =
				panelB.CreateChild<TestUpdateElement>(4, updateOrder);

			const TestUpdateElement& elementC =
				root.CreateChild<TestUpdateElement>(5, updateOrder);

			constexpr UiUpdateContext context{
				.deltaTime  = 1.0f / 120.0f,
				.frameIndex = 250
			};

			UiTree::Update(root, context);

			const std::vector expectedOrder{0, 1, 2, 3, 4, 5};

			UASSERT(updateOrder == expectedOrder);

			UASSERT(root.GetUpdateCount() == 1);
			UASSERT(panelA.GetUpdateCount() == 1);
			UASSERT(elementA.GetUpdateCount() == 1);
			UASSERT(panelB.GetUpdateCount() == 1);
			UASSERT(elementB.GetUpdateCount() == 1);
			UASSERT(elementC.GetUpdateCount() == 1);

			UASSERT(
				root.GetLastUpdateContext() ==
				&context
			);

			UASSERT(
				panelA.GetLastUpdateContext() ==
				&context
			);

			UASSERT(
				elementA.GetLastUpdateContext() ==
				&context
			);

			UASSERT(
				panelB.GetLastUpdateContext() ==
				&context
			);

			UASSERT(
				elementB.GetLastUpdateContext() ==
				&context
			);

			UASSERT(
				elementC.GetLastUpdateContext() ==
				&context
			);
		}

		void TestMultipleFrames() {
			std::vector<int> updateOrder;

			TestUpdateContainer root(
				0,
				updateOrder
			);

			const TestUpdateElement& child =
				root.CreateChild<TestUpdateElement>(1, updateOrder);

			constexpr UiUpdateContext firstContext{
				.deltaTime  = 1.0f / 60.0f,
				.frameIndex = 1
			};

			UiTree::Update(root, firstContext);

			constexpr UiUpdateContext secondContext{
				.deltaTime  = 1.0f / 30.0f,
				.frameIndex = 2
			};

			UiTree::Update(root, secondContext);

			const std::vector expectedOrder{0, 1, 0, 1};

			UASSERT(updateOrder == expectedOrder);

			UASSERT(root.GetUpdateCount() == 2);
			UASSERT(child.GetUpdateCount() == 2);

			UASSERT(root.GetLastUpdateContext() == &secondContext);

			UASSERT(child.GetLastUpdateContext() == &secondContext);
		}
	}

	void RunUiTreeTests() {
		TestEmptyTreeUpdate();
		TestDepthFirstUpdateOrder();
		TestMultipleFrames();
	}
}
