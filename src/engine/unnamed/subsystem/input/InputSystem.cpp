#include <pch.h>

#include <algorithm>
#include <ranges>

#include <engine/unnamed/subsystem/console/ConsoleSystem.h>
#include <engine/unnamed/subsystem/console/concommand/ConCommand.h>
#include <engine/unnamed/subsystem/input/InputSystem.h>
#include <engine/unnamed/subsystem/input/KeyNameTable.h>
#include <engine/unnamed/subsystem/input/device/gamepad/GamepadDevice.h>
#include <engine/unnamed/subsystem/input/device/keyboard/KeyboardDevice.h>
#include <engine/unnamed/subsystem/input/device/mouse/MouseDevice.h>
#include <engine/unnamed/subsystem/interface/ServiceLocator.h>

#include <Dbt.h>

namespace Unnamed {
	static constexpr std::string_view kChannel = "Input";

	namespace {
		/// @brief 指定されたウィンドウが実際にアクティブかどうかを判定します
		/// @param hWnd ウィンドウハンドル
		/// @return アクティブであればtrue
		bool IsWindowActuallyActive(const HWND hWnd) {
			if (!hWnd) {
				return false;
			}
			const HWND foreground = GetForegroundWindow();
			return foreground == hWnd;
		}
	}

	InputSystem::~InputSystem() = default;

	/// @brief 初期化
	bool InputSystem::Init() {
		ServiceLocator::Register<InputSystem>(this);

		// 起動直後に何も触っていなくてもカーソルが消えるのを防ぐため、
		// 入力システム側のデフォルトは「表示・ロック解除」に寄せておく。
		mMouseCursorLockRequested    = false;
		mMouseCursorLockOverride     = false;
		mMouseCursorLockOverrideMode = false;
		mMouseCursorVisible          = true;

		mBindCommand = std::make_unique<ConCommand>(
			"bind",
			[&](const std::vector<std::string>& args) {
				// bind <key> <command...>
				if (args.size() < 2) {
					return false;
				}

				const auto key = KeyNameTable::FromString(args[0]);
				if (!key.has_value()) {
					return false;
				}

				// コマンド部分を結合
				std::string combined;
				for (size_t i = 1; i < args.size(); ++i) {
					combined += args[i];
					if (i + 1 < args.size()) {
						combined += " ";
					}
				}

				BindCommand(
					{
						.device = key->device,
						.code   = key->code
					},
					combined
				);
				return true;
			},
			"Bind a key to a command. Usage: bind <key> <command>"
		);

		mUnbindCommand = std::make_unique<ConCommand>(
			"unbind",
			[&](const std::vector<std::string>& args) {
				// unbind <key>
				if (args.empty()) {
					return false;
				}
				const auto key = KeyNameTable::FromString(args[0]);
				if (!key.has_value()) {
					return false;
				}
				UnbindCommand({.device = key->device, .code = key->code});
				return true;
			},
			"Unbind a key from a command. Usage: unbind <key>"
		);

		mUnbindAllCommand = std::make_unique<ConCommand>(
			"unbindall",
			[&](const std::vector<std::string>&) {
				UnbindAllCommands();
				return true;
			},
			"Unbind all keys."
		);

		auto ToggleCursorLockHandler = [&](const std::vector<std::string>&) {
			const bool bNewState = !IsMouseCursorLocked();
			if (bNewState == mMouseCursorLockRequested) {
				mMouseCursorLockOverride = false;
			} else {
				mMouseCursorLockOverride     = true;
				mMouseCursorLockOverrideMode = bNewState;
			}
			// ロック状態に合わせて表示も同期する。
			SetMouseCursorVisible(!bNewState);
			if (!bNewState) {
				ClearMouseCursorLockAnchor();
			}
			DevMsg(
				kChannel,
				"Cursor lock state changed: {}",
				bNewState ? "Locked" : "Unlocked"
			);
			return true;
		};

		mToggleLockCursorCommand = std::make_unique<ConCommand>(
			"togglelockcursor",
			ToggleCursorLockHandler,
			"Toggle the cursor lock state."
		);

		mToggleCursorLockCommand = std::make_unique<ConCommand>(
			"togglecursorlock",
			ToggleCursorLockHandler,
			"Toggle the cursor lock state."
		);

		return true;
	}

