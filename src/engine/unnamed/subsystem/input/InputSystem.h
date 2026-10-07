#pragma once
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <engine/IWin32MsgListener.h>
#include <engine/unnamed/subsystem/input/InputSnapshot.h>
#include <engine/unnamed/subsystem/input/device/base/BaseInputDevice.h>
#include <engine/unnamed/subsystem/input/mapping/InputMappingSystem.h>
#include <engine/unnamed/subsystem/interface/ISubsystem.h>

namespace Unnamed {
	class ConCommand;
	class ConsoleSystem;

	/// @brief InputSystemは、入力の初期化、フレーム更新、および終了順序を統括します
	class InputSystem final : public ISubsystem, public IWin32MsgListener {
	public:
		~InputSystem() override;

		//---------------------------------------------------------------------
		// ISubsystem
		//---------------------------------------------------------------------
		bool Init() override;
		void Update(float) override;
		void Shutdown() override;

		[[nodiscard]] const std::string_view GetName() const override;

		//---------------------------------------------------------------------
		// IWin32MsgListener
		//---------------------------------------------------------------------
		/// @brief Win32メッセージの処理
		/// @param hWnd ウィンドウハンドル
		/// @param msg メッセージID
		/// @param wParam メッセージのWPARAM
		/// @param lParam メッセージのLPARAM
		/// @return メッセージが処理された場合はtrue、そうでない場合はfalse
		bool OnWin32Message(
			HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
		) override;

		//---------------------------------------------------------------------
		// InputSystem
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

		/// @brief 指定したアクションが押下されているかを取得します
		/// @param action 取得するアクション名
		/// @return 押下されている場合はtrue、押下されていない場合はfalse
		[[nodiscard]] bool IsPressed(const std::string& action) const;

		/// @brief 指定したアクションが保持されているかを取得します
		/// @param action 取得するアクション名
		/// @return 保持されている場合はtrue、保持されていない場合はfalse
		[[nodiscard]] bool IsHeld(const std::string& action) const;

		/// @brief 指定したアクションが解放されているかを取得します
		/// @param action 取得するアクション名
		/// @return 解放されている場合はtrue、解放されていない場合はfalse
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

		/// @brief マウスカーソルのロック位置をクリアします
		void ClearMouseCursorLockAnchor();

		/// @brief マウスカーソルのクライアントビューポート矩形を設定します
		/// @param origin ビューポートの原点(クライアント座標)
		/// @param size ビューポートのサイズ(クライアント座標)
		void SetMouseClientViewportRect(const Vec2& origin, const Vec2& size);

		/// @brief マウスカーソルのクライアントビューポート矩形を設定します（仮想サイズを指定）
		/// @param origin ビューポートの原点(クライアント座標)
		/// @param size ビューポートのサイズ(クライアント座標)
		/// @param virtualSize ビューポートの仮想サイズ(クライアント座標)
		void SetMouseClientViewportRect(
			const Vec2& origin,
			const Vec2& size,
			const Vec2& virtualSize
		);

		/// @brief マウスカーソルのクライアントビューポート矩形のオーバーライドをクリアします
		void ClearMouseClientViewportRectOverride();

		/// @brief クライアント座標系でのマウス座標を取得します。
		/// @return クライアント座標系でのマウス座標
		[[nodiscard]] Vec2 GetMouseClientPosition() const;

		/// @brief マウスカーソルのクライアントビューポートサイズを取得します。
		/// @return マウスカーソルのクライアントビューポートサイズ
		[[nodiscard]] Vec2 GetMouseClientViewportSize() const;

		/// @brief 指定したマウスボタンが押されているかどうかを取得します
		/// @param code 取得するマウスボタンのコード
		/// @return 押されている場合はtrue、押されていない場合はfalse
		[[nodiscard]] bool IsMouseButtonDown(MOUSE_INPUT_CODE code) const;

		/// @brief 指定したマウスボタンがこのフレームで押されたかどうかを取得します
		/// @param code 取得するマウスボタンのコード
		/// @return 押された場合はtrue、押されていない場合はfalse
		[[nodiscard]] bool WasMouseButtonPressed(
			MOUSE_INPUT_CODE code
		) const;

