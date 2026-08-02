#pragma once
#include "../geometry/UiGeometry.h"

namespace Unnamed {
	struct UiAbsoluteSlot final {
		UiRect rect = {};

		[[nodiscard]] bool IsValid() const noexcept {
			return rect.IsValid();
		}
	};
}
