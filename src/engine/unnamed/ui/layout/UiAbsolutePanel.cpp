#include "pch.h"
#include "UiAbsolutePanel.h"

#include "../utils/UiUtils.h"

namespace Unnamed {
	namespace {
		[[nodiscard]] UiAbsoluteSlot SanitizeSlot(
			UiAbsoluteSlot slot
		) noexcept {
			constexpr uint32_t vec2ElementCount = 2;

			for (uint32_t i = 0; i < vec2ElementCount; ++i) {
				if (!std::isfinite(slot.rect.pos[i])) {
					slot.rect.pos[i] = 0.0f;
				}

				if (!std::isfinite(slot.rect.size[i]) || slot.rect.size[i] <
				    0.0f) {
					slot.rect.size[i] = 0.0f;
				}
			}

			return slot;
		}
	}

	bool UiAbsolutePanel::SetChildSlot(
		UiElement& child, const UiAbsoluteSlot& slot
	) {
		UASSERT(
			slot.IsValid() &&
			"Absolute layout slot must contain a valid rectangle."
		);

		const std::optional<size_t> index = FindChildIndex(child);

		if (!index.has_value()) {
			return false;
		}

		const UiAbsoluteSlot sanitizedSlot = SanitizeSlot(slot);

		UiAbsoluteSlot& currentSlot = mSlots[*index];

		if (AreUiRectsExactlyEqual(currentSlot.rect, sanitizedSlot.rect)) {
			return true;
		}

		currentSlot = sanitizedSlot;

		Invalidate(UI_INVALIDATION::MEASURE | UI_INVALIDATION::ARRANGE);

		return true;
	}

	const UiAbsoluteSlot* UiAbsolutePanel::GetChildSlot(
		const UiElement& child
	) const noexcept {
		const std::optional<size_t> index = FindChildIndex(child);

		if (!index.has_value()) {
			return nullptr;
		}

		return &mSlots[*index];
	}

	Vec2 UiAbsolutePanel::OnMeasure(const Vec2& availableSize) {
		static_cast<void>(availableSize);

		const auto children = GetChildren();

		UASSERT(children.size() == mSlots.size());

		Vec2 desiredSize;

		for (size_t index = 0; index < children.size(); ++index) {
			UiElement& child = *children[index];

			const UiAbsoluteSlot& slot = mSlots[index];

			static_cast<void>(child.Measure(slot.rect.size));

			if (child.GetVisibility() == UI_VISIBILITY::COLLAPSED) {
				continue;
			}

			const float right  = slot.rect.pos.x + slot.rect.size.x;
			const float bottom = slot.rect.pos.y + slot.rect.size.y;

			desiredSize.x = std::max(desiredSize.x, right);
			desiredSize.y = std::max(desiredSize.y, bottom);
		}

		desiredSize.x = std::max(desiredSize.x, 0.0f);
		desiredSize.y = std::max(desiredSize.y, 0.0f);

		return desiredSize;
	}

	void UiAbsolutePanel::OnArrange(const UiRect& finalRect) {
		static_cast<void>(finalRect);

		const auto children = GetChildren();

		UASSERT(children.size()==mSlots.size());

		for (size_t index = 0; index < children.size(); ++index) {
			UiElement& child = *children[index];

			const UiAbsoluteSlot& slot = mSlots[index];

			child.Arrange(slot.rect);
		}
	}

	void UiAbsolutePanel::OnChildAdded(
		UiElement& child, const size_t index
	) noexcept {
		static_cast<void>(child);

		UASSERT(index <= mSlots.size());

		mSlots.insert(
			mSlots.begin() + static_cast<ptrdiff_t>(index),
			UiAbsoluteSlot{}
		);
	}

	void UiAbsolutePanel::OnChildRemoving(
		UiElement& child, const size_t index
	) noexcept {
		static_cast<void>(child);

		UASSERT(index < mSlots.size());

		mSlots.erase(mSlots.begin() + static_cast<ptrdiff_t>(index));
	}

	std::optional<size_t> UiAbsolutePanel::FindChildIndex(
		const UiElement& child
	) const noexcept {
		const auto children = GetChildren();

		for (size_t index = 0; index < children.size(); ++index) {
			if (children[index].get() == &child) {
				return index;
			}
		}

		return std::nullopt;
	}
}
