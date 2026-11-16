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
		/// @param consumedKeys 自分より上位のコンテキストによって使用されたキーのセット
		void Evaluate(
			const InputSnapshot&          snapshot,
			std::unordered_set<InputKey>& consumedKeys
		);

		/// @brief Actionへキーをバインドします
		/// @param action バインドするAction名
		/// @param key バインドするInputKey
		/// @param consumeInput このバインディングが使用した入力を下のコンテキストへ渡さないかどうか
		void BindAction(
			const std::string& action,
			const InputKey&    key,
			bool               consumeInput = false
		);

		/// @brief 1D Axisへキーをバインドします。
		/// @param axis バインドするAxis名
		/// @param key バインドするInputKey
		/// @param scale スケール値（デフォルトは1.0f)
		/// @param consumeInput このバインディングが使用した入力を下のコンテキストへ渡さないかどうか
		void BindAxis1D(
			const std::string& axis,
			const InputKey&    key,
			float              scale        = 1.0f,
			bool               consumeInput = false
		);

		/// @brief 2D Axisへキーをバインドします。
		/// @param axis バインドするAxis名
		/// @param key バインドするInputKey
		/// @param axisType バインドするAxisのタイプ（XまたはY)
		/// @param scale スケール値（デフォルトは1.0f)
		/// @param consumeInput このバインディングが使用した入力を下のコンテキストへ渡さないかどうか
		void BindAxis2D(
			const std::string& axis, const InputKey& key, INPUT_AXIS axisType,
			float              scale        = 1.0f,
			bool               consumeInput = false
		);

		/// @brief 指定したアクションまたは軸からすべてのバインドを解除します
		/// @param target バインドを解除するアクションまたは軸
		void Unbind(const std::string& target);

		/// @brief 外部からのアクション入力を処理します
		/// @param action アクション名
		/// @param pressed 押下状態（trueなら押下、falseなら解放）
		/// @param sourceId 入力ソースの識別子（0は直接実行用）
		void HandleExternalAction(
			const std::string& action, bool pressed, uint64_t sourceId = 0
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
		/// @param preserveExternalSources 同期Release用のコマンドソースを保持するか
		void ResetStates(bool preserveExternalSources = false);

	private:
		std::unordered_map<std::string, InputActionState> mActionStates;
		std::unordered_map<std::string, InputAxisState1D> mAxisStates1D;
		std::unordered_map<std::string, InputAxisState2D> mAxisStates2D;

		// 0は直接Console入力、その他は押下コマンドの有効期間を識別
		std::unordered_map<std::string, std::vector<uint64_t>> mExternalHeldActions;

		std::vector<InputBinding> mBindings;
	};
}
