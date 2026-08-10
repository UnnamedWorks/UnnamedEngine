#pragma once
#include <vector>

#include "engine/unnamed/subsystem/input/InputKey.h"

namespace Unnamed {
	/// @brief 入力デバイス基底クラス
	class BaseInputDevice {
	public:
		virtual      ~BaseInputDevice() = default;
		virtual void Update() = 0;

		[[nodiscard]]
		virtual bool GetKeyState(const InputKey& key) const = 0;
		[[nodiscard]]
		virtual float GetAnalogValue(const InputKey& key) const = 0;
		[[nodiscard]]
		virtual std::vector<InputKey> GetSupportedKeys() const = 0;
		[[nodiscard]]
		virtual InputDeviceType GetDeviceType() const = 0;

		virtual void ResetStates() = 0;
	};
}