		/// @brief 指定したマウスボタンがこのフレームで離されたかどうかを取得します
		/// @param code 取得するマウスボタンのコード
		/// @return 離された場合はtrue、離されていない場合はfalse
		[[nodiscard]] bool WasMouseButtonReleased(
			MOUSE_INPUT_CODE code
		) const;

		/// @brief 現在フレームの物理入力状態を取得します。
		/// @return 入力状態のスナップショット
		[[nodiscard]] const InputSnapshot& GetSnapshot() const;

		// ------ InputContext ------------------------------------------------

		/// @brief 入力コンテキストを作成します。
		[[nodiscard]] InputContextId CreateInputContext(
			const std::string& name,
			INPUT_CONTEXT_MODE mode = INPUT_CONTEXT_MODE::PASS_THROUGH
		);

		/// @brief 入力コンテキストを削除します。
		/// @param contextId 削除する入力コンテキストのID
		bool DestroyInputContext(InputContextId contextId);

		/// @brief 入力コンテキストを最上位へ追加します。
		/// @param contextId 追加する入力コンテキストのID
		bool PushInputContext(InputContextId contextId);

		/// @brief 入力コンテキストをスタックから取り除きます。
		/// @param contextId 取り除く入力コンテキストのID
		bool PopInputContext(InputContextId contextId);

		/// @brief 入力コンテキストが有効かを取得します。
		/// @param contextId 取得する入力コンテキストのID
		[[nodiscard]] bool IsInputContextActive(InputContextId contextId) const;

		/// @brief 入力コンテキストにアクションをバインドします。
		/// @param contextId バインドする入力コンテキストのID
		/// @param action バインドするアクション名
		/// @param key バインドする入力キー
		/// @param consumeInput 入力を消費するかどうか（デフォルトはfalse）
		void BindAction(
			InputContextId     contextId,
			const std::string& action,
			const InputKey&    key,
			bool               consumeInput = false
		);

		/// @brief 入力コンテキストに1D軸をバインドします。
		/// @param contextId バインドする入力コンテキストのID
		/// @param axis バインドする軸名
		/// @param key バインドする入力キー
		/// @param scale スケール値（デフォルトは1.0f）
		/// @param consumeInput 入力を消費するかどうか（デフォルトはfalse）
		void BindAxis1D(
			InputContextId     contextId,
			const std::string& axis,
			const InputKey&    key,
			float              scale        = 1.0f,
			bool               consumeInput = false
		);

		/// @brief 入力コンテキストに2D軸をバインドします。
		/// @param contextId バインドする入力コンテキストのID
		/// @param axis バインドする軸名
		/// @param key バインドする入力キー
		/// @param axisType 軸のタイプ
		/// @param scale スケール値（デフォルトは1.0f）
		/// @param consumeInput 入力を消費するかどうか（デフォルトはfalse）
		void BindAxis2D(
			InputContextId     contextId,
			const std::string& axis,
			const InputKey&    key,
			INPUT_AXIS         axisType,
			float              scale        = 1.0f,
			bool               consumeInput = false
		);

		/// @brief 指定したアクションが押下されているかを取得します
		/// @param contextId 取得する入力コンテキストのID
		/// @param action 取得するアクション名
		/// @return 押下されている場合はtrue、押下されていない場合はfalse
		[[nodiscard]] bool IsPressed(
			InputContextId     contextId,
			const std::string& action
		) const;

		/// @brief 指定したアクションが保持されているかを取得します
		/// @param contextId 取得する入力コンテキストのID
		/// @param action 取得するアクション名
		/// @return 保持されている場合はtrue、保持されていない場合はfalse
		[[nodiscard]] bool IsHeld(
			InputContextId     contextId,
			const std::string& action
		) const;

		/// @brief 指定したアクションが解放されているかを取得します
		/// @param contextId 取得する入力コンテキストのID
		/// @param action 取得するアクション名
		/// @return 解放されている場合はtrue、解放されていない場合はfalse
		[[nodiscard]] bool IsReleased(
			InputContextId     contextId,
			const std::string& action
		) const;

