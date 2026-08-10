#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>

#include "InputCodes.h"

namespace Unnamed {
	enum class InputDeviceType {
		KEYBOARD, MOUSE, GAMEPAD
	};

	/// @brief 入力デバイス種別とデバイス内キーコードの組を識別します
	struct InputKey {
		InputDeviceType device;
		uint32_t        code;

		[[nodiscard]] static constexpr InputKey Mouse(
			const MOUSE_INPUT_CODE code
		) noexcept {
			return {
				.device = InputDeviceType::MOUSE,
				.code   = std::to_underlying(code)
			};
		}

		[[nodiscard]] static constexpr InputKey Gamepad(
			const GAMEPAD_INPUT_CODE code
		) noexcept {
			return {
				.device = InputDeviceType::GAMEPAD,
				.code   = std::to_underlying(code)
			};
		}

		[[nodiscard]] constexpr bool Is(
			const MOUSE_INPUT_CODE other
		) const noexcept {
			return device == InputDeviceType::MOUSE &&
					code == std::to_underlying(other);
		}

		[[nodiscard]] constexpr bool Is(
			const GAMEPAD_INPUT_CODE other
		) const noexcept {
			return device == InputDeviceType::GAMEPAD &&
					code == std::to_underlying(other);
		}

		bool operator==(const InputKey& other) const = default;
	};
}

/// @brief InputKeyのデバイス種別とコードを連想コンテナ用ハッシュへ変換します
template <>
struct std::hash<Unnamed::InputKey> {
	size_t operator()(const Unnamed::InputKey& key) const noexcept {
		return
				hash<uint32_t>()(static_cast<uint32_t>(key.device)) ^
				hash<uint32_t>()(key.code << 1);
	}
};
