#include "pch.h"
#include "UiTree.h"

#include "UiElement.h"

namespace Unnamed {
	void UiTree::Update(UiElement& root, const UiUpdateContext& context) {
		UpdateRecursive(root, context);
	}

	void UiTree::UpdateRecursive(
		UiElement& element, const UiUpdateContext& context) {
		// 親を先に更新する
		element.OnUpdate(context);

		const size_t childCount = element.GetTraversalChildCount();

		for (size_t index = 0; index < childCount; ++index) {
			UiElement& child = element.GetTraversalChild(index);

			UpdateRecursive(child, context);
		}
	}
} // namespace Unnamed
