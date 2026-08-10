#include "MouseDevice.h"

namespace Unnamed {
	namespace {
		bool IsRemoteSession() {
			return GetSystemMetrics(SM_REMOTESESSION) != 0;
		}
	}

	/// @brief コンストラクタ
	/// @param hWnd ウィンドウハンドル
	MouseDevice::MouseDevice(const HWND hWnd) {
		mWindowHandle = hWnd;
		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01;
		rid.usUsage     = 0x02; // マウス
		// ウィンドウ外のマウス移動を取得する
		rid.dwFlags     = 0;    // RIDEV_INPUTSINK;
		rid.hwndTarget  = hWnd;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));
	}

	/// @brief 更新処理
	void MouseDevice::Update() {
	}

	/// @brief 生の入力を処理する
	/// @param raw 生の入力データ
	void MouseDevice::HandleRawInput(const RAWINPUT& raw) {
		if (raw.header.dwType != RIM_TYPEMOUSE) {
			return;
		}
		const auto& mouseData = raw.data.mouse;

		// 第5ボタンまでの状態を取得 マウスの形をしたキーボードは知らん
		if (mouseData.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
			mButtonStates[MOUSE_INPUT_CODE::LEFT_BUTTON] = true;
		}
		if (mouseData.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
			mButtonStates[MOUSE_INPUT_CODE::LEFT_BUTTON] = false;
		}
		if (mouseData.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
			mButtonStates[MOUSE_INPUT_CODE::RIGHT_BUTTON] = true;
		}
		if (mouseData.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
			mButtonStates[MOUSE_INPUT_CODE::RIGHT_BUTTON] = false;
		}
		if (mouseData.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
			mButtonStates[MOUSE_INPUT_CODE::MIDDLE_BUTTON] = true;
		}
		if (mouseData.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
			mButtonStates[MOUSE_INPUT_CODE::MIDDLE_BUTTON] = false;
		}
		if (mouseData.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) {
			mButtonStates[MOUSE_INPUT_CODE::X1_BUTTON] = true;
		}
		if (mouseData.usButtonFlags & RI_MOUSE_BUTTON_4_UP) {
			mButtonStates[MOUSE_INPUT_CODE::X1_BUTTON] = false;
		}
		if (mouseData.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) {
			mButtonStates[MOUSE_INPUT_CODE::X2_BUTTON] = true;
		}
		if (mouseData.usButtonFlags & RI_MOUSE_BUTTON_5_UP) {
			mButtonStates[MOUSE_INPUT_CODE::X2_BUTTON] = false;
		}

		// リモートセッションではマウス移動が正しく取得できないため、移動量の更新をスキップする
		if (!IsRemoteSession()) {
			// 移動量
			mDeltaX += mouseData.lLastX;
			mDeltaY += mouseData.lLastY;
		} else {
			POINT current;

			GetCursorPos(&current);

			const auto deltaX = current.x - mPrevCursor.x;
			const auto deltaY = current.y - mPrevCursor.y;

			mDeltaX += deltaX;
			mDeltaY += deltaY;

			mPrevCursor = current;
		}

		// ホイール
		if (mouseData.usButtonFlags & RI_MOUSE_WHEEL) {
			const SHORT wheelData = static_cast<SHORT>(mouseData.usButtonData);
			mWheelRemainder       += wheelData;
			if (wheelData > 0) {
				mButtonStates[MOUSE_INPUT_CODE::WHEEL_UP]   = true;
				mButtonStates[MOUSE_INPUT_CODE::WHEEL_DOWN] = false;
			} else if (wheelData < 0) {
				mButtonStates[MOUSE_INPUT_CODE::WHEEL_DOWN] = true;
				mButtonStates[MOUSE_INPUT_CODE::WHEEL_UP] = false;
			}
			const int clicks = mWheelRemainder / WHEEL_DELTA;
			if (clicks != 0) {
				mWheelDelta     += clicks;
				mWheelRemainder -= clicks * WHEEL_DELTA;
			}
		}
	}

	/// @brief キーの状態を取得する
	/// @param key 入力キー
	/// @return 押されているかどうか
	bool MouseDevice::GetKeyState(const InputKey& key) const {
		if (key.device != InputDeviceType::MOUSE) {
			return false;
		}
		const auto code = static_cast<MOUSE_INPUT_CODE>(key.code);
		const auto it   = mButtonStates.find(code);
		return it != mButtonStates.end() ? it->second : false;
	}

	/// @brief アナログ値を取得する
	/// @param key 入力キー
	/// @return アナログ値
	float MouseDevice::GetAnalogValue(const InputKey& key) const {
		if (key.device != InputDeviceType::MOUSE) {
			return 0.0f;
		}

		auto code = static_cast<MOUSE_INPUT_CODE>(key.code);

		if (code == MOUSE_INPUT_CODE::AXIS_X) {
			return static_cast<float>(mDeltaX);
		}
		if (code == MOUSE_INPUT_CODE::AXIS_Y) {
			return static_cast<float>(mDeltaY);
		}
		if (code == MOUSE_INPUT_CODE::WHEEL) {
			return static_cast<float>(mWheelDelta);
		}
		return 0.0f;
	}

	/// @brief サポートされているキーのリストを取得する
	/// @return サポートされているキーのベクター
	std::vector<InputKey> MouseDevice::GetSupportedKeys() const {
		return {
			InputKey::Mouse(MOUSE_INPUT_CODE::LEFT_BUTTON),
			InputKey::Mouse(MOUSE_INPUT_CODE::RIGHT_BUTTON),
			InputKey::Mouse(MOUSE_INPUT_CODE::MIDDLE_BUTTON),
			InputKey::Mouse(MOUSE_INPUT_CODE::X1_BUTTON),
			InputKey::Mouse(MOUSE_INPUT_CODE::X2_BUTTON),
			InputKey::Mouse(MOUSE_INPUT_CODE::AXIS_X),
			InputKey::Mouse(MOUSE_INPUT_CODE::AXIS_Y),
			InputKey::Mouse(MOUSE_INPUT_CODE::WHEEL),
			InputKey::Mouse(MOUSE_INPUT_CODE::WHEEL_UP),
			InputKey::Mouse(MOUSE_INPUT_CODE::WHEEL_DOWN),
		};
	}

	/// @brief デバイスの種類を取得する
	/// @return デバイスの種類
	InputDeviceType MouseDevice::GetDeviceType() const {
		return InputDeviceType::MOUSE;
	}

	/// @brief 状態をリセットする
	void MouseDevice::ResetStates() {
		mButtonStates.clear();
		mDeltaX         = 0;
		mDeltaY         = 0;
		mWheelDelta     = 0;
		mWheelRemainder = 0;
	}

	/// @brief デルタ値をリセットする
	void MouseDevice::ResetDelta() {
		mDeltaX                                     = 0;
		mDeltaY                                     = 0;
		mWheelDelta                                 = 0;
		mButtonStates[MOUSE_INPUT_CODE::WHEEL_UP]   = false;
		mButtonStates[MOUSE_INPUT_CODE::WHEEL_DOWN] = false;
	}
}
