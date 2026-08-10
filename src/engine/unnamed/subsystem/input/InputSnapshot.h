#pragma once

#include <unordered_map>
#include <engine/unnamed/subsystem/input/InputKey.h>

namespace Unnamed {
	/// @brief 1フレーム分の物理入力状態を保持します。
	/// @details InputSystemによって更新され、InputSystem::Update完了後に次の入力更新まで読み取り専用のスナップショットとして使用されます。
	class InputSnapshot final {
	public:
		/// @brief 指定したキーが現在押されているか取得します。
		/// @param key 取得するキー
		/// @return 指定したキーが現在押されているか?
		[[nodiscard]] bool IsKeyDown(const InputKey& key) const;

		/// @brief 指定したキーがこのフレームで押されたかを取得します。
		/// @param key 取得するキー
		/// @return 指定したキーがこのフレームで押されたか?
		[[nodiscard]] bool WasKeyPressed(const InputKey& key) const;

		/// @brief 指定したキーがこのフレームで離されたか取得します。
		/// @param key 取得するキー
		/// @return 指定したキーがこのフレームで離されたか?
		[[nodiscard]] bool WasKeyReleased(const InputKey& key) const;

		/// @brief 指定した入力キーのアナログ値を取得します。
		/// @param key 取得するキー
		/// @return アナログ値
		[[nodiscard]] float GetAnalogValue(const InputKey& key) const;

		/// @brief マウスのクライアント座標を取得します。
		/// @return マウスのクライアント座標
		[[nodiscard]] Vec2 GetMouseClientPosition() const;

		/// @brief マウス入力に使用されるビューポートのサイズを取得します。
		/// @return マウス入力に使用されるビューポートのサイズ
		[[nodiscard]] Vec2 GetMouseClientViewportSize() const;

	private:
		friend class InputSystem;

		std::unordered_map<InputKey, bool>  mCurrentKeyStates;
		std::unordered_map<InputKey, bool>  mPreviousKeyStates;
		std::unordered_map<InputKey, float> mAnalogValues;

		Vec2 mMouseClientPosition     = Vec2::zero;
		Vec2 mMouseClientViewportSize = Vec2::zero;
	};
}
