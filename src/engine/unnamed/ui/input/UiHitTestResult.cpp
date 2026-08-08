#include "pch.h"
#include "UiHitTestResult.h"

namespace Unnamed {
	bool UiHitTestResult::HasHit() const noexcept {
		return target != nullptr;
	}
}
