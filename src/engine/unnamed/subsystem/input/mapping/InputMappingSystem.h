#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <engine/unnamed/subsystem/input/mapping/InputMappingContext.h>

namespace Unnamed {
	class InputMappingSystem final {
	public:
		InputMappingSystem();

		/// @brief フレーム開始時に一時的な入力状態をリセットします。
		void BeginFrame();

		/// @brief 現在の物理入力状態からAction・Axisを評価します。
		/// @param snapshot 現在フレームの入力スナップショット
		void Evaluate(const InputSnapshot& snapshot);

		/// @brief 入力状態をリセットします
		void ResetStates();

		//---------------------------------------------------------------------
		// コンテキスト 
		//---------------------------------------------------------------------

		/// @brief 新しいコンテキストを作成します。
		/// @param name コンテキスト名
		[[nodiscard]] InputContextId CreateContext(const std::string& name);

		/// @brief コンテキストを削除します。
		/// @param contextId 削除するコンテキストのID
		bool DestroyContext(InputContextId contextId);

		/// @brief コンテキストを最上位へプッシュします。
		/// @param contextId プッシュするコンテキストのID
		bool PushContext(InputContextId contextId);

		/// @brief コンテキストをスタックから取り除きます。
		/// @param contextId ポップするコンテキストのID
		bool PopContext(InputContextId contextId);

		/// @brief コンテキストがアクティブかどうかを確認します。
		/// @param contextId 確認するコンテキストのID
		[[nodiscard]] bool IsContextActive(InputContextId contextId) const;

		[[nodiscard]] InputContextId GetDefaultContextId() const;

		//---------------------------------------------------------------------
		// コンテキスト指定バインディング
		//---------------------------------------------------------------------

		/// @brief Actionへキーをバインドします
		/// @param contextId 指定するコンテキスト
		/// @param action バインドするAction名
		/// @param key バインドするInputKey
		void BindAction(
			InputContextId     contextId,
			const std::string& action,
			const InputKey&    key
		);

		/// @brief 1D Axisへキーをバインドします。
		/// @param contextId 指定するコンテキスト
		/// @param axis バインドするAxis名
		/// @param key バインドするInputKey
		/// @param scale スケール値（デフォルトは1.0f)
		void BindAxis1D(
			InputContextId     contextId,
			const std::string& axis,
			const InputKey&    key,
			float              scale = 1.0f
		);

		/// @brief 2D Axisへキーをバインドします。
		/// @param contextId 指定するコンテキスト
		/// @param axis バインドするAxis名
		/// @param key バインドするInputKey
		/// @param axisType バインドするAxisのタイプ（XまたはY)
		/// @param scale スケール値（デフォルトは1.0f)
		void BindAxis2D(
			InputContextId     contextId,
			const std::string& axis,
			const InputKey&    key,
			INPUT_AXIS         axisType,
			float              scale = 1.0f
		);

		/// @brief 指定したアクションまたは軸からすべてのバインドを解除します
		/// @param contextId 指定するコンテキスト
		/// @param target バインドを解除するアクションまたは軸
		void Unbind(InputContextId contextId, const std::string& target);

		/// @brief 指定したアクションの状態を取得します
		/// @param contextId 指定するコンテキスト
		/// @param action 取得するアクションの名前
		/// @return 押下状態
		[[nodiscard]] bool IsPressed(
			InputContextId contextId, const std::string& action
		) const;

		/// @brief 指定したアクションの状態を取得します
		/// @param contextId 指定するコンテキスト
		/// @param action 取得するアクションの名前
		/// @return 保持状態
		[[nodiscard]] bool IsHeld(
			InputContextId contextId, const std::string& action
		) const;

		/// @brief 指定したアクションの状態を取得します
		/// @param contextId 指定するコンテキスト
		/// @param action 取得するアクションの名前
		/// @return 解放状態
		[[nodiscard]] bool IsReleased(
			InputContextId contextId, const std::string& action
		) const;

		/// @brief 指定した1D軸の値を取得します
		/// @param contextId 指定するコンテキスト
		/// @param axis 取得する軸の名前
		/// @return 軸の値
		[[nodiscard]] float Axis1D(
			InputContextId contextId, const std::string& axis
		) const;

		/// @brief 指定した2D軸の値を取得します
		/// @param contextId 指定するコンテキスト
		/// @param axis 取得する軸の名前
		/// @return 軸の値
		[[nodiscard]] Vec2 Axis2D(
			InputContextId contextId, const std::string& axis
		) const;

		/// @brief 外部からのアクション入力を処理します
		/// @param contextId 指定するコンテキスト
		/// @param action アクション名
		/// @param pressed 押下状態（trueなら押下、falseなら解放）
		void HandleExternalAction(
			InputContextId contextId, const std::string& action, bool pressed
		);

		//---------------------------------------------------------------------
		// DEPRECATED 従来API
		//---------------------------------------------------------------------

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

		/// @brief 外部からのアクション入力を処理します
		/// @param action アクション名
		/// @param pressed 押下状態（trueなら押下、falseなら解放）
		void HandleExternalAction(
			const std::string& action, bool pressed
		);

	private:
		struct ContextEntry {
			InputContextId id;
			std::string    name;

			std::unique_ptr<InputMappingContext> context;
		};

		/// @brief コンテキストを検索します
		/// @param contextId 検索するコンテキストのID
		/// @return 見つかった場合はContextEntryへのポインタ、見つからなかった場合はnullptr
		[[nodiscard]] ContextEntry* FindContext(InputContextId contextId);

		/// @brief コンテキストを検索します（const版）
		/// @param contextId 検索するコンテキストのID
		/// @return 見つかった場合はContextEntryへのポインタ、見つからなかった場合はnullptr
		[[nodiscard]] const ContextEntry* FindContext(
			InputContextId contextId
		) const;

		std::vector<ContextEntry> mContexts;

		std::vector<InputContextId> mContextStack;

		InputContextId mDefaultContextId;

		uint64_t mNextContextId = 1;
	};
}
