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

	void UiPaintContext::DrawSolidBorder(
		const UiRoundedRect& roundedRect, const float thickness,
		const UiColor& color
	) const {
		mCommandList.AddSolidBorder(roundedRect, thickness, color);
	}
}
