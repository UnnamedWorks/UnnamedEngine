#include "pch.h"
#include "UiElement.h"

#include "UiContainer.h"

#include "../utils/UiUtils.h"

namespace Unnamed {
	namespace {
		/// @brief Measure() の引数として有効な値かどうかを判定します。
		/// @param value 判定する値
		/// @return 有効な値の場合は true、無効な値の場合は false
		[[nodiscard]] bool IsValidMeasureAxis(const float value) noexcept {
			return !std::isnan(value) && value >= 0.0f;
		}

		/// @brief Measure() の引数として有効な Vec2 かどうかを判定します。
		/// @param size 判定する Vec2
		/// @return 有効な Vec2 の場合は true、無効な Vec2 の場合は false
		[[nodiscard]] bool IsValidMeasureConstraint(const Vec2& size) noexcept {
			return IsValidMeasureAxis(size.x) && IsValidMeasureAxis(size.y);
		}

		/// @brief Measure() の引数として有効な Vec2 に変換します。
		/// @param size 変換する Vec2
		/// @return 有効な Vec2 に変換された結果
		[[nodiscard]] Vec2 SanitizeMeasureConstraint(Vec2 size) noexcept {
			if (!IsValidMeasureAxis(size.x)) {
				size.x = 0.0f;
			}

			if (!IsValidMeasureAxis(size.y)) {
				size.y = 0.0f;
			}

			return size;
		}

		/// @brief 有効な Vec2 かどうかを判定します。
		/// @param size 判定する Vec2
		/// @return 有効な Vec2 の場合は true、無効な Vec2 の場合は false
		[[nodiscard]] bool IsValidDesiredSize(const Vec2& size) noexcept {
			return size.IsValid() && size.x >= 0.0f && size.y >= 0.0f;
		}

		/// @brief 有効な Vec2 に変換します。
		/// @param size 変換する Vec2
		/// @return 有効な Vec2 に変換された結果
		[[nodiscard]] Vec2 SanitizeDesiredSize(Vec2 size) noexcept {
			if (!std::isfinite(size.x) || size.x < 0.0f) {
				size.x = 0.0f;
			}

			if (!std::isfinite(size.y) || size.y < 0.0f) {
				size.y = 0.0f;
			}

			return size;
		}

		[[nodiscard]] UiRect SanitizeArrangeRect(UiRect rect) noexcept {
			constexpr int vec2ElementCount = 2;

			for (int i = 0; i < vec2ElementCount; ++i) {
				if (!std::isfinite(rect.pos[i])) {
					rect.pos[i] = 0.0f;
				}

				if (!std::isfinite(rect.size[i]) || rect.size[i] < 0.0f) {
					rect.size[i] = 0.0f;
				}
			}

			return rect;
		}
	}

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

	Vec2 UiElement::Measure(const Vec2& availableSize) {
		// 有効な値かどうかを判定する
		UASSERT(
			IsValidMeasureConstraint(availableSize) &&
			"UI measure constraint must be non-negative and not NaN."
		);

		// 有効な Vec2 に変換する
		const Vec2 sanitizedAvailableSize = SanitizeMeasureConstraint(
			availableSize
		);

		// 前回と同じかどうかを判定する
		const bool constraintChanged =
			!mHasMeasureResult ||
			!AreUiVectorsExactlyEqual(
				mLastMeasureAvailableSize,
				sanitizedAvailableSize
			);

		// 前回と同じで、MEASURE フラグが設定されていない場合は、前回の希望サイズを返す
		if (!constraintChanged && !IsInvalidated(UI_INVALIDATION::MEASURE)) {
			return mDesiredSize;
		}

		UASSERT(
			!mIsMeasuring &&
			"Recursive Measure() call detected on the same UI element."
		);

		// Measure() の実行中の場合は、前回の希望サイズを返す
		if (mIsMeasuring) {
			return mDesiredSize;
		}

		mIsMeasuring = true;

		// 利用可能サイズを保存する
		mLastMeasureAvailableSize = sanitizedAvailableSize;

		// OnMeasure() の呼び出し中に再度Measureが無効化された場合、その状態を消さないよう呼び出し前に解除する。
		ClearInvalidation(UI_INVALIDATION::MEASURE);

		Vec2 desiredSize;

		// 表示状態が COLLAPSED の場合は、希望サイズを (0, 0) にする
		if (mVisibility == UI_VISIBILITY::COLLAPSED) {
			desiredSize = Vec2::zero;
		} else {
			desiredSize = OnMeasure(sanitizedAvailableSize);

			UASSERT(
				IsValidDesiredSize(desiredSize) &&
				"UI desired size must be finite and non-negative."
			);

			desiredSize = SanitizeDesiredSize(desiredSize);
		}

		mDesiredSize      = desiredSize;
		mHasMeasureResult = true;
		mIsMeasuring      = false;

		return mDesiredSize;
	}

