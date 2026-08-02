#include "pch.h"
#include "UiElement.h"

#include "UiContainer.h"

namespace Unnamed {
	UiContainer* UiElement::GetParent() noexcept {
		return mParent;
	}

	const UiContainer* UiElement::GetParent() const noexcept {
		return mParent;
	}

	bool UiElement::HasParent() const noexcept {
		return mParent != nullptr;
	}

	bool UiElement::IsAncestorOf(const UiElement& element) const noexcept {
		const UiElement* current = element.GetParent();

		// 祖先を辿っていき、this が見つかれば true を返す
		while (current != nullptr) {
			if (current == this) {
				return true;
			}

			current = current->GetParent();
		}

		return false;
	}

	void UiElement::SetVisibility(UI_VISIBILITY visibility) {
		// 変わらない場合は返す
		if (mVisibility == visibility) {
			return;
		}

		// 変更前の表示状態を保存し、表示状態を更新する
		const UI_VISIBILITY oldVisibility = std::exchange(
			mVisibility,
			visibility
		);

		// COLLAPSED 状態の変更があったかどうかを判定する
		const bool collapsedStateChanged =
			oldVisibility == UI_VISIBILITY::COLLAPSED ||
			mVisibility == UI_VISIBILITY::COLLAPSED;

		// 無効化状態を更新する
		if (collapsedStateChanged) {
			Invalidate(
				UI_INVALIDATION::MEASURE |
				UI_INVALIDATION::ARRANGE |
				UI_INVALIDATION::PAINT |
				UI_INVALIDATION::HIT_TEST
			);
		} else {
			Invalidate(UI_INVALIDATION::PAINT | UI_INVALIDATION::HIT_TEST);
		}

		// 変更後の表示状態を通知する
		OnVisibilityChanged(oldVisibility, mVisibility);
	}

	UI_VISIBILITY UiElement::GetVisibility() const noexcept {
		return mVisibility;
	}

	void UiElement::SetEnabled(bool enabled) {
		// 変わらない場合は返す
		if (mEnabled == enabled) {
			return;
		}

		// 変更前の有効状態を保存し、有効状態を更新する
		const bool oldEnabled = std::exchange(mEnabled, enabled);

		// 無効化状態を更新する
		Invalidate(UI_INVALIDATION::PAINT | UI_INVALIDATION::HIT_TEST);

		// 変更後の有効状態を通知する
		OnEnabledChanged(oldEnabled, mEnabled);
	}

	bool UiElement::IsEnabled() const noexcept {
		return mEnabled;
	}

	void UiElement::SetHitTestVisible(bool hitTestVisible) {
		// 変わらない場合は返す
		if (mHitTestVisible == hitTestVisible) {
			return;
		}

		// 変更前のヒットテスト対象状態を保存し、ヒットテスト対象状態を更新する
		const bool oldHitTestVisible = std::exchange(
			mHitTestVisible,
			hitTestVisible
		);

		// 無効化状態を更新する
		Invalidate(UI_INVALIDATION::HIT_TEST);

		// 変更後のヒットテスト対象状態を通知する
		OnHitTestVisibleChanged(oldHitTestVisible, mHitTestVisible);
	}

	bool UiElement::IsHitTestVisible() const noexcept {
		return mHitTestVisible;
	}

	void UiElement::SetOpacity(float opacity) {
		// 値が有限であることを確認する
		UASSERT(std::isfinite(opacity) && "UI opacity must be a finite value.");

		// 不正な値が渡された場合は 0.0f する
		if (!std::isfinite(opacity)) {
			opacity = 0.0f;
		}

		// 0.0～1.0 の範囲にクランプする
		const float clampedOpacity = std::clamp(opacity, 0.0f, 1.0f);

		// 変わらない場合は返す
		if (mOpacity == clampedOpacity) {
			return;
		}

		// 変更前の不透明度を保存し、不透明度を更新する
		const float oldOpacity = std::exchange(mOpacity, clampedOpacity);

		// 無効化状態を更新する
		Invalidate(UI_INVALIDATION::PAINT);

		// 変更後の不透明度を通知する
		OnOpacityChanged(oldOpacity, mOpacity);
	}

	float UiElement::GetOpacity() const noexcept {
		return mOpacity;
	}

	void UiElement::Invalidate(const UI_INVALIDATION invalidation) {
		// 無効化状態が NONE の場合は何もしない
		if (invalidation == UI_INVALIDATION::NONE) {
			return;
		}

		// すでに無効化されている状態を除外し、新たに無効化された状態を取得する
		const UI_INVALIDATION newInvalidation = invalidation & ~mInvalidation;

		// 無効化状態がない場合は何もしない
		if (newInvalidation == UI_INVALIDATION::NONE) {
			return;
		}

		// 無効化状態を更新する
		mInvalidation |= newInvalidation;

		// 無効化状態が変更されたことを通知する
		OnInvalidated(newInvalidation);

		// 親要素が存在する場合は、親要素に無効化状態の変更を通知する
		UiContainer* parent = GetParent();

		// 親要素が存在しない場合は何もしない
		if (parent == nullptr) {
			return;
		}

		// サイズの再計算が必要な場合は、親要素に MEASURE と ARRANGE の両方を通知する
		if (HasAnyInvalidation(newInvalidation, UI_INVALIDATION::MEASURE)) {
			parent->Invalidate(
				UI_INVALIDATION::MEASURE | UI_INVALIDATION::ARRANGE
			);

			return;
		}

		// 配置の再計算が必要な場合は、親要素に ARRANGE を通知する
		if (HasAnyInvalidation(newInvalidation, UI_INVALIDATION::ARRANGE)) {
			parent->Invalidate(UI_INVALIDATION::ARRANGE);
		}
	}

	UI_INVALIDATION UiElement::GetInvalidation() const noexcept {
		return mInvalidation;
	}

	bool UiElement::IsInvalidated(
		const UI_INVALIDATION invalidation
	) const noexcept {
		return HasAnyInvalidation(mInvalidation, invalidation);
	}

	void UiElement::ClearInvalidation(
		const UI_INVALIDATION invalidation
	) noexcept {
		mInvalidation &= ~invalidation;
	}

	void UiElement::SetParent(UiContainer* parent) noexcept {
		// 親要素が変更されていない場合は何もしない
		if (mParent == parent) {
			return;
		}

		// 親要素を変更し、OnParentChanged を呼び出す
		UiContainer* oldParent = std::exchange(mParent, parent);
		OnParentChanged(oldParent, parent);
	}

	size_t UiElement::GetTraversalChildCount() const noexcept {
		return 0; // 通常、UiElementは子を持たない。
	}

	UiElement& UiElement::GetTraversalChild(const size_t index) {
		static_cast<void>(index);

		UASSERT(false && "UiElement does not have traversal children.");

		return *this;
	}
} // namespace Unnamed
