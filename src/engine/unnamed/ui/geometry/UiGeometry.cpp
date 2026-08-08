#include "pch.h"
#include "UiGeometry.h"

namespace Unnamed {
	Vec2 UiRect::GetMin() const noexcept {
		return pos;
	}

	Vec2 UiRect::GetMax() const noexcept {
		return pos + size;
	}

	Vec2 UiRect::GetCenter() const noexcept {
		return pos + size * 0.5f;
	}

	Vec2 UiRect::GetHalfSize() const noexcept {
		return size * 0.5f;
	}

	float UiRect::GetLeft() const noexcept {
		return pos.x;
	}

	float UiRect::GetTop() const noexcept {
		return pos.y;
	}

	float UiRect::GetRight() const noexcept {
		return pos.x + size.x;
	}

	float UiRect::GetBottom() const noexcept {
		return pos.y + size.y;
	}

	float UiRect::GetWidth() const noexcept {
		return size.x;
	}

	float UiRect::GetHeight() const noexcept {
		return size.y;
	}

	bool UiRect::IsValid() const noexcept {
		return pos.IsValid() &&
				size.IsValid() &&
				size.x >= 0.0f &&
				size.y >= 0.0f;
	}

	bool UiRect::IsEmpty() const noexcept {
		return size.x <= 0.0f || size.y <= 0.0f;
	}

	bool UiRect::Contains(const Vec2& point) const noexcept {
		if (!IsValid() || IsEmpty()) {
			return false;
		}

		return point.x >= GetLeft() &&
				point.y >= GetTop() &&
				point.x < GetRight() &&
				point.y < GetBottom();
	}

	UiRect UiRect::FromMinMax(const Vec2& min, const Vec2& max) noexcept {
		return UiRect{
			.pos  = min,
			.size = max - min
		};
	}

	UiThickness UiThickness::Uniform(const float value) noexcept {
		return UiThickness{
			.left   = value,
			.top    = value,
			.right  = value,
			.bottom = value
		};
	}

	UiThickness UiThickness::Symmetric(
		const float horizontal, const float vertical
	) noexcept {
		return UiThickness{
			.left   = horizontal,
			.top    = vertical,
			.right  = horizontal,
			.bottom = vertical
		};
	}

	float UiThickness::GetHorizontal() const noexcept {
		return left + right;
	}

	float UiThickness::GetVertical() const noexcept {
		return top + bottom;
	}

	bool UiThickness::IsFinite() const noexcept {
		return std::isfinite(left) &&
				std::isfinite(top) &&
				std::isfinite(right) &&
				std::isfinite(bottom);
	}

	UiCornerRadii UiCornerRadii::Uniform(const float radius) noexcept {
		return UiCornerRadii{
			.topLeft     = radius,
			.topRight    = radius,
			.bottomRight = radius,
			.bottomLeft  = radius
		};
	}

	UiCornerRadii UiCornerRadii::Capsule(const Vec2& size) noexcept {
		const float radius = std::max(
			std::min(size.x, size.y) * 0.5f,
			0.0f
		);

		return Uniform(radius);
	}

	UiCornerRadii UiCornerRadii::GetNormalized(
		const Vec2& size
	) const noexcept {
		UiCornerRadii result{
			.topLeft     = std::max(topLeft, 0.0f),
			.topRight    = std::max(topRight, 0.0f),
			.bottomRight = std::max(bottomRight, 0.0f),
			.bottomLeft  = std::max(bottomLeft, 0.0f)
		};

		const float width  = std::max(size.x, 0.0f);
		const float height = std::max(size.y, 0.0f);

		float scale = 1.0f;

		const auto ApplyLimit = [&scale](
			const float available, const float required
		) {
			if (required <= 0.0f) {
				return;
			}

			scale = std::min(scale, available / required);
		};

		// 上辺
		ApplyLimit(width, result.topLeft + result.topRight);

		// 下辺
		ApplyLimit(width, result.bottomLeft + result.bottomRight);

		// 左辺
		ApplyLimit(height, result.topLeft + result.bottomLeft);

		// 右辺
		ApplyLimit(height, result.topRight + result.bottomRight);

		scale = std::clamp(scale, 0.0f, 1.0f);

		result.topLeft     *= scale;
		result.topRight    *= scale;
		result.bottomRight *= scale;
		result.bottomLeft  *= scale;

		return result;
	}

	bool UiCornerRadii::IsFinite() const noexcept {
		return std::isfinite(topLeft) &&
				std::isfinite(topRight) &&
				std::isfinite(bottomRight) &&
				std::isfinite(bottomLeft);
	}

	UiRoundedRect UiRoundedRect::GetNormalized() const noexcept {
		return UiRoundedRect{
			.rect  = rect,
			.radii = radii.GetNormalized(rect.size)
		};
	}

	bool UiRoundedRect::IsValid() const noexcept {
		if (!rect.IsValid()) {
			return false;
		}

		if (!radii.IsFinite()) {
			return false;
		}

		return radii.topLeft >= 0.0f &&
				radii.topRight >= 0.0f &&
				radii.bottomRight >= 0.0f &&
				radii.bottomLeft >= 0.0f;
	}

	bool UiRoundedRect::Contains(const Vec2& point) const noexcept {
		const UiRoundedRect normalized = GetNormalized();

		if (!normalized.rect.Contains(point)) {
			return false;
		}

		const float localX = point.x - normalized.rect.pos.x;

		const float localY = point.y - normalized.rect.pos.y;

		const float width  = normalized.rect.size.x;
		const float height = normalized.rect.size.y;

		const auto IsInsideCircle = [](
			const float pointX,
			const float pointY, const float centerX,
			const float centerY,
			const float radius
		) noexcept {
			const float offsetX = pointX - centerX;
			const float offsetY = pointY - centerY;

			return offsetX * offsetX + offsetY * offsetY <= radius * radius;
		};

		const float topLeftRadius = normalized.radii.topLeft;

		if (
			topLeftRadius > 0.0f &&
			localX < topLeftRadius &&
			localY < topLeftRadius
		) {
			return IsInsideCircle(
				localX,
				localY,
				topLeftRadius,
				topLeftRadius,
				topLeftRadius
			);
		}

		const float topRightRadius = normalized.radii.topRight;

		if (
			topRightRadius > 0.0f &&
			localX >= width - topRightRadius &&
			localY < topRightRadius
		) {
			return IsInsideCircle(
				localX,
				localY,
				width - topRightRadius,
				topRightRadius,
				topRightRadius
			);
		}

		const float bottomRightRadius = normalized.radii.bottomRight;

		if (
			bottomRightRadius > 0.0f &&
			localX >= width - bottomRightRadius &&
			localY >= height - bottomRightRadius
		) {
			return IsInsideCircle(
				localX,
				localY,
				width - bottomRightRadius,
				height - bottomRightRadius,
				bottomRightRadius
			);
		}

		const float bottomLeftRadius = normalized.radii.bottomLeft;

		if (
			bottomLeftRadius > 0.0f &&
			localX < bottomLeftRadius &&
			localY >= height - bottomLeftRadius
		) {
			return IsInsideCircle(
				localX,
				localY,
				bottomLeftRadius,
				height - bottomLeftRadius,
				bottomLeftRadius
			);
		}

		return true;
	}
}
