#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <engine/IWin32MsgListener.h>
#include <engine/unnamed/subsystem/input/InputSnapshot.h>
#include <engine/unnamed/subsystem/input/device/base/BaseInputDevice.h>
#include <engine/unnamed/subsystem/interface/ISubsystem.h>
#include <engine/unnamed/subsystem/input/mapping/InputMappingSystem.h>

namespace Unnamed {
	class ConCommand;

	/// @brief InputSystemは、入力の初期化、フレーム更新、および終了順序を統括します
	class InputSystem final : public ISubsystem, public IWin32MsgListener {
	public:
		~InputSystem() override;

		// ISubsystem
		bool Init() override;
		void Update(float) override;
		void Shutdown() override;

		[[nodiscard]] const std::string_view GetName() const override;

		// IWin32MsgListener
		bool OnWin32Message(HWND, UINT, WPARAM, LPARAM) override;

		//---------------------------------------------------------------------

		/// @brief マウスカーソルのロックと表示状態をチェックします
		/// @param hWnd ウィンドウハンドル
		void CheckMouseCursorLockAndVisibility(HWND hWnd) const;

		/// @brief 入力デバイスを登録します
		/// @param device 登録する入力デバイスの共有ポインタ
		void RegisterDevice(const std::shared_ptr<BaseInputDevice>& device);

		/// @brief ゲームパッドランブルを追加再生します
		/// @param low 低周波モータ強度 [0..1]
		/// @param high 高周波モータ強度 [0..1]
		/// @param durationSec 再生時間（秒）
		void PlayGamepadRumble(float low, float high, float durationSec) const;

		/// @brief すべてのゲームパッドランブルを停止します
		void StopGamepadRumble() const;

		/// @brief コマンドをキーにバインドします
		/// @param key バインドするキー
		/// @param command バインドするコマンド文字列
		void BindCommand(const InputKey& key, const std::string& command);

		/// @brief キーからバインドされているコマンドを解除します
		/// @param key バインドを解除するキー
		void UnbindCommand(const InputKey& key);

		/// @brief すべてのコマンドバインドを解除します
		void UnbindAllCommands();

		/// @brief キーにバインドされているコマンドを取得します
		/// @param key キー
		/// @return バインドされているコマンド文字列（存在しない場合は空文字列）
		[[nodiscard]] std::string GetBoundCommand(const InputKey& key) const;

		/// @brief アクションまたは軸にキーをバインドします
		/// @param action バインドするアクションまたは軸の名前
		/// @param key バインドするキー
		void BindAction(const std::string& action, const InputKey& key);

		/// @brief 1D軸にキーをバインドします
		/// @param axis バインドする軸の名前
		/// @param key バインドするキー
		/// @param scale スケール値（デフォルトは1.0f）
		void BindAxis1D(
			const std::string& axis,
			const InputKey&    key,
			float              scale = 1.0f
		);

		/// @brief 2D軸にキーをバインドします
		/// @param axis バインドする軸の名前
		/// @param key バインドするキー
		/// @param axisType 軸のタイプ（XまたはY）
		/// @param scale スケール値（デフォルトは1.0f）
		void BindAxis2D(
			const std::string& axis,
			const InputKey&    key,
			const INPUT_AXIS&  axisType,
			const float&       scale = 1.0f
		);

		/// @brief アクションまたは軸からすべてのバインドを解除します
		/// @param action バインドを解除するアクションまたは軸の名前
		void Unbind(const std::string& action);

		/// @brief コンソールからのアクション入力を処理します
		/// @param action アクション名
		/// @param bPressed 押下状態（trueなら押下、falseなら解放）
		void HandleConsoleAction(
			const std::string& action, const bool& bPressed
		);

		/// @brief 指定したアクションの状態を取得します
		/// @param action 取得するアクションの名前
		/// @return 押下状態、保持状態、解放状態
		[[nodiscard]] bool IsPressed(const std::string& action) const;

		/// @brief 指定したアクションの状態を取得します
		/// @param action 取得するアクションの名前
		/// @return 押下状態、保持状態、解放状態
		[[nodiscard]] bool IsHeld(const std::string& action) const;

		/// @brief 指定したアクションの状態を取得します
		/// @param action 取得するアクションの名前
		/// @return 押下状態、保持状態、解放状態
		[[nodiscard]] bool IsReleased(const std::string& action) const;

