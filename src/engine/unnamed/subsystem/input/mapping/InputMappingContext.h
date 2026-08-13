#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <engine/unnamed/subsystem/input/InputSnapshot.h>
#include <engine/unnamed/subsystem/input/mapping/InputMappingTypes.h>

namespace Unnamed {
	class InputMappingContext final {
	public:
		/// @brief フレーム開始時に一時的な入力状態をリセットします。
		void BeginFrame();

		/// @brief 現在の物理入力状態からAction・Axisを評価します。
		/// @param snapshot 現在フレームの入力スナップショット
		void Evaluate(const InputSnapshot& snapshot);

		/// @brief Actionへキーをバインドします
		/// @param action バインドするAction名
		/// @param key バインドするInputKey
		void BindAction(const std::string& action, const InputKey& key);

		/// @brief 1D Axisへキーをバインドします。
		/// @param axis バインドするAxis名
		/// @param key バインドするInputKey
		/// @param scale スケール値（デフォルトは1.0f)
		void BindAxis1D(
			const std::string& axis, const InputKey& key, float scale = 1.0f
		);

		/// @brief 2D Axisへキーをバインドします。
		/// @param axis バインドするAxis名
		/// @param key バインドするInputKey
		/// @param axisType バインドするAxisのタイプ（XまたはY)
		/// @param scale スケール値（デフォルトは1.0f)
		void BindAxis2D(
			const std::string& axis, const InputKey& key, INPUT_AXIS axisType,
			float              scale = 1.0f
		);

		/// @brief 指定したアクションまたは軸からすべてのバインドを解除します
		/// @param target バインドを解除するアクションまたは軸
		void Unbind(const std::string& target);

		/// @brief 外部からのアクション入力を処理します
		/// @param action アクション名
		/// @param pressed 押下状態（trueなら押下、falseなら解放）
		void HandleExternalAction(
			const std::string& action, const bool pressed
		);

		/// @brief 指定したアクションの状態を取得します
		/// @param action 取得するアクションの名前
		/// @return 押下状態
		[[nodiscard]] bool IsPressed(const std::string& action) const;

		/// @brief 指定したアクションの状態を取得します
		/// @param action 取得するアクションの名前
		/// @return 保持状態
		[[nodiscard]] bool IsHeld(const std::string& action) const;

		/// @brief 指定したアクションの状態を取得します
		/// @param action 取得するアクションの名前
		/// @return 解放状態
		[[nodiscard]] bool IsReleased(const std::string& action) const;

		/// @brief 指定した1D軸の値を取得します
		/// @param axis 取得する軸の名前
		/// @return 軸の値
		[[nodiscard]] float Axis1D(const std::string& axis) const;

		/// @brief 指定した2D軸の値を取得します
		/// @param axis 取得する軸の名前
		/// @return 軸の値
		[[nodiscard]] Vec2 Axis2D(const std::string& axis) const;

		/// @brief 入力状態をリセットします
		void ResetStates();

	private:
		std::unordered_map<std::string, InputActionState> mActionStates;
		std::unordered_map<std::string, InputAxisState1D> mAxisStates1D;
		std::unordered_map<std::string, InputAxisState2D> mAxisStates2D;

		std::unordered_set<std::string> mExternalHeldActions;

		std::vector<InputBinding> mBindings;
	};
}
