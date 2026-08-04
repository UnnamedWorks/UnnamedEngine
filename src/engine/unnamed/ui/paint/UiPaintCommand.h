#pragma once
#include <variant>

#include "UiColor.h"

#include "../geometry/UiGeometry.h"

namespace Unnamed {
	/// @brief 単色矩形の描画コマンドです。
	struct UiSolidRectPaintCommand final {
		UiRect  mRect  = {};
		UiColor mColor = {};
	};

	/// @brief 単色角丸矩形の単色コマンドです。
	struct UiSolidRoundedRectPaintCommand final {
		UiRoundedRect mRoundedRect = {};
		UiColor       mColor       = {};
	};

	using UiPaintCommand = std::variant<
		UiSolidRectPaintCommand, UiSolidRoundedRectPaintCommand
	>;
}
