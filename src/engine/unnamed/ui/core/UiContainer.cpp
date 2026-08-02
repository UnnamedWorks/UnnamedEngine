#include "UiContainer.h"
#include "pch.h"

namespace Unnamed {
	static constexpr std::string_view kChannel = "UI";

	UiContainer::~UiContainer() {
		ClearChildren();
	}

	UiElement& UiContainer::AddChild(std::unique_ptr<UiElement> child) {
		// nullチェック
		UASSERT(child != nullptr);

		if (child == nullptr) {
			Fatal(kChannel, "Attempted to add a null UI child.");
		}

		// 親要素が既に存在する場合は追加できない
		UASSERT(!child->HasParent());

		if (child->HasParent()) {
			Fatal(
				kChannel,
				"Attempted to add a UI element that already has a parent."
			);
		}

		// 自分自身は子に追加できない
		UASSERT(child.get() != this);
		if (child.get() == this) {
			Fatal(kChannel, "A UI container cannot be added to itself.");
		}

		// 循環参照防止
		UASSERT(!child->IsAncestorOf(*this));
		if (child->IsAncestorOf(*this)) {
			Fatal(kChannel, "Adding the UI element would create a tree cycle.");
		}

		UiElement* result = child.get();

		const size_t index = mChildren.size();

		// 子要素を追加し、親要素を設定する
		mChildren.emplace_back(std::move(child));
		result->SetParent(this);

		OnChildAdded(*result, index);

		// 子要素の変更を通知する
		NotifyChildrenChanged();

		return *result;
	}

	std::unique_ptr<UiElement> UiContainer::RemoveChild(UiElement& child) {
		// 子要素の配列から指定された子要素を検索する
		const auto iterator =
			std::ranges::find_if(
				mChildren,
				[&child](const auto& current) {
					return current.get() == &child;
				}
			);

		// 子要素が見つからなかった場合は nullptr を返す
		if (iterator == mChildren.end()) {
			return nullptr;
		}

		const size_t index = static_cast<size_t>(std::distance(
			mChildren.begin(),
			iterator
		));

		OnChildRemoving(child, index);

		std::unique_ptr<UiElement> result = std::move(*iterator);

		// 子要素を削除する
		mChildren.erase(iterator);

		// 対象の親要素を nullptr に設定する
		result->SetParent(nullptr);

		// 子要素の変更を通知する
		NotifyChildrenChanged();

		return result;
	}

	bool UiContainer::DestroyChild(UiElement& child) {
		return RemoveChild(child) != nullptr;
	}

	void UiContainer::ClearChildren() noexcept {
		// 子要素が存在しない場合は何もしない
		if (mChildren.empty()) {
			return;
		}

		// 子要素を逆順に削除することで、インデックスのずれを防ぐ
		for (size_t count = mChildren.size(); count > 0; --count) {
			const size_t index = count - 1;

			UiElement& child = *mChildren[index];

			OnChildRemoving(child, index);
			child.SetParent(nullptr);
		}

		mChildren.clear();

		// 子要素の変更を通知する
		NotifyChildrenChanged();
	}

	size_t UiContainer::GetChildCount() const noexcept {
		return mChildren.size();
	}

	UiElement& UiContainer::GetChild(const size_t index) {
		UASSERT(index < mChildren.size());
		return *mChildren[index];
	}

	const UiElement& UiContainer::GetChild(const size_t index) const {
		UASSERT(index < mChildren.size());
		return *mChildren[index];
	}

	std::span<const std::unique_ptr<UiElement>>
	UiContainer::GetChildren() const noexcept {
		return {
			mChildren.data(),
			mChildren.size(),
		};
	}

	size_t UiContainer::GetTraversalChildCount() const noexcept {
		return GetChildCount();
	}

	UiElement& UiContainer::GetTraversalChild(const size_t index) {
		return GetChild(index);
	}

	void UiContainer::NotifyChildrenChanged() noexcept {
		Invalidate(
			UI_INVALIDATION::MEASURE |
			UI_INVALIDATION::ARRANGE |
			UI_INVALIDATION::PAINT |
			UI_INVALIDATION::HIT_TEST |
			UI_INVALIDATION::CHILDREN
		);

		OnChildrenChanged();
	}
} // namespace Unnamed
