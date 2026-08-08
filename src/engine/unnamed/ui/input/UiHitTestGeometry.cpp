#include "pch.h"
#include "UiHitTestGeometry.h"

namespace Unnamed {
	UiHitTestGeometry UiHitTestGeometry::Empty() noexcept {
		return UiHitTestGeometry(std::monostate{});
	}

	UiHitTestGeometry UiHitTestGeometry::FromRect(const UiRect& rect) {
		UASSERT(rect.IsValid() && "Hit Test rectangle must be valid.");

		if (!rect.IsValid()) {
			return Empty();
		}

		return UiHitTestGeometry(rect);
	}

	UiHitTestGeometry UiHitTestGeometry::FromRoundedRect(
		const UiRoundedRect& roundedRect
	) {
		UASSERT(
			roundedRect.IsValid() && "Hit Test rounded rectangle must be valid."
		);

		if (!roundedRect.IsValid()) {
			return Empty();
		}

		return UiHitTestGeometry(roundedRect.GetNormalized());
	}

	bool UiHitTestGeometry::Contains(const Vec2& point) const noexcept {
		return std::visit(
			[&point](const auto& shape) -> bool {
				using ShapeType = std::remove_cvref_t<decltype(shape)>;

				if constexpr (std::same_as<ShapeType, std::monostate>) {
					return false;
				} else {
					return shape.Contains(point);
				}
			},
			mShape
		);
	}

	bool UiHitTestGeometry::IsEmpty() const noexcept {
		return std::visit(
			[](const auto& shape) -> bool {
				using ShapeType = std::remove_cvref_t<decltype(shape)>;

				if constexpr (
					std::same_as<ShapeType, std::monostate>
				) {
					return true;
				} else if constexpr (std::same_as<ShapeType, UiRect>) {
					return shape.IsEmpty();
				} else {
					return shape.rect.IsEmpty();
				}
			},
			mShape
		);
	}

	UiHitTestGeometry::UiHitTestGeometry(const Shape& shape) noexcept :
		mShape(shape) {
	}
}
