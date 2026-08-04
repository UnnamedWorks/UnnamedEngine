#include "pch.h"
#include "UiPaintCommandList.h"

namespace Unnamed {
	namespace {
		[[nodiscard]] float SanitizeOpacityMultiplier(
			const float opacityMultiplier
		) noexcept {
			if (!std::isfinite(opacityMultiplier)) {
				return 0.0f;
			}

			return std::clamp(opacityMultiplier, 0.0f, 1.0f);
		}

		void ApplyOpacityMultiplier(
			UiPaintCommand& command,
			const float     opacityMultiplier
		) {
			std::visit(
				[opacityMultiplier](auto& typedCommand) {
					typedCommand.mColor =
							typedCommand.mColor.WithOpacityMultiplier(
								opacityMultiplier
							);
				},
				command
			);
		}

		[[nodiscard]] float GetCommandAlpha(const UiPaintCommand& command) {
			return std::visit(
				[](const auto& typedCommand) {
					return typedCommand.mColor.mAlpha;
				},
				command
			);
		}
	}

	void UiPaintCommandList::Clear() noexcept {
		mCommands.clear();
	}

	void UiPaintCommandList::Reserve(size_t commandCount) {
		mCommands.reserve(commandCount);
	}

	void UiPaintCommandList::AddSolidRect(
		const UiRect& rect, const UiColor& color
	) {
		UASSERT(rect.IsValid() && "UI paint rectangle must be valid.");

		UASSERT(
			color.IsValid() &&
			"UI paint color must use finite normalized components."
		);

		if (!rect.IsValid() || rect.IsEmpty()) {
			return;
		}

		const UiColor sanitizedColor = color.GetSanitized();

		if (sanitizedColor.mAlpha <= 0.0f) {
			return;
		}

		mCommands.emplace_back(
			UiSolidRectPaintCommand{
				.mRect = rect,
				.mColor = sanitizedColor
			}
		);
	}

	void UiPaintCommandList::AddSolidRoundedRect(
		const UiRoundedRect& roundedRect, const UiColor& color
	) {
		UASSERT(
			roundedRect.IsValid() && "UI rounded paint rectangle must be valid."
		);

		UASSERT(
			color.IsValid() &&
			"UI paint color must use finite normalized components."
		);

		if (!roundedRect.IsValid() || roundedRect.rect.IsEmpty()) {
			return;
		}

		const UiColor sanitizedColor = color.GetSanitized();

		if (sanitizedColor.mAlpha <= 0.0f) {
			return;
		}

		mCommands.emplace_back(
			UiSolidRoundedRectPaintCommand{
				.mRoundedRect = roundedRect.GetNormalized(),
				.mColor        = sanitizedColor
			}
		);
	}

	void UiPaintCommandList::AddSolidBorder(
		const UiRoundedRect& roundedRect, float thickness, const UiColor& color
	) {
		UASSERT(roundedRect.IsValid() && "UI border rectangle must be valid.");

		UASSERT(
			std::isfinite(thickness) && thickness >= 0.0f &&
			"UI border thickness must be finite and non-negative."
		);

		UASSERT(
			color.IsValid() &&
			"UI border color must use finite normalized components."
		);

		if (!roundedRect.IsValid() || roundedRect.rect.IsEmpty()) {
			return;
		}

		if (!std::isfinite(thickness) || thickness <= 0.0f) {
			return;
		}

		const UiColor sanitizedColor = color.GetSanitized();

		if (sanitizedColor.mAlpha <= 0.0f) {
			return;
		}

		const UiRoundedRect normalizedRoundedRect = roundedRect.GetNormalized();

		const float maximumThickness = std::min(
			normalizedRoundedRect.rect.size.x,
			normalizedRoundedRect.rect.size.y
		) * 0.5f;

		const float sanitizedThickness = std::clamp(
			thickness,
			0.0f,
			maximumThickness
		);

		if (sanitizedThickness <= 0.0f) {
			return;
		}

		mCommands.emplace_back(
			UiSolidBorderPaintCommand{
				.roundedRect = normalizedRoundedRect,
				.thickness   = sanitizedThickness,
				.mColor       = sanitizedColor
			}
		);
	}

	void UiPaintCommandList::Append(
		const UiPaintCommandList& source, const float opacityMultiplier
	) {
		UASSERT(
			this != &source && "A UI paint command list cannot append itself."
		);

		if (this == &source) {
			return;
		}

		const float sanitizedOpacity = SanitizeOpacityMultiplier(
			opacityMultiplier
		);

		if (sanitizedOpacity <= 0.0f) {
			return;
		}

		mCommands.reserve(mCommands.size() + source.mCommands.size());

		for (const UiPaintCommand& sourceCommand : source.mCommands) {
			UiPaintCommand command = sourceCommand;

			ApplyOpacityMultiplier(command, sanitizedOpacity);

			if (GetCommandAlpha(command) <= 0.0f) {
				continue;
			}

			mCommands.emplace_back(command);
		}
	}

	size_t UiPaintCommandList::GetCommandCount() const noexcept {
		return mCommands.size();
	}

	bool UiPaintCommandList::IsEmpty() const noexcept {
		return mCommands.empty();
	}

	std::span<const UiPaintCommand>
	UiPaintCommandList::GetCommands() const noexcept {
		return mCommands;
	}
}