	/// @brief 更新
	void InputSystem::Update(float) {
		// このフレームのアクションと軸を同じデバイス状態から組み立てる
		for (const auto& inputDevice : mDevices) {
			inputDevice->Update();
		}

		// 前フレームの状態を保存
		mSnapshot.mPreviousKeyStates = mSnapshot.mCurrentKeyStates;

		// 今フレームを再構築
		mSnapshot.mCurrentKeyStates.clear();
		mSnapshot.mAnalogValues.clear();

		// スナップショットの構築
		for (const auto& device : mDevices) {
			for (const InputKey& key : device->GetSupportedKeys()) {
				mSnapshot.mCurrentKeyStates[key] = device->GetKeyState(key);

				mSnapshot.mAnalogValues[key] = device->GetAnalogValue(key);
			}
		}

		HWND primaryMouseWindow = nullptr;
		for (const auto& device : mDevices) {
			if (device->GetDeviceType() != InputDeviceType::MOUSE) {
				continue;
			}

			const auto mouseDevice = std::static_pointer_cast<MouseDevice>(
				device
			);
			primaryMouseWindow = mouseDevice->GetWindowHandle();
			break;
		}

		if (primaryMouseWindow != nullptr) {
			POINT cursorPos = {};
			if (GetCursorPos(&cursorPos) != 0) {
				POINT clientPos = cursorPos;
				ScreenToClient(primaryMouseWindow, &clientPos);
				mSnapshot.mMouseClientPosition = Vec2(
					static_cast<float>(clientPos.x),
					static_cast<float>(clientPos.y)
				);
			}

			RECT clientRect = {};
			if (GetClientRect(primaryMouseWindow, &clientRect) != 0) {
				mSnapshot.mMouseClientViewportSize = Vec2(
					static_cast<float>(clientRect.right - clientRect.left),
					static_cast<float>(clientRect.bottom - clientRect.top)
				);
			}
		}

		// マッピングシステムのフレーム開始
		mMappingSystem.BeginFrame();

		// コマンドバインドの処理
		ProcessCommandBinds();

		// もし何かあったら外部からのアクション

		// 評価
		mMappingSystem.Evaluate(mSnapshot);

		//マウスのデルタをリセット
		for (const auto& device : mDevices) {
			if (device->GetDeviceType() == InputDeviceType::MOUSE) {
				const auto mouse = std::static_pointer_cast<
					MouseDevice>(device);
				mouse->ResetDelta();
			}
		}
	}

	void InputSystem::Shutdown() {
		mBindCommand.reset();
		mUnbindCommand.reset();
		mUnbindAllCommand.reset();
		mToggleLockCursorCommand.reset();
		mToggleCursorLockCommand.reset();
	}

	/// @brief サブシステム名の取得
	const std::string_view InputSystem::GetName() const {
		return "InputSystem";
	}

