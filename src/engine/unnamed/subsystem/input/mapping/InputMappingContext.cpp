#include "pch.h"
#include "InputMappingContext.h"

#include <ranges>

#include "engine/unnamed/subsystem/input/KeyNameTable.h"

namespace Unnamed {
	static constexpr std::string_view kChannel = "InputMapping";

	void InputMappingContext::BeginFrame() {
		// 入力状態をリセット
		for (auto& state : mActionStates | std::views::values) {
			state.bIsPressed  = false;
			state.bIsHeld     = false;
			state.bIsReleased = false;
		}

		// External ActionのHeldだけ復元
		for (const auto& action : mExternalHeldActions) {
			mActionStates[action].bIsHeld = true;
		}

		// 
		for (auto& [value] : mAxisStates1D | std::views::values) {
			value = 0.0f;
		}

		for (auto& [value] : mAxisStates2D | std::views::values) {
			value = Vec2::zero;
		}
	}

	void InputMappingContext::Evaluate(
		const InputSnapshot&          snapshot,
		std::unordered_set<InputKey>& consumedKeys
	) {
		std::unordered_set<InputKey> consumedByThisContext;

		// アクション状態を一時的に保存するマップ
		std::unordered_map<std::string, std::tuple<bool, bool, bool>>
			tempActionStates;

		// <axis名, <正の値, 負の値>>
		std::unordered_map<std::string, std::pair<float, float>> tempAxis1D;
		// <axis名, <正のVec2, 負のVec2>>
		std::unordered_map<std::string, std::pair<Vec2, Vec2>>   tempAxis2D;

		// すべての入力を処理して一時マップに値を蓄積
		for (
			const auto& [
				key,
				type,
				target,
				scale,
				scale2D,
				axis,
				consumeInput
			] : mBindings
		) {
			if (consumedKeys.contains(key)) {
				continue; // このキーはすでに上位コンテキストで使用されている
			}

			const bool  bIsDown     = snapshot.IsKeyDown(key);
			const bool  bIsPressed  = snapshot.WasKeyPressed(key);
			const bool  bIsReleased = snapshot.WasKeyReleased(key);
			const float analogValue = snapshot.GetAnalogValue(key);

			if (type == BINDING_TYPE::ACTION) {
				// このキーの状態を計算
				const bool bIsHeld = bIsDown;

				// 状態を論理ORで更新
				auto& [existing_pressed, existing_held, existing_released] =
					tempActionStates[target];

				existing_pressed  = existing_pressed || bIsPressed;
				existing_held     = existing_held || bIsHeld;
				existing_released = existing_released || bIsReleased;
			}

			if (type == BINDING_TYPE::AXIS_1D) {
				float scaledValue          = analogValue * scale;
				auto& [positive, negative] = tempAxis1D[target];

				if (scaledValue > 0) {
					positive = std::max(positive, scaledValue);
				} else if (scaledValue < 0) {
					negative = std::min(negative, scaledValue);
				}
			}

			if (type == BINDING_TYPE::AXIS_2D) {
				auto& [positive, negative] = tempAxis2D[target];

				if (axis == INPUT_AXIS::X) {
					float scaledValue = analogValue * scale2D.x;
					if (scaledValue > 0) {
						positive.x = std::max(positive.x, scaledValue);
					} else if (scaledValue < 0) {
						negative.x = std::min(negative.x, scaledValue);
					}
				} else {
					float scaledValue = analogValue * scale2D.y;
					if (scaledValue > 0) {
						positive.y = std::max(positive.y, scaledValue);
					} else if (scaledValue < 0) {
						negative.y = std::min(negative.y, scaledValue);
					}
				}
			}

			const bool hasInput =
				bIsDown ||
				bIsPressed ||
				bIsReleased ||
				analogValue != 0.0f;

			if (consumeInput && hasInput) {
				consumedByThisContext.insert(key);
			}
		}

		consumedKeys.insert(
			consumedByThisContext.begin(),
			consumedByThisContext.end()
		);

		// 一時マップから最終アクション状態を更新（デバイス入力分）
		for (const auto& [action, states] : tempActionStates) {
			mActionStates[action].bIsPressed =
				mActionStates[action].bIsPressed || std::get<0>(states);
			mActionStates[action].bIsHeld =
				mActionStates[action].bIsHeld || std::get<1>(states);
			mActionStates[action].bIsReleased =
				mActionStates[action].bIsReleased || std::get<2>(states);
		}

		// 一時マップから最終的な値を計算
		for (const auto& [axisName, values] : tempAxis1D) {
			auto& [value] = mAxisStates1D[axisName];
			value         = values.first + values.second; // 正の値と負の値を合計
		}

		for (const auto& [axisName, values] : tempAxis2D) {
			auto& [value] = mAxisStates2D[axisName];
			value.x       = values.first.x + values.second.x;
			value.y       = values.first.y + values.second.y;
		}
	}

