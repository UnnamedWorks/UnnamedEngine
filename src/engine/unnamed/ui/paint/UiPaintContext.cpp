#include "pch.h"
#include "UiPaintContext.h"

namespace Unnamed {
	void UiPaintContext::DrawSolidRect(
		const UiRect& rect, const UiColor& color
	) const {
		mCommandList.AddSolidRect(rect, color);
	}

	void UiPaintContext::DrawSolidRoundedRect(
		const UiRoundedRect& roundedRect, const UiColor& color
	) const {
		mCommandList.AddSolidRoundedRect(roundedRect, color);
	}
}
