#include "pch.h"
#include "UiPanel.h"

#include "engine/unnamed/ui/paint/UiPaintContext.h"

namespace Unnamed {
	namespace {
		[[nodiscard]] bool AreUiColorsExactlyEqual(
			const UiColor& first, const UiColor& second
		) noexcept {
			return Math::ExactlyEqual(first.mRed, second.mRed) &&
					Math::ExactlyEqual(first.mGreen, second.mGreen) &&
					Math::ExactlyEqual(first.mBlue, second.mBlue) &&
					Math::ExactlyEqual(first.mAlpha, second.mAlpha);
		}

		[[nodiscard]] bool AreCornerRadiiExactlyEqual(
			const UiCornerRadii& first, const UiCornerRadii& second
		) noexcept {
			return Math::ExactlyEqual(first.topLeft, second.topLeft) &&
					Math::ExactlyEqual(first.topRight, second.topRight) &&
					Math::ExactlyEqual(first.bottomRight, second.bottomRight) &&
					Math::ExactlyEqual(first.bottomLeft, second.bottomLeft);
		}

		[[nodiscard]] bool ArePanelStylesExactlyEqual(
			const UiPanelStyle& first, const UiPanelStyle& second
		) noexcept {
			return AreUiColorsExactlyEqual(
						first.backgroundColor,
						second.backgroundColor
					) &&
					AreUiColorsExactlyEqual(
						first.borderColor,
						second.borderColor
					) &&
					AreCornerRadiiExactlyEqual(
						first.cornerRadii,
						second.cornerRadii
					) &&
					Math::ExactlyEqual(
						first.borderThickness,
						second.borderThickness
					);
		}

		[[nodiscard]] bool HasRoundedCorners(
			const UiCornerRadii& cornerRadii
		) noexcept {
			return cornerRadii.topLeft > 0.0f ||
					cornerRadii.topRight > 0.0f ||
					cornerRadii.bottomRight > 0.0f ||
					cornerRadii.bottomLeft > 0.0f;
		}
	}

	void UiPanel::SetStyle(const UiPanelStyle& style) {
		UASSERT(style.IsValid() && "UiPanel style must contain valid values.");

		const UiPanelStyle sanitizedStyle = style.GetSanitized();

		if (ArePanelStylesExactlyEqual(mStyle, sanitizedStyle)) {
			return;
		}

		mStyle = sanitizedStyle;

		Invalidate(UI_INVALIDATION::PAINT);
	}

	const UiPanelStyle& UiPanel::GetStyle() const noexcept {
		return mStyle;
	}

	void UiPanel::SetBackgroundColor(const UiColor& color) {
		UASSERT(color.IsValid() && "UiPanel background color must be valid.");

		const UiColor sanitizedColor = color.GetSanitized();

		if (AreUiColorsExactlyEqual(mStyle.backgroundColor, sanitizedColor)) {
			return;
		}

		mStyle.backgroundColor = sanitizedColor;

		Invalidate(UI_INVALIDATION::PAINT);
	}


	const UiColor& UiPanel::GetBackgroundColor() const noexcept {
		return mStyle.backgroundColor;
	}

	void UiPanel::SetBorderColor(const UiColor& color) {
		UASSERT(color.IsValid() && "UiPanel border color must be valid.");

		const UiColor sanitizedColor = color.GetSanitized();

		if (AreUiColorsExactlyEqual(mStyle.borderColor, sanitizedColor)) {
			return;
		}

		mStyle.borderColor = sanitizedColor;

		Invalidate(UI_INVALIDATION::PAINT);
	}

	const UiColor& UiPanel::GetBorderColor() const noexcept {
		return mStyle.borderColor;
	}

	void UiPanel::SetBorderThickness(float thickness) {
		UASSERT(
			std::isfinite(thickness) && thickness >= 0.0f &&
			"UiPanel border thickness must be finite and non-negative."
		);

		const float sanitizedThickness = std::max(thickness, 0.0f);

		if (Math::ExactlyEqual(mStyle.borderThickness, sanitizedThickness)) {
			return;
		}

		mStyle.borderThickness = sanitizedThickness;

		Invalidate(UI_INVALIDATION::PAINT);
	}

	float UiPanel::GetBorderThickness() const noexcept {
		return mStyle.borderThickness;
	}

	void UiPanel::SetCornerRadii(const UiCornerRadii& cornerRadii) {
		const UiPanelStyle sanitizedStyle = UiPanelStyle{
			.backgroundColor = mStyle.backgroundColor,
			.borderColor     = mStyle.borderColor,
			.cornerRadii     = cornerRadii,
			.borderThickness = mStyle.borderThickness
		}.GetSanitized();

		UASSERT(
			cornerRadii.IsFinite() && cornerRadii.topLeft >= 0.0f &&
			cornerRadii.topRight >= 0.0f &&
			cornerRadii.bottomRight >= 0.0f &&
			cornerRadii.bottomLeft >= 0.0f &&
			"UiPanel corner radii must be finite and non-negative."
		);

		if (AreCornerRadiiExactlyEqual(
			mStyle.cornerRadii,
			sanitizedStyle.cornerRadii
		)) {
			return;
		}

		mStyle.cornerRadii = sanitizedStyle.cornerRadii;

		Invalidate(UI_INVALIDATION::PAINT);
	}

	const UiCornerRadii& UiPanel::GetCornerRadii() const noexcept {
		return mStyle.cornerRadii;
	}

	void UiPanel::OnPaint(UiPaintContext& context) {
		const UiRoundedRect roundedRect{
			.rect  = GetWorldRect(),
			.radii = mStyle.cornerRadii
		};

		if (mStyle.backgroundColor.mAlpha > 0.0f) {
			if (HasRoundedCorners(mStyle.cornerRadii)) {
				context.DrawSolidRoundedRect(
					roundedRect,
					mStyle.backgroundColor
				);
			} else {
				context.DrawSolidRect(GetWorldRect(), mStyle.backgroundColor);
			}
		}

		if (mStyle.borderThickness > 0.0f && mStyle.borderColor.mAlpha > 0.0f) {
			context.DrawSolidBorder(
				roundedRect,
				mStyle.borderThickness,
				mStyle.borderColor
			);
		}
	}
}
