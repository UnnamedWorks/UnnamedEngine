#pragma once

#include <cstdint>
#include <string>
#include <core/math/Vec2.h>
#include <engine/unnamed/subsystem/input/InputKey.h>

namespace Unnamed {
	/// @brief 入力アクション状態構造体
	struct InputActionState {
		bool bIsPressed  = false;
		bool bIsHeld     = false;
		bool bIsReleased = false;
	};

	/// @brief 1D入力軸状態構造体
	struct InputAxisState1D {
		float value = 0.0f;
	};

	/// @brief 2D入力軸状態構造体
	enum class INPUT_AXIS : uint8_t {
		X, Y
	};

	/// @brief 2D入力軸状態構造体
	struct InputAxisState2D {
		Vec2 value = Vec2::zero;
	};

	/// @brief 入力バインディングタイプ列挙型
	enum class BINDING_TYPE : uint8_t {
		ACTION, AXIS_1D, AXIS_2D
	};

	/// @brief 入力バインディング構造体
	struct InputBinding {
		InputKey     key;
		BINDING_TYPE type = BINDING_TYPE::ACTION;
		std::string  target;

		float      scale   = 1.0f;
		Vec2       scale2D = Vec2::one;
		INPUT_AXIS axis    = INPUT_AXIS::X;
	};
}
