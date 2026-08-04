#pragma once
#include "engine/unnamed/ui/geometry/UiGeometry.h"
#include "engine/unnamed/ui/paint/UiColor.h"

namespace Unnamed {
	/// @brief UiPanelの視覚スタイルです。
	struct UiPanelStyle final {
		UiColor backgroundColor = UiColor::Transparent();

		UiColor borderColor = UiColor::Transparent();

		UiCornerRadii cornerRadii = {};

		float borderThickness = 0.0f;

		/// @brief スタイルの値がすべて有効か判定します。
		[[nodiscard]] bool IsValid() const noexcept {
			return
					backgroundColor.IsValid() &&
					borderColor.IsValid() &&
					cornerRadii.IsFinite() &&
					cornerRadii.topLeft >= 0.0f &&
					cornerRadii.topRight >= 0.0f &&
					cornerRadii.bottomRight >= 0.0f &&
					cornerRadii.bottomLeft >= 0.0f &&
					std::isfinite(borderThickness) &&
					borderThickness >= 0.0f;
		}

		/// @brief 不正な値を安全な範囲へ修正します。
		[[nodiscard]] UiPanelStyle GetSanitized() const noexcept {
			const auto SanitizeRadius = [](float radius) noexcept {
				if (!std::isfinite(radius)) {
					return 0.0f;
				}

				return std::max(radius, 0.0f);
			};

			float sanitizedBorderThickness = borderThickness;

			if (!std::isfinite(sanitizedBorderThickness) ||
				sanitizedBorderThickness < 0.0f) {
				sanitizedBorderThickness = 0.0f;
			}

			return UiPanelStyle{
				.backgroundColor = backgroundColor.GetSanitized(),
				.borderColor     = borderColor.GetSanitized(),
				.cornerRadii     = UiCornerRadii{
					.topLeft     = SanitizeRadius(cornerRadii.topLeft),
					.topRight    = SanitizeRadius(cornerRadii.topRight),
					.bottomRight = SanitizeRadius(cornerRadii.bottomRight),
					.bottomLeft  = SanitizeRadius(cornerRadii.bottomLeft),
				},
				.borderThickness = sanitizedBorderThickness
			};
		}
	};
}