		/// @brief 指定した1D軸の値を取得します
		/// @param contextId 取得する入力コンテキストのID
		/// @param axis 取得する軸の名前
		/// @return 軸の値
		[[nodiscard]] float Axis1D(
			InputContextId contextId, const std::string& axis
		) const;

		/// @brief 指定した2D軸の値を取得します
		/// @param contextId 取得する入力コンテキストのID
		/// @param axis 取得する軸の名前
		/// @return 軸の値
		[[nodiscard]] Vec2 Axis2D(
			InputContextId contextId, const std::string& axis
		) const;

		/// @brief 外部からのアクション入力を処理します
		/// @param contextId 取得する入力コンテキストのID
		/// @param action 取得するアクション名
		/// @param pressed 押下状態（trueなら押下、falseなら解放）
		void HandleExternalAction(
			InputContextId     contextId,
			const std::string& action,
			bool               pressed
		);

		/// @brief 入力コンテキストに属するコンソールコマンドをキーへバインドします
		/// @param contextId バインド先の入力コンテキスト
		/// @param key 入力キー
		/// @param command 実行するコンソールコマンド
		/// @param consumeInput 入力を消費するかどうか
		void BindContextCommand(
			InputContextId     contextId, const InputKey& key,
			const std::string& command, bool              consumeInput
		);

	private:
		struct InputContextCommandBinding {
			InputContextId contextId;
			InputKey       key;

			std::vector<std::string> commands;

			bool consumeInput = false;
		};

		struct ActiveCommand {
			// コンテキストの場合はContextID、グローバルの場合は std::nullopt
			std::optional<InputContextId> contextId;

			InputKey    key;
			std::string pressCommand;
		};

		/// @brief マウスカーソルのロック状態を解決します
		/// @return ロックされている場合はtrue、ロックされていない場合はfalse
		[[nodiscard]] bool ResolveMouseCursorLockState() const;

		/// @brief マウスカーソルの表示状態を解決します
		/// @return 表示されている場合はtrue、非表示の場合はfalse
		[[nodiscard]] bool ResolveMouseCursorVisibleState() const;

		/// @brief 入力状態をリセットします
		void ResetInputStates();

		/// @brief RAWINPUTメッセージを処理します
		/// @param rawInput RAWINPUT構造体
		void OnRawInput(const RAWINPUT& rawInput) const;

		/// @brief コマンドバインドを押下/解放に応じて実行します
		void ProcessCommandBinds();

		/// @brief Releaseが必要なコマンドをアクティブとして登録します。
		/// @param contextId コンテキストID（グローバルの場合はstd::nullopt）
		/// @param key 入力キー
		/// @param pressCommand 押下コマンド文字列
		void RegisterActiveCommand(
			std::optional<InputContextId> contextId,
			const InputKey&               key,
			const std::string&            pressCommand
		);

		/// @brief 現在Release条件を満たしているActiveCommandを解放します。
		/// @param console コンソールシステム
		void ProcessActiveCommandReleases(ConsoleSystem& console);

		/// @brief すべてのアクティブコマンドを強制的に解放します。
		void ReleaseAllActiveCommands(ConsoleSystem* console);

		//---------------------------------------------------------------------

		InputSnapshot      mSnapshot;
		InputMappingSystem mMappingSystem;

		// コマンドバインドのマップ（キー -> コマンド文字列のリスト）
		std::unordered_map<InputKey, std::vector<std::string>> mCommandBinds;

		// コンテキストに属するコマンドバインド
		std::vector<InputContextCommandBinding> mContextCommandBinds;

		// コンテキストが入力権を失ったことを検知する
		std::vector<ActiveCommand> mActiveCommands;

		// 入力デバイスのリスト
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

		std::unique_ptr<ConCommand> mBindContextCommand;
		std::unique_ptr<ConCommand> mInputContextCommand;

		std::unique_ptr<ConCommand> mToggleLockCursorCommand;
		std::unique_ptr<ConCommand> mToggleCursorLockCommand;
	};
}
