#include <algorithm>
#include <Dbt.h>
#include <optional>
#include <pch.h>
#include <ranges>
#include <unordered_set>
#include <utility>

#include <engine/unnamed/subsystem/console/ConsoleSystem.h>
#include <engine/unnamed/subsystem/console/concommand/ConCommand.h>
#include <engine/unnamed/subsystem/input/InputSystem.h>
#include <engine/unnamed/subsystem/input/KeyNameTable.h>
#include <engine/unnamed/subsystem/input/device/gamepad/GamepadDevice.h>
#include <engine/unnamed/subsystem/input/device/keyboard/KeyboardDevice.h>
#include <engine/unnamed/subsystem/input/device/mouse/MouseDevice.h>
#include <engine/unnamed/subsystem/interface/ServiceLocator.h>

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

		/// @brief 入力コンテキストモードを文字列から解析します
		/// @param value 文字列
		/// @return 解析結果（不正な場合はstd::nullopt）
		[[nodiscard]] std::optional<INPUT_CONTEXT_MODE> ParseInputContextMode(
			const std::string& value
		) {
			if (value == "pass_through") {
				return INPUT_CONTEXT_MODE::PASS_THROUGH;
			}

			if (value == "block_lower") {
				return INPUT_CONTEXT_MODE::BLOCK_LOWER;
			}

			return std::nullopt;
		}

		/// @brief 引数の文字列を結合します
		/// @param args 引数の文字列ベクター
		/// @param beginIndex 結合を開始するインデックス
		/// @return 結合された文字列
		[[nodiscard]] std::string JoinArguments(
			const std::vector<std::string>& args,
			const size_t                    beginIndex
		) {
			std::string result;
			for (size_t i = beginIndex; i < args.size(); ++i) {
				if (!result.empty()) {
					result += ' ';
				}

				result += args[i];
			}

			return result;
		}
	}

	InputSystem::~InputSystem() = default;

	bool InputSystem::Init() {
		ServiceLocator::Register<InputSystem>(this);

		// 起動直後に何も触っていなくてもカーソルが消えるのを防ぐため、デフォルトは「表示・ロック解除」にしておく。
		mMouseCursorLockRequested    = false;
		mMouseCursorLockOverride     = false;
		mMouseCursorLockOverrideMode = false;
		mMouseCursorVisible          = true;

		// --------------------------------------------------------------------
		// コンソールコマンドの登録
		// --------------------------------------------------------------------

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

				const std::string command = JoinArguments(args, 1);

				BindCommand(
					{
						.device = key->device,
						.code   = key->code
					},
					command
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

		mBindContextCommand = std::make_unique<ConCommand>(
			"bindctx",
			[&](const std::vector<std::string>& args) {
				// bindctx [--consume] <context> <key> <command...>
				bool   consumeInput   = false;
				size_t argumentOffset = 0;

				if (!args.empty() && args[0] == "--consume") {
					consumeInput   = true;
					argumentOffset = 1;
				}

				if (args.size() < argumentOffset + 3) {
					Warning(
						kChannel,
						"Usage: bindctx [--consume] <context> <key> <command...>"
					);
					return false;
				}

				const std::string& contextName = args[argumentOffset];

				const auto contextId = mMappingSystem.FindContextIdByName(
					contextName
				);

				if (!contextId.has_value()) {
					Warning(
						kChannel,
						"Input context not found: {}",
						contextName
					);

					return false;
				}

				const auto key = KeyNameTable::FromString(
					args[argumentOffset + 1]
				);

				if (!key.has_value()) {
					Warning(
						kChannel,
						"Invalid key name: {}",
						args[argumentOffset + 1]
					);
					return false;
				}

				const std::string command = JoinArguments(
					args,
					argumentOffset + 2
				);

				if (command.empty()) {
					return false;
				}

				BindContextCommand(*contextId, *key, command, consumeInput);

				DevMsg(
					kChannel,
					"Bound command to context: context = {}, key = {}, command = {}",
					contextName,
					KeyNameTable::ToString(*key),
					command
				);

				return true;
			},
			"Bind a command to a key in an input context. "
			"Usage: bindctx [--consume] <context> <key> <command...>"
		);

		mInputContextCommand = std::make_unique<ConCommand>(
			"inputcontext",
			[&](const std::vector<std::string>& args) {
				// inputcontext [コンテキスト名] [モード]

				// 引数が足りないので失敗
				if (args.size() != 2) {
					Warning(
						kChannel,
						"Usage: inputcontext <name> <pass_through|block_lower>"
					);
					return false;
				}

				const std::string& name = args[0];

				const auto mode = ParseInputContextMode(args[1]);

				// モードが解析不可なので失敗
				if (!mode.has_value()) {
					Warning(
						kChannel,
						"Invalid input context mode: {}",
						args[1]
					);
					return false;
				}

				// 既存のコンテキストならモードだけ更新
				if (
					const auto existing = mMappingSystem.FindContextIdByName(
						name
					);
					existing.has_value()
				) {
					// モードを更新
					if (!mMappingSystem.SetContextMode(*existing, *mode)) {
						Warning(
							kChannel,
							"Failed to set mode for input context: {}",
							name
						);
						return false;
					}

					DevMsg(kChannel, "Input Context Updated: {}", name);

					return true;
				}

				// なければ新規作成
				const InputContextId contextId = mMappingSystem.CreateContext(
					name,
					*mode
				);

				// 作成に失敗
				if (!contextId.IsValid()) {
					Warning(
						kChannel,
						"Failed to create input context: {}",
						name
					);
					return false;
				}

				DevMsg(kChannel, "Input Context Created: {}", name);

				return true;
			},
			"Define an input context."
			"Usage: inputcontext <name> <pass_through|block_lower>"
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
		// TODO: ServiceLocatorは極力使わないように
		auto* console = ServiceLocator::Get<ConsoleSystem>();
		ReleaseAllActiveCommands(console);

		mBindCommand.reset();
		mUnbindCommand.reset();
		mUnbindAllCommand.reset();

		mBindContextCommand.reset();
		mInputContextCommand.reset();

		mToggleLockCursorCommand.reset();
		mToggleCursorLockCommand.reset();
	}

	const std::string_view InputSystem::GetName() const {
		return "InputSystem";
	}

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

		// 非アクティブ時にカーソルが非表示になっている場合、OS 側のカーソル表示状態を強制的に可視に収束させる
		if (!bActive && !bCursorVisible) {
			// ShowCursor の戻り値: 0 以上なら表示、0 未満なら非表示
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
		// すでに同じキーのバインディングが存在するか確認
		const auto it = mCommandBinds.find(key);
		if (it == mCommandBinds.end() || it->second.empty()) {
			return {};
		}

		// コマンドを結合して返す
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
		const std::string& axis, const InputKey& key, const float scale
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

	float InputSystem::Axis1D(const std::string& axis) const {
		return mMappingSystem.Axis1D(axis);
	}

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

	InputContextId InputSystem::CreateInputContext(
		const std::string& name, const INPUT_CONTEXT_MODE mode
	) {
		return mMappingSystem.CreateContext(name, mode);
	}

	bool InputSystem::DestroyInputContext(const InputContextId contextId) {
		return mMappingSystem.DestroyContext(contextId);
	}

	bool InputSystem::PushInputContext(const InputContextId contextId) {
		return mMappingSystem.PushContext(contextId);
	}

	bool InputSystem::PopInputContext(const InputContextId contextId) {
		return mMappingSystem.PopContext(contextId);
	}

	bool InputSystem::IsInputContextActive(
		const InputContextId contextId
	) const {
		return mMappingSystem.IsContextActive(contextId);
	}

	void InputSystem::BindAction(
		const InputContextId contextId, const std::string& action,
		const InputKey&      key, const bool               consumeInput
	) {
		mMappingSystem.BindAction(contextId, action, key, consumeInput);
	}

	void InputSystem::BindAxis1D(
		const InputContextId contextId, const std::string& axis,
		const InputKey&      key,
		const float          scale, const bool consumeInput
	) {
		mMappingSystem.BindAxis1D(contextId, axis, key, scale, consumeInput);
	}

	void InputSystem::BindAxis2D(
		const InputContextId contextId, const std::string& axis,
		const InputKey& key,
		const INPUT_AXIS axisType, const float scale, const bool consumeInput
	) {
		mMappingSystem.BindAxis2D(
			contextId,
			axis,
			key,
			axisType,
			scale,
			consumeInput
		);
	}

	bool InputSystem::IsPressed(
		const InputContextId contextId, const std::string& action
	) const {
		return mMappingSystem.IsPressed(contextId, action);
	}

	bool InputSystem::IsHeld(
		const InputContextId contextId, const std::string& action
	) const {
		return mMappingSystem.IsHeld(contextId, action);
	}

	bool InputSystem::IsReleased(
		const InputContextId contextId, const std::string& action
	) const {
		return mMappingSystem.IsReleased(contextId, action);
	}

	float InputSystem::Axis1D(
		const InputContextId contextId, const std::string& axis
	) const {
		return mMappingSystem.Axis1D(contextId, axis);
	}

	Vec2 InputSystem::Axis2D(
		const InputContextId contextId, const std::string& axis
	) const {
		return mMappingSystem.Axis2D(contextId, axis);
	}

	void InputSystem::HandleExternalAction(
		const InputContextId contextId, const std::string& action,
		const bool           pressed
	) {
		return mMappingSystem.HandleExternalAction(contextId, action, pressed);
	}

	void InputSystem::BindContextCommand(
		const InputContextId contextId, const InputKey& key,
		const std::string&   command, const bool        consumeInput
	) {
		// すでに同じコンテキストIDとキーのバインディングが存在するか確認
		const auto it = std::ranges::find_if(
			mContextCommandBinds,
			[&](const InputContextCommandBinding& binding) {
				return binding.contextId == contextId && binding.key == key;
			}
		);

		// 存在する場合はコマンドを追加
		if (it != mContextCommandBinds.end()) {
			if (!std::ranges::contains(it->commands, command)) {
				it->commands.emplace_back(command);
			}

			// 同じコンテキスト+キーのどれかがConsumeならこのキーは下位コンテキストに渡さない
			it->consumeInput |= consumeInput;

			return;
		}

		// 存在しない場合は新しいバインディングを作成
		InputContextCommandBinding binding = {
			.contextId    = contextId,
			.key          = key,
			.commands     = {},
			.consumeInput = consumeInput
		};

		// コマンドを追加
		binding.commands.emplace_back(command);

		// 新しいバインディングをリストに追加
		mContextCommandBinds.emplace_back(std::move(binding));
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

	void InputSystem::ResetInputStates() {
		DevMsg(
			kChannel,
			"ウィンドウが非アクティブになったため、入力状態をリセットします。"
		);

		// TODO: ServiceLocatorは極力使わないように なんかキモいし。
		auto* console = ServiceLocator::Get<ConsoleSystem>();

		// +commandを解放
		ReleaseAllActiveCommands(console);

		// 物理入力をリセット
		mSnapshot.mCurrentKeyStates.clear();
		mSnapshot.mPreviousKeyStates.clear();
		mSnapshot.mAnalogValues.clear();

		// 論理入力をリセット
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
		/// @param pressCommand 押下コマンド文字列
		/// @return 離脱コマンド文字列（押下コマンドでない場合は空文字列）
		std::optional<std::string> MakeReleaseCommandFromPress(
			const std::string& pressCommand
		) {
			const std::string trimmed = StrUtil::TrimSpaces(pressCommand);
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
		// TODO: ServiceLocatorは極力使わないように なんかキモいし。
		auto* console = ServiceLocator::Get<ConsoleSystem>();
		if (!console) {
			return;
		}

		const auto globalBinds = mCommandBinds;

		// コマンドバインドの処理
		for (const auto& [key, commands] : globalBinds) {
			if (!mSnapshot.WasKeyPressed(key)) {
				continue;
			}

			for (const auto& command : commands) {
				console->ExecuteCommand(
					command,
					EXEC_FLAG::FROM_USER | EXEC_FLAG::SILENT
				);

				RegisterActiveCommand(std::nullopt, key, command);
			}
		}

		// --------------------------------------------------------------------
		// Active Command Release
		// --------------------------------------------------------------------

		ProcessActiveCommandReleases(*console);

		// --------------------------------------------------------------------
		// Context Command Press
		// --------------------------------------------------------------------

		// コマンド実行中のbindctxによる変更から走査を保護する
		const auto contextBinds = mContextCommandBinds;

		const auto contexts = mMappingSystem.GetReceivingContextsTopToBottom();

		std::unordered_set<InputKey> consumedKeys;

		for (const InputContextId contextId : contexts) {
			// コマンド実行によってコンテキストスタックが変化している可能性がある
			if (!mMappingSystem.IsContextReceivingInput(contextId)) {
				continue;
			}

			std::unordered_set<InputKey> consumedByThisContext;

			for (const auto& binding : contextBinds) {
				if (binding.contextId != contextId) {
					continue;
				}

				//-------------------------------------------------------------
				// 上位コンテキストで既にConsumeされている
				//-------------------------------------------------------------

				if (consumedKeys.contains(binding.key)) {
					continue;
				}

				const bool  bIsDown     = mSnapshot.IsKeyDown(binding.key);
				const bool  bIsPressed  = mSnapshot.WasKeyPressed(binding.key);
				const bool  bIsReleased = mSnapshot.WasKeyReleased(binding.key);
				const float analogValue = mSnapshot.GetAnalogValue(binding.key);

				// 入力があるか?
				const bool hasInput =
					bIsDown || bIsPressed || bIsReleased || analogValue != 0.0f;

				if (binding.consumeInput && hasInput) {
					consumedByThisContext.emplace(binding.key);

					mMappingSystem.RegisterConsumedKey(
						contextId,
						binding.key
					);
				}

				// コマンド自体はPress時だけ実行
				if (!bIsPressed) {
					continue;
				}

				for (const auto& command : binding.commands) {
					// 直前のコマンドでコンテキストスタックが変わった可能性がある
					if (
						!mMappingSystem.IsContextReceivingInput(
							binding.contextId
						)
					) {
						break;
					}

					DevMsg(
						kChannel,
						"[D3 TEST] ContextCommand: context={}, key={}, command={}",
						binding.contextId.value,
						KeyNameTable::ToString(binding.key),
						command
					);

					console->ExecuteCommand(
						command,
						EXEC_FLAG::FROM_USER | EXEC_FLAG::SILENT
					);

					RegisterActiveCommand(
						binding.contextId,
						binding.key,
						command
					);
				}
			}

			//-----------------------------------------------------------------
			// このコンテキストを全部処理した後、下位コンテキスト向けにConsumeを有効化
			//-----------------------------------------------------------------

			consumedKeys.insert(
				consumedByThisContext.begin(),
				consumedByThisContext.end()
			);
		}
	}

	void InputSystem::RegisterActiveCommand(
		const std::optional<InputContextId> contextId, const InputKey& key,
		const std::string&                  pressCommand
	) {
		// +command でなければRelease管理は不要
		if (!MakeReleaseCommandFromPress(pressCommand).has_value()) {
			return;
		}

		const bool bAlreadyActive =
			std::ranges::any_of(
				mActiveCommands,
				[&](const ActiveCommand& active) {
					return active.contextId == contextId &&
					       active.key == key &&
					       active.pressCommand == pressCommand;
				}
			);

		if (bAlreadyActive) {
			return;
		}

		mActiveCommands.emplace_back(
			ActiveCommand{
				.contextId = contextId, .key = key, .pressCommand = pressCommand
			}
		);
	}

	void InputSystem::ProcessActiveCommandReleases(ConsoleSystem& console) {
		auto activeCommands = std::move(mActiveCommands);

		mActiveCommands.clear();

		for (auto& active : activeCommands) {
			const bool bKeyReleased = mSnapshot.WasKeyReleased(active.key);

			bool bContextBlocked = false;

			if (active.contextId.has_value()) {
				bContextBlocked = !mMappingSystem.IsContextReceivingInput(
					*active.contextId
				);
			}

			if (!bKeyReleased && !bContextBlocked) {
				mActiveCommands.emplace_back(std::move(active));
				continue;
			}

			const auto releaseCommand = MakeReleaseCommandFromPress(
				active.pressCommand
			);

			if (!releaseCommand.has_value()) {
				continue;
			}

			console.ExecuteCommand(
				*releaseCommand,
				EXEC_FLAG::FROM_USER | EXEC_FLAG::SILENT
			);
		}
	}

	void InputSystem::ReleaseAllActiveCommands(ConsoleSystem* console) {
		const auto activeCommands = std::move(mActiveCommands);

		mActiveCommands.clear();

		if (!console) {
			return;
		}

		for (const auto& active : activeCommands) {
			const auto releaseCommand = MakeReleaseCommandFromPress(
				active.pressCommand
			);

			if (!releaseCommand.has_value()) {
				continue;
			}

			DevMsg(
				kChannel,
				"[TEST] Forced Release: key={}, command={}",
				KeyNameTable::ToString(active.key),
				*releaseCommand
			);

			console->ExecuteCommand(
				*releaseCommand,
				EXEC_FLAG::FROM_USER | EXEC_FLAG::SILENT
			);
		}
	}
}
