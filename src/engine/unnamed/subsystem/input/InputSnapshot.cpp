#include "pch.h"
#include "InputSnapshot.h"

namespace Unnamed {
	namespace {
		/// @brief 指定したキーの状態を取得します。
		/// @param states 入力状態のマップ
		/// @param key 取得するキー
		/// @return 指定したキーが押されているか?
		[[nodiscard]] bool FindKeyState(
			const std::unordered_map<InputKey, bool>& states,
			const InputKey&                           key
		) {
			const auto itr = states.find(key);

			return itr != states.end() && itr->second;
		}
	}


	bool InputSnapshot::IsKeyDown(const InputKey& key) const {
		return FindKeyState(mCurrentKeyStates, key);
	}

	bool InputSnapshot::WasKeyPressed(const InputKey& key) const {
		return FindKeyState(mCurrentKeyStates, key) &&
				!FindKeyState(mPreviousKeyStates, key);
	}

	bool InputSnapshot::WasKeyReleased(const InputKey& key) const {
		return !FindKeyState(mCurrentKeyStates, key) &&
				FindKeyState(mPreviousKeyStates, key);
	}

	float InputSnapshot::GetAnalogValue(const InputKey& key) const {
		const auto itr = mAnalogValues.find(key);

		if (itr == mAnalogValues.end()) {
			return 0.0f;
		}

		return itr->second;
	}

	Vec2 InputSnapshot::GetMouseClientPosition() const {
		return mMouseClientPosition;
	}

	Vec2 InputSnapshot::GetMouseClientViewportSize() const {
		return mMouseClientViewportSize;
	}

}