	void InputMappingContext::BindAction(
		const std::string& action, const InputKey& key, const bool consumeInput
	) {
		InputBinding binding;
		binding.key          = key;
		binding.type         = BINDING_TYPE::ACTION;
		binding.target       = action;
		binding.consumeInput = consumeInput;
		mBindings.emplace_back(binding);
		DevMsg(
			kChannel,
			"アクションをバインドしました: {}, key = {}",
			action,
			KeyNameTable::ToString(key)
		);
	}

	void InputMappingContext::BindAxis1D(
		const std::string& axis,
		const InputKey&    key,
		float              scale,
		const bool         consumeInput
	) {
		InputBinding binding;
		binding.key          = key;
		binding.type         = BINDING_TYPE::AXIS_1D;
		binding.target       = axis;
		binding.scale        = scale;
		binding.consumeInput = consumeInput;
		mBindings.emplace_back(binding);
		DevMsg(
			kChannel,
			"1D軸をバインドしました: {}, key = {}, scale = {}",
			axis,
			KeyNameTable::ToString(key),
			scale
		);
	}

	void InputMappingContext::BindAxis2D(
		const std::string& axis,
		const InputKey&    key,
		INPUT_AXIS         axisType,
		float              scale,
		const bool         consumeInput
	) {
		InputBinding binding;
		binding.key    = key;
		binding.type   = BINDING_TYPE::AXIS_2D;
		binding.target = axis;
		if (axisType == INPUT_AXIS::X) {
			binding.scale2D.x = scale;
			binding.scale2D.y = 0.0f; // Y軸は無視される
		} else {
			binding.scale2D.x = 0.0f; // X軸は無視される
			binding.scale2D.y = scale;
		}
		binding.axis         = axisType;
		binding.consumeInput = consumeInput;
		mBindings.emplace_back(binding);
		DevMsg(
			kChannel,
			"2D軸をバインドしました: {}, key = {}, axis = {}, scale = {}",
			axis,
			KeyNameTable::ToString(key),
			static_cast<int>(axisType),
			scale
		);
	}

	void InputMappingContext::Unbind(const std::string& target) {
		std::erase_if(
			mBindings,
			[&target](const InputBinding& binding) {
				return binding.target == target;
			}
		);
	}

	void InputMappingContext::HandleExternalAction(
		const std::string& action, const bool pressed
	) {
		auto& state = mActionStates[action];

		if (pressed) {
			mExternalHeldActions.insert(action);

			state.bIsPressed = true;
			state.bIsHeld    = true;
		} else {
			mExternalHeldActions.erase(action);

			state.bIsReleased = true;
			state.bIsHeld     = false;
		}
	}

	bool InputMappingContext::IsPressed(const std::string& action) const {
		const auto it = mActionStates.find(action);
		if (it == mActionStates.end()) {
			return false;
		}
		return it->second.bIsPressed;
	}

	bool InputMappingContext::IsHeld(const std::string& action) const {
		const auto it = mActionStates.find(action);
		if (it == mActionStates.end()) {
			return false;
		}
		return it->second.bIsHeld;
	}

	bool InputMappingContext::IsReleased(const std::string& action) const {
		const auto it = mActionStates.find(action);
		if (it == mActionStates.end()) {
			return false;
		}
		return it->second.bIsReleased;
	}

	float InputMappingContext::Axis1D(const std::string& axis) const {
		const auto it = mAxisStates1D.find(axis);

		if (it == mAxisStates1D.end()) {
			return 0.0f;
		}

		return it->second.value;
	}

	Vec2 InputMappingContext::Axis2D(const std::string& axis) const {
		const auto it = mAxisStates2D.find(axis);
		if (it == mAxisStates2D.end()) {
			return Vec2::zero;
		}
		return it->second.value;
	}

	void InputMappingContext::ResetStates() {
		// 外部ホールドアクションをリセット
		mExternalHeldActions.clear();

		// アクションの状態をリセット
		for (auto& state : mActionStates | std::views::values) {
			state.bIsPressed  = false;
			state.bIsHeld     = false;
			state.bIsReleased = false;
		}

		// アナログ値のリセット
		for (auto& state : mAxisStates1D | std::views::values) {
			state.value = 0.0f;
		}

		for (auto& state : mAxisStates2D | std::views::values) {
			state.value = Vec2::zero;
		}
	}
}