	/// @brief Win32メッセージの処理
	/// @param hWnd ウィンドウハンドル
	/// @param msg メッセージID
	/// @param wParam メッセージのWPARAM
	/// @param lParam メッセージのLPARAM
	/// @return メッセージが処理された場合はtrue、そうでない場合はfalse
	bool InputSystem::OnWin32Message(
		[[maybe_unused]] HWND hWnd,
		const UINT            msg,
		const WPARAM          wParam,
		const LPARAM          lParam
	) {
		switch (msg) {
			case WM_ACTIVATE:
				// ウィンドウが非アクティブになったら入力をリセット
				if (LOWORD(wParam) == WA_INACTIVE) {
					ResetInputStates();
					// 非アクティブ時はカーソルクリップを解除
					ClipCursor(nullptr);
				}
				break;

			case WM_KILLFOCUS:
				// フォーカス喪失も解除（Alt+Tab、Winキー時）
				ResetInputStates();
				ClipCursor(nullptr);
				break;

			case WM_INPUT: {
				// RAW Inputを受け取る
				UINT size = 0;
				GetRawInputData(
					reinterpret_cast<HRAWINPUT>(lParam),
					// NOLINT(performance-no-int-to-ptr)
					RID_INPUT,
					nullptr,
					&size,
					sizeof(RAWINPUTHEADER)
				);

				std::vector<uint8_t> buffer(size);

				if (
					GetRawInputData(
						reinterpret_cast<HRAWINPUT>(lParam),
						// NOLINT(performance-no-int-to-ptr)
						RID_INPUT,
						buffer.data(),
						&size,
						sizeof(RAWINPUTHEADER)
					) == size
				) {
					const RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.
						data());
					OnRawInput(*raw);
				}
				break;
			}

			case WM_DEVICECHANGE: {
				if (
					wParam == DBT_DEVICEARRIVAL ||
					wParam == DBT_DEVICEREMOVECOMPLETE ||
					wParam == DBT_DEVNODES_CHANGED
				) {
					for (const auto& device : mDevices) {
						if (device->GetDeviceType() !=
							InputDeviceType::GAMEPAD) {
							continue;
						}

						std::static_pointer_cast<GamepadDevice>(device)
								->RequestDirectInputRefresh();
					}
				}
				break;
			}

			default: break;
		}

