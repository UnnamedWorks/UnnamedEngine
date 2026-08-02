#include "pch.h"
#include "UiOwnershipTests.h"

#include "../core/UiContainer.h"
#include "../core/UiElement.h"

namespace Unnamed {
	namespace {
		class TestElement final : public UiElement {
		public:
			explicit TestElement(int& destructionCount) : mDestructionCount(
				destructionCount
			) {
			}

			~TestElement() override {
				++mDestructionCount;
			}

		private:
			int& mDestructionCount;
		};

		class TestContainer final : public UiContainer {
		};

		void TestCreateChild() {
			int destructionCount = 0;

			{
				TestContainer root;

				TestElement& child =
					root.CreateChild<TestElement>(
						destructionCount
					);

				UASSERT(root.GetChildCount() == 1);
				UASSERT(&root.GetChild(0) == &child);

				UASSERT(child.HasParent());
				UASSERT(child.GetParent() == &root);
			}

			UASSERT(destructionCount == 1);
		}

		void TestNestedChildren() {
			int destructionCount = 0;

			{
				TestContainer root;

				TestContainer& panel =
					root.CreateChild<TestContainer>();

				TestElement& button =
					panel.CreateChild<TestElement>(
						destructionCount
					);

				UASSERT(root.GetChildCount() == 1);
				UASSERT(panel.GetChildCount() == 1);

				UASSERT(panel.GetParent() == &root);
				UASSERT(button.GetParent() == &panel);

				UASSERT(root.IsAncestorOf(panel));
				UASSERT(root.IsAncestorOf(button));
				UASSERT(panel.IsAncestorOf(button));

				UASSERT(!button.IsAncestorOf(panel));
				UASSERT(!button.IsAncestorOf(root));
			}

			UASSERT(destructionCount == 1);
		}

		void TestRemoveChild() {
			int destructionCount = 0;

			TestContainer root;

			TestElement& child =
				root.CreateChild<TestElement>(
					destructionCount
				);

			std::unique_ptr<UiElement> removedChild =
				root.RemoveChild(child);

			UASSERT(removedChild != nullptr);
			UASSERT(removedChild.get() == &child);

			UASSERT(root.GetChildCount() == 0);
			UASSERT(!removedChild->HasParent());
			UASSERT(removedChild->GetParent() == nullptr);

			// RemoveChildでは破棄されない。
			UASSERT(destructionCount == 0);

			removedChild.reset();

			UASSERT(destructionCount == 1);
		}

		void TestReparentChild() {
			int destructionCount = 0;

			TestContainer firstParent;
			TestContainer secondParent;

			TestElement& child =
				firstParent.CreateChild<TestElement>(
					destructionCount
				);

			const UiElement* originalAddress = &child;

			std::unique_ptr<UiElement> removedChild =
				firstParent.RemoveChild(child);

			UASSERT(removedChild != nullptr);
			UASSERT(firstParent.GetChildCount() == 0);
			UASSERT(removedChild->GetParent() == nullptr);

			UiElement& reparentedChild =
				secondParent.AddChild(
					std::move(removedChild)
				);

			UASSERT(&reparentedChild == originalAddress);
			UASSERT(secondParent.GetChildCount() == 1);
			UASSERT(reparentedChild.GetParent() == &secondParent);

			// unique_ptrを移動してもUiElement自体のアドレスは変わらない。
			UASSERT(&secondParent.GetChild(0) == originalAddress);

			UASSERT(destructionCount == 0);
		}

		void TestDestroyChild() {
			int destructionCount = 0;

			TestContainer root;

			TestElement& child =
				root.CreateChild<TestElement>(
					destructionCount
				);

			const bool destroyed =
				root.DestroyChild(child);

			UASSERT(destroyed);
			UASSERT(root.GetChildCount() == 0);
			UASSERT(destructionCount == 1);
		}

		void TestDestroyUnknownChild() {
			int firstDestructionCount  = 0;
			int secondDestructionCount = 0;

			TestContainer firstParent;
			TestContainer secondParent;

			TestElement& firstChild =
				firstParent.CreateChild<TestElement>(
					firstDestructionCount
				);

			TestElement& secondChild =
				secondParent.CreateChild<TestElement>(
					secondDestructionCount
				);

			const bool destroyed =
				firstParent.DestroyChild(secondChild);

			UASSERT(!destroyed);

			UASSERT(firstParent.GetChildCount() == 1);
			UASSERT(secondParent.GetChildCount() == 1);

			UASSERT(firstChild.GetParent() == &firstParent);
			UASSERT(secondChild.GetParent() == &secondParent);

			UASSERT(firstDestructionCount == 0);
			UASSERT(secondDestructionCount == 0);
		}

		void TestClearChildren() {
			int destructionCount = 0;

			TestContainer root;

			root.CreateChild<TestElement>(destructionCount);
			root.CreateChild<TestElement>(destructionCount);
			root.CreateChild<TestElement>(destructionCount);

			UASSERT(root.GetChildCount() == 3);

			root.ClearChildren();

			UASSERT(root.GetChildCount() == 0);
			UASSERT(destructionCount == 3);

			// 空の状態でも安全に呼べることを確認する。
			root.ClearChildren();

			UASSERT(root.GetChildCount() == 0);
			UASSERT(destructionCount == 3);
		}

		void TestRecursiveDestruction() {
			int destructionCount = 0;

			{
				TestContainer root;

				TestContainer& firstPanel =
					root.CreateChild<TestContainer>();

				TestContainer& secondPanel =
					firstPanel.CreateChild<TestContainer>();

				secondPanel.CreateChild<TestElement>(
					destructionCount
				);

				secondPanel.CreateChild<TestElement>(
					destructionCount
				);

				UASSERT(root.GetChildCount() == 1);
				UASSERT(firstPanel.GetChildCount() == 1);
				UASSERT(secondPanel.GetChildCount() == 2);
			}

			// Rootの破棄だけですべての子孫が破棄される。
			UASSERT(destructionCount == 2);
		}
	}

	void RunUiOwnershipTests() {
		TestCreateChild();
		TestNestedChildren();
		TestRemoveChild();
		TestReparentChild();
		TestDestroyChild();
		TestDestroyUnknownChild();
		TestClearChildren();
		TestRecursiveDestruction();
	}
}
