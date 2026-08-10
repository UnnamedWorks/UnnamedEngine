#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <unordered_map>
#include <Windows.h>

#include <engine/unnamed/subsystem/input/device/base/BaseInputDevice.h>

namespace Unnamed {
	/// @brief マウスデバイスクラス
	class MouseDevice final : public BaseInputDevice {
	public:
		explicit MouseDevice(HWND hWnd);
		~MouseDevice() override = default;

		void Update() override;

		void HandleRawInput(const RAWINPUT& raw);

		[[nodiscard]] bool GetKeyState(const InputKey& key) const override;
		[[nodiscard]] float GetAnalogValue(const InputKey& key) const override;
		[[nodiscard]] std::vector<InputKey> GetSupportedKeys() const override;
		[[nodiscard]] InputDeviceType GetDeviceType() const override;
		void ResetStates() override;

		[[nodiscard]] int GetDeltaX() const {
			return mDeltaX;
		}

		[[nodiscard]] int GetDeltaY() const {
			return mDeltaY;
		}

		[[nodiscard]] int GetWheelDelta() const {
			return mWheelDelta;
		}

		[[nodiscard]] HWND GetWindowHandle() const {
			return mWindowHandle;
		}

		void ResetDelta();

	private:
		HWND                                       mWindowHandle = nullptr;
		std::unordered_map<MOUSE_INPUT_CODE, bool> mButtonStates;

		POINT mPrevCursor; // RDPセッションでのカーソル位置を追跡するための変数

		int mDeltaX         = 0;
		int mDeltaY         = 0;
		int mWheelDelta     = 0;
		int mWheelRemainder = 0;
	};
}
