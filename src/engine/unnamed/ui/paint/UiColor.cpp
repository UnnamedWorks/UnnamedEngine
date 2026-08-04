#include "pch.h"
#include "UiColor.h"

namespace Unnamed {
	bool UiColor::IsValid() const noexcept {
		return
				std::isfinite(mRed) &&
				std::isfinite(mGreen) &&
				std::isfinite(mBlue) &&
				std::isfinite(mAlpha) &&
				mRed >= 0.0f &&
				mRed <= 1.0f &&
				mGreen >= 0.0f &&
				mGreen <= 1.0f &&
				mBlue >= 0.0f &&
				mBlue <= 1.0f &&
				mAlpha >= 0.0f &&
				mAlpha <= 1.0f;
	}

	UiColor UiColor::GetSanitized() const noexcept {
		const auto SanitizeComponent = [](const float value) noexcept {
			if (!std::isfinite(value)) {
				return 0.0f;
			}

			return std::clamp(value, 0.0f, 1.0f);
		};

		return UiColor{
			.mRed   = SanitizeComponent(mRed),
			.mGreen = SanitizeComponent(mGreen),
			.mBlue  = SanitizeComponent(mBlue),
			.mAlpha = SanitizeComponent(mAlpha)
		};
	}

	UiColor UiColor::WithOpacityMultiplier(
		float opacityMultiplier
	) const noexcept {
		if (!std::isfinite(opacityMultiplier)) {
			opacityMultiplier = 0.0f;
		}

		opacityMultiplier = std::clamp(opacityMultiplier, 0.0f, 1.0f);

		UiColor result = GetSanitized();

		result.mAlpha *= opacityMultiplier;

		return result;
	}
}
