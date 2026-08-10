#pragma once

namespace Unnamed {
	enum class MOUSE_INPUT_CODE : uint32_t {
		LEFT_BUTTON,
		RIGHT_BUTTON,
		MIDDLE_BUTTON,
		X1_BUTTON,
		X2_BUTTON,
		AXIS_X,
		AXIS_Y,
		WHEEL,
		WHEEL_UP,
		WHEEL_DOWN
	};

	enum class GAMEPAD_INPUT_CODE : uint32_t {
		A = 1000,
		B,
		X,
		Y,
		LB,
		RB,
		BACK,
		START,
		LS,
		RS,
		DPAD_UP,
		DPAD_DOWN,
		DPAD_LEFT,
		DPAD_RIGHT,
		LX,
		LY,
		RX,
		RY,
		LT,
		RT,
		LSTICK_LEFT,
		LSTICK_RIGHT,
		LSTICK_UP,
		LSTICK_DOWN,
		RSTICK_LEFT,
		RSTICK_RIGHT,
		RSTICK_UP,
		RSTICK_DOWN
	};
}