		/// @brief 指定した1D軸の値を取得します
		/// @param axis 取得する軸の名前
		/// @return 軸の値
		[[nodiscard]] float Axis1D(const std::string& axis) const;

		/// @brief 指定した2D軸の値を取得します
		/// @param axis 取得する軸の名前
		/// @return 軸の値
		[[nodiscard]] Vec2 Axis2D(const std::string& axis) const;

		/// @brief マウスカーソルがロックされているかどうかを取得します
		/// @return ロックされているならtrue
		[[nodiscard]] bool IsMouseCursorLocked() const;

		/// @brief マウスカーソルのロック状態を設定します
		/// @param locked ロックするならtrue、解除するならfalse
		void SetMouseCursorLocked(const bool& locked);

		/// @brief マウスカーソルが表示されているかどうかを取得します
		/// @return 表示されているならtrue
		[[nodiscard]] bool IsMouseCursorVisible() const;

		/// @brief マウスカーソルの表示状態を設定します
		/// @param visible 表示するならtrue、非表示にするならfalse
		void SetMouseCursorVisible(const bool& visible);

		/// @brief マウスカーソルのロック位置を設定します
		/// @param hwnd ロック位置の基準となるウィンドウ
		/// @param clientPos ロック位置のクライアント座標
		void SetMouseCursorLockClientPosition(HWND hwnd, const Vec2& clientPos);
		void ClearMouseCursorLockAnchor();
		void SetMouseClientViewportRect(const Vec2& origin, const Vec2& size);
		void SetMouseClientViewportRect(
			const Vec2& origin,
			const Vec2& size,
			const Vec2& virtualSize
		);
		void ClearMouseClientViewportRectOverride();

		[[nodiscard]] Vec2 GetMouseClientPosition() const;
		[[nodiscard]] Vec2 GetMouseClientViewportSize() const;
		[[nodiscard]] bool IsMouseButtonDown(MOUSE_INPUT_CODE code) const;
		[[nodiscard]] bool WasMouseButtonPressed(
			MOUSE_INPUT_CODE code
		) const;
		[[nodiscard]] bool WasMouseButtonReleased(
			MOUSE_INPUT_CODE code
		) const;

		/// @brief 現在フレームの物理入力状態を取得します。
		/// @return 入力状態のスナップショット
		[[nodiscard]] const InputSnapshot& GetSnapshot() const;

	private:
		[[nodiscard]] bool ResolveMouseCursorLockState() const;
		[[nodiscard]] bool ResolveMouseCursorVisibleState() const;

		/// @brief 入力状態をリセットします
		void ResetInputStates();

		/// @brief RAWINPUTメッセージを処理します
		/// @param rawInput RAWINPUT構造体
		void OnRawInput(const RAWINPUT& rawInput) const;

		/// @brief コマンドバインドを押下/解放に応じて実行します
		void ProcessCommandBinds();

		InputSnapshot      mSnapshot;
		InputMappingSystem mMappingSystem;

		std::unordered_map<InputKey, std::vector<std::string>> mCommandBinds;

		std::vector<std::shared_ptr<BaseInputDevice>> mDevices;

		/// @brief MouseLockAnchorは、mouse lock開始時のscreen座標とclient座標を復帰位置として保持します
		struct MouseLockAnchor {
			HWND hwnd      = nullptr;
			Vec2 clientPos = Vec2::zero;
			bool valid     = false;
		};

		MouseLockAnchor mMouseLockAnchor             = {};
		bool            mMouseCursorLockRequested    = false;
		bool            mMouseCursorLockOverride     = false;
		bool            mMouseCursorLockOverrideMode = false;
		bool            mMouseCursorVisible          = false;

		bool mMouseViewportRectOverride = false;
		Vec2 mMouseViewportOrigin       = Vec2::zero;
		Vec2 mMouseViewportSizeOverride = Vec2::zero;
		Vec2 mMouseViewportVirtualSize  = Vec2::zero;

		std::unique_ptr<ConCommand> mBindCommand;
		std::unique_ptr<ConCommand> mUnbindCommand;
		std::unique_ptr<ConCommand> mUnbindAllCommand;
		std::unique_ptr<ConCommand> mToggleLockCursorCommand;
		std::unique_ptr<ConCommand> mToggleCursorLockCommand;
	};
}