		return false;
	}

	void InputSystem::CheckMouseCursorLockAndVisibility(
		const HWND hWnd
	) const {
		static int  cursorCount        = 0;
		static bool sLastCursorVisible = true;

		const bool bActive        = IsWindowActuallyActive(hWnd);
		const bool bCursorLocked  = ResolveMouseCursorLockState();
		const bool bCursorVisible = ResolveMouseCursorVisibleState();

		// 非アクティブ中は OS 側の挙動や他 UI による ShowCursor 操作と競合しやすいので、
		// カーソルは必ず「表示」に寄せる。
		if (!bActive && !bCursorVisible) {
			// const 関数のため、表示状態そのものは変えずに OS 側だけ可視へ収束させる
			sLastCursorVisible          = true;
			constexpr int maxIterations = 32;
			int           it            = 0;
			while (cursorCount < 0 && it++ < maxIterations) {
				cursorCount = ShowCursor(TRUE);
			}
		}

		// マウスカーソルのロックと表示状態の更新
		if (bCursorLocked) {
			// 非アクティブ時は必ずロック解除
			if (!bActive) {
				ClipCursor(nullptr);
			} else {
				const HWND anchorHwnd = mMouseLockAnchor.valid &&
				                        mMouseLockAnchor.hwnd != nullptr ?
					                        mMouseLockAnchor.hwnd :
					                        hWnd;
				POINT lockPos;
				if (mMouseLockAnchor.valid) {
					lockPos.x = static_cast<LONG>(mMouseLockAnchor.clientPos.x);
					lockPos.y = static_cast<LONG>(mMouseLockAnchor.clientPos.y);
				} else {
					RECT clientRect = {};
					GetClientRect(anchorHwnd, &clientRect);
					lockPos.x = (clientRect.right - clientRect.left) / 2;
					lockPos.y = (clientRect.bottom - clientRect.top) / 2;
				}
				ClientToScreen(anchorHwnd, &lockPos);

				// ウィンドウがアクティブな場合のみロック
				if (bActive) {
					RECT rect;
					rect.left   = lockPos.x;
					rect.top    = lockPos.y;
					rect.right  = lockPos.x + 1;
					rect.bottom = lockPos.y + 1;
					ClipCursor(&rect); // ClipCursorのほうがガタガタしない
					SetCursorPos(lockPos.x, lockPos.y);
				}
			}
		} else {
			// カーソルロック解除
			ClipCursor(nullptr);
		}

		if (bCursorVisible != sLastCursorVisible) {
			sLastCursorVisible = bCursorVisible;

			// ShowCursor の戻り値: 0 以上なら表示、0 未満なら非表示
			constexpr int maxIterations = 32;
			int           it            = 0;

			if (bCursorVisible) {
				while (cursorCount < 0 && it++ < maxIterations) {
					cursorCount = ShowCursor(TRUE);
				}
			} else {
				while (cursorCount >= 0 && it++ < maxIterations) {
					cursorCount = ShowCursor(FALSE);
				}
			}

			if (it >= maxIterations) {
				Warning(
					kChannel,
					"ShowCursor のカウントが収束しませんでした (targetVisible={}, lastCount={}).",
					bCursorVisible,
					cursorCount
				);
			}
		}
	}

	/// @brief 入力デバイスを登録します
	/// @param device 登録する入力デバイスの共有ポインタ
	void InputSystem::RegisterDevice(
		const std::shared_ptr<BaseInputDevice>& device
	) {
		mDevices.emplace_back(device);
	}

	void InputSystem::PlayGamepadRumble(
		const float low,
		const float high,
		const float durationSec
	) const {
		for (const auto& device : mDevices) {
			if (device->GetDeviceType() != InputDeviceType::GAMEPAD) {
				continue;
			}

			std::static_pointer_cast<GamepadDevice>(device)->EnqueueRumble(
				low,
				high,
				durationSec
			);
			return;
		}
	}

	void InputSystem::StopGamepadRumble() const {
		for (const auto& device : mDevices) {
			if (device->GetDeviceType() != InputDeviceType::GAMEPAD) {
				continue;
			}

			std::static_pointer_cast<GamepadDevice>(device)->StopRumble();
			return;
		}
	}

	void InputSystem::BindCommand(
		const InputKey& key, const std::string& command
	) {
		auto& commands = mCommandBinds[key];
		if (!std::ranges::contains(commands, command)) {
			commands.emplace_back(command);
		}
	}

	void InputSystem::UnbindCommand(const InputKey& key) {
		mCommandBinds.erase(key);
		DevMsg(
			kChannel,
			"コマンドのバインドを解除しました: key = {}",
			KeyNameTable::ToString(key)
		);
	}

	void InputSystem::UnbindAllCommands() {
		mCommandBinds.clear();
		DevMsg(kChannel, "すべてのコマンドバインドを解除しました。");
	}

	std::string InputSystem::GetBoundCommand(const InputKey& key) const {
		const auto it = mCommandBinds.find(key);
		if (it == mCommandBinds.end() || it->second.empty()) {
			return {};
		}

		std::string combined;
		for (size_t i = 0; i < it->second.size(); ++i) {
			combined += it->second[i];
			if (i + 1 < it->second.size()) {
				combined += "; ";
			}
		}
		return combined;
	}

	void InputSystem::BindAction(
		const std::string& action, const InputKey& key
	) {
		mMappingSystem.BindAction(action, key);
	}

	void InputSystem::BindAxis1D(
		const std::string& axis, const InputKey& key, float scale
	) {
		mMappingSystem.BindAxis1D(axis, key, scale);
	}

	void InputSystem::BindAxis2D(
		const std::string& axis, const InputKey&  key,
		const INPUT_AXIS&  axisType, const float& scale
	) {
		mMappingSystem.BindAxis2D(axis, key, axisType, scale);
	}

	void InputSystem::Unbind(const std::string& action) {
		mMappingSystem.Unbind(action);
	}

	void InputSystem::HandleConsoleAction(
		const std::string& action, const bool& bPressed
	) {
		mMappingSystem.HandleExternalAction(action, bPressed);
	}

	bool InputSystem::IsPressed(const std::string& action) const {
		return mMappingSystem.IsPressed(action);
	}

	bool InputSystem::IsHeld(const std::string& action) const {
		return mMappingSystem.IsHeld(action);
	}

	bool InputSystem::IsReleased(const std::string& action) const {
		return mMappingSystem.IsReleased(action);
	}

	/// @brief 指定した1D軸の値を取得します
	/// @param axis 軸名
	/// @return 軸の値
	float InputSystem::Axis1D(const std::string& axis) const {
		return mMappingSystem.Axis1D(axis);
	}

	/// @brief 指定した2D軸の値を取得します
	/// @param axis 軸名
	/// @return 軸の値
	Vec2 InputSystem::Axis2D(const std::string& axis) const {
		return mMappingSystem.Axis2D(axis);
	}

	bool InputSystem::IsMouseCursorLocked() const {
		return ResolveMouseCursorLockState();
	}

	void InputSystem::SetMouseCursorLocked(const bool& locked) {
		mMouseCursorLockRequested = locked;
		if (!ResolveMouseCursorLockState()) {
			ClearMouseCursorLockAnchor();
		}
	}

	bool InputSystem::IsMouseCursorVisible() const {
		return ResolveMouseCursorVisibleState();
	}

	void InputSystem::SetMouseCursorVisible(const bool& visible) {
		mMouseCursorVisible = visible;
	}

	void InputSystem::SetMouseCursorLockClientPosition(
		const HWND hwnd, const Vec2& clientPos
	) {
		mMouseLockAnchor.hwnd      = hwnd;
		mMouseLockAnchor.clientPos = clientPos;
		mMouseLockAnchor.valid     = hwnd != nullptr;
	}

	void InputSystem::ClearMouseCursorLockAnchor() {
		mMouseLockAnchor = {};
	}

	void InputSystem::SetMouseClientViewportRect(
		const Vec2& origin,
		const Vec2& size
	) {
		SetMouseClientViewportRect(origin, size, size);
	}

	void InputSystem::SetMouseClientViewportRect(
		const Vec2& origin,
		const Vec2& size,
		const Vec2& virtualSize
	) {
		if (size.x <= 0.0f || size.y <= 0.0f) {
			ClearMouseClientViewportRectOverride();
			return;
		}
		if (virtualSize.x <= 0.0f || virtualSize.y <= 0.0f) {
			ClearMouseClientViewportRectOverride();
			return;
		}

		mMouseViewportRectOverride = true;
		mMouseViewportOrigin       = origin;
		mMouseViewportSizeOverride = Vec2(
			std::max(1.0f, size.x),
			std::max(1.0f, size.y)
		);
		mMouseViewportVirtualSize = Vec2(
			std::max(1.0f, virtualSize.x),
			std::max(1.0f, virtualSize.y)
		);
	}

	void InputSystem::ClearMouseClientViewportRectOverride() {
		mMouseViewportRectOverride = false;
		mMouseViewportOrigin       = Vec2::zero;
		mMouseViewportSizeOverride = Vec2::zero;
		mMouseViewportVirtualSize  = Vec2::zero;
	}

	Vec2 InputSystem::GetMouseClientPosition() const {
		if (!mMouseViewportRectOverride) {
			return mSnapshot.mMouseClientPosition;
		}

		auto localPos = Vec2(
			mSnapshot.mMouseClientPosition.x - mMouseViewportOrigin.x,
			mSnapshot.mMouseClientPosition.y - mMouseViewportOrigin.y
		);
		if (
			mMouseViewportSizeOverride.x > 0.0f &&
			mMouseViewportSizeOverride.y > 0.0f &&
			mMouseViewportVirtualSize.x > 0.0f &&
			mMouseViewportVirtualSize.y > 0.0f
		) {
			localPos.x *=
					mMouseViewportVirtualSize.x / mMouseViewportSizeOverride.x;
			localPos.y *=
					mMouseViewportVirtualSize.y / mMouseViewportSizeOverride.y;
		}
		return localPos;
	}

	Vec2 InputSystem::GetMouseClientViewportSize() const {
		if (
			mMouseViewportRectOverride &&
			mMouseViewportVirtualSize.x > 0.0f &&
			mMouseViewportVirtualSize.y > 0.0f
		) {
			return mMouseViewportVirtualSize;
		}
		return mSnapshot.mMouseClientViewportSize;
	}

	bool InputSystem::IsMouseButtonDown(
		const MOUSE_INPUT_CODE code
	) const {
		return mSnapshot.IsKeyDown(InputKey::Mouse(code));
	}

	bool InputSystem::WasMouseButtonPressed(
		const MOUSE_INPUT_CODE code
	) const {
		return mSnapshot.WasKeyPressed(InputKey::Mouse(code));
	}

	bool InputSystem::WasMouseButtonReleased(
		const MOUSE_INPUT_CODE code
	) const {
		return mSnapshot.WasKeyReleased(InputKey::Mouse(code));
	}

	const InputSnapshot& InputSystem::GetSnapshot() const {
		return mSnapshot;
	}

	bool InputSystem::ResolveMouseCursorLockState() const {
		return mMouseCursorLockOverride ?
			       mMouseCursorLockOverrideMode :
			       mMouseCursorLockRequested;
	}

	bool InputSystem::ResolveMouseCursorVisibleState() const {
		if (mMouseCursorLockOverride && !mMouseCursorLockOverrideMode) {
			return true;
		}
		return mMouseCursorVisible;
	}

	/// @brief 入力状態をリセットします
	void InputSystem::ResetInputStates() {
		DevMsg(
			kChannel,
			"ウィンドウが非アクティブになったため、入力状態をリセットします。"
		);

		// 物理入力のスナップショットをリセット
		mSnapshot.mCurrentKeyStates.clear();
		mSnapshot.mPreviousKeyStates.clear();
		mSnapshot.mAnalogValues.clear();

		// 論理入力状態をリセット
		mMappingSystem.ResetStates();

		// デバイス内部の入力状態をリセット
		for (const auto& device : mDevices) {
			device->ResetStates();
		}

		// マウス関連状態をリセット
		ClearMouseCursorLockAnchor();

		mSnapshot.mMouseClientPosition     = Vec2::zero;
		mSnapshot.mMouseClientViewportSize = Vec2::zero;
	}

	/// @brief RAW Inputの処理
	/// @param rawInput RAW Inputデータ
	void InputSystem::OnRawInput(const RAWINPUT& rawInput) const {
		for (auto& device : mDevices) {
			if (device->GetDeviceType() == InputDeviceType::KEYBOARD) {
				std::static_pointer_cast<KeyboardDevice>(device)->
						HandleRawInput(rawInput);
			}

			if (device->GetDeviceType() == InputDeviceType::MOUSE) {
				std::static_pointer_cast<MouseDevice>(device)->
						HandleRawInput(rawInput);
			}
		}
	}

	namespace {
		/// @brief 押下コマンドから離脱コマンドを生成します
		/// @param cmd 押下コマンド文字列
		/// @return 離脱コマンド文字列（押下コマンドでない場合は空文字列）
		std::string MakeReleaseCommandFromPress(const std::string& cmd) {
			const std::string trimmed = StrUtil::TrimSpaces(cmd);
			if (trimmed.empty() || trimmed[0] != '+') {
				return {};
			}

			const size_t firstSpace = trimmed.find(' ');
			const size_t len        =
					firstSpace == std::string::npos ?
						trimmed.size() :
						firstSpace;

			std::string out;
			out.reserve(len);
			out.push_back('-');
			out.append(trimmed, 1, len - 1);
			return out;
		}
	}

	void InputSystem::ProcessCommandBinds() {
		auto* console = ServiceLocator::Get<ConsoleSystem>();
		if (!console) {
			return;
		}

		for (const auto& [key, commands] : mCommandBinds) {
			const bool bPressed  = mSnapshot.WasKeyPressed(key);
			const bool bReleased = mSnapshot.WasKeyReleased(key);

			if (bPressed) {
				for (const auto& command : commands) {
					console->ExecuteCommand(
						command, EXEC_FLAG::FROM_USER | EXEC_FLAG::SILENT
					);
				}
			}

			if (bReleased) {
				for (const auto& command : commands) {
					const auto releasedCommand = MakeReleaseCommandFromPress(
						command
					);

					if (!releasedCommand.empty()) {
						console->ExecuteCommand(
							releasedCommand,
							EXEC_FLAG::FROM_USER | EXEC_FLAG::SILENT
						);
					}
				}
			}
		}
	}
}