	const Vec2& UiElement::GetDesiredSize() const noexcept {
		return mDesiredSize;
	}

	void UiElement::Arrange(const UiRect& finalRect) {
		// 有効な矩形かどうかを判定する
		UASSERT(
			finalRect.IsValid() &&
			"UI arrange rectangle must be finite and have a non-negative size."
		);

		// 有効な矩形に変換する
		const UiRect sanitizedFinalRect = SanitizeArrangeRect(finalRect);

		// 前回と同じかどうかを判定する
		const bool arrangeConstraintChanged =
			!mHasArrangeResult ||
			!AreUiRectsExactlyEqual(
				mLastArrangeRect,
				sanitizedFinalRect
			);

		// 前回と同じで、ARRANGE フラグが設定されていない場合は、何もしない
		if (
			!arrangeConstraintChanged &&
			!IsInvalidated(UI_INVALIDATION::ARRANGE)
		) {
			return;
		}

		// 再帰的な Arrange() 呼び出しを検出する
		UASSERT(
			!mIsArranging &&
			"Recursive Arrange() call detected on the same UI element."
		);

		// Arrange() の実行中の場合は、何もしない
		if (mIsArranging) {
			return;
		}

		// Arrange() の実行中であることを示すフラグを設定する
		mIsArranging = true;

		const bool hadArrangeResult = mHasArrangeResult;

		mLastArrangeRect = sanitizedFinalRect;

		// Arrange() の呼び出し中に再度 Arrange が無効化された場合、その状態を消さないよう呼び出し前に解除する。
		ClearInvalidation(UI_INVALIDATION::ARRANGE);

		UiRect arrangeRect = sanitizedFinalRect;

		// 表示状態が COLLAPSED の場合は、サイズを (0, 0) にする
		if (mVisibility == UI_VISIBILITY::COLLAPSED) {
			arrangeRect.size = Vec2::zero;
		}

		// 前回の配置矩形と異なる場合は、無効化状態を更新する
		const bool layoutRectChanged =
			!hadArrangeResult || !AreUiRectsExactlyEqual(
				mLayoutRect,
				arrangeRect
			);

		mLayoutRect       = arrangeRect;
		mHasArrangeResult = true;

		// 配置矩形が変更された場合は、無効化状態を更新する
		if (layoutRectChanged) {
			Invalidate(
				UI_INVALIDATION::TRANSFORM | UI_INVALIDATION::PAINT |
				UI_INVALIDATION::HIT_TEST
			);
		}

		// 表示状態が COLLAPSED でない場合は、OnArrange() を呼び出す
		if (mVisibility != UI_VISIBILITY::COLLAPSED) {
			OnArrange(mLayoutRect);
		}

		// Arrange() の実行中であることを示すフラグを解除する
		mIsArranging = false;
	}

	const UiRect& UiElement::GetLayoutRect() const noexcept {
		return mLayoutRect;
	}

	void UiElement::ClearInvalidation(
		const UI_INVALIDATION invalidation
	) noexcept {
		mInvalidation &= ~invalidation;
	}

	Vec2 UiElement::OnMeasure(const Vec2& availableSize) {
		static_cast<void>(availableSize);
		return Vec2::zero;
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
