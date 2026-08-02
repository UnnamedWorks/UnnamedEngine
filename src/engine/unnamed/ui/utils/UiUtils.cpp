#include "pch.h"
#include "UiUtils.h"

#include "../geometry/UiGeometry.h"

namespace Unnamed {
	bool AreUiVectorsExactlyEqual(
		const Vec2& first, const Vec2& second
	) noexcept {
		return Math::ExactlyEqual(first.x, second.x) &&
		       Math::ExactlyEqual(first.y, second.y);
	}

	bool AreUiRectsExactlyEqual(
		const UiRect& first, const UiRect& second
	) noexcept {
		return AreUiVectorsExactlyEqual(first.pos, second.pos) &&
		       AreUiVectorsExactlyEqual(first.size, second.size);
	}
}
