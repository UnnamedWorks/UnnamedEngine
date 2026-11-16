#include "pch.h"
#include "InputMappingSystem.h"

#include <algorithm>
#include <ranges>
#include <utility>

namespace Unnamed {
	InputMappingSystem::InputMappingSystem() {
		mDefaultContextId = CreateContext("Default");
		PushContext(mDefaultContextId);
	}

	void InputMappingSystem::BeginFrame() {
		mExternalConsumedKeys.clear();

		for (const InputContextId contextId : mContextStack) {
			// ReSharper disable once CppLocalVariableMayBeConst
			auto* entry = FindContext(contextId);

			if (!entry) {
				continue;
			}

			entry->context->BeginFrame();
		}
	}

	void InputMappingSystem::Evaluate(const InputSnapshot& snapshot) {
		std::unordered_set<InputKey> consumedKeys;

		for (
			auto it = mContextStack.rbegin();
			it != mContextStack.rend();
			++it
		) {
			const auto* entry = FindContext(*it);

			if (!entry) {
				continue;
			}

			entry->context->Evaluate(snapshot, consumedKeys);

			//-----------------------------------------------------------------
			// bindctx等によってこのコンテキストでConsumeされたキーを下位コンテキスト向けに追加 
			//-----------------------------------------------------------------

			const auto externalIt = std::ranges::find_if(
				mExternalConsumedKeys,
				[&](const ExternalConsumedContext& consumed) {
					return consumed.contextId == entry->id;
				}
			);

			if (
				externalIt != mExternalConsumedKeys.end()
			) {
				consumedKeys.insert(
					externalIt->keys.begin(),
					externalIt->keys.end()
				);
			}

			// ブロッキング
			if (entry->mode == INPUT_CONTEXT_MODE::BLOCK_LOWER) {
				break;
			}
		}
	}

	// ReSharper disable once CppMemberFunctionMayBeConst
	void InputMappingSystem::ResetStates() {
		for (const auto& entry : mContexts) {
			entry.context->ResetStates();
		}
	}

	InputContextId InputMappingSystem::CreateContext(
		const std::string& name, const INPUT_CONTEXT_MODE mode
	) {
		// 名前が空の場合は作成しない
		if (name.empty()) {
			return {};
		}

		// 同名のコンテキストが存在する場合は作成しない
		if (FindContextIdByName(name).has_value()) {
			return {};
		}

		const InputContextId id = {
			.value = mNextContextId++
		};

		ContextEntry entry = {
			.id      = id,
			.name    = name,
			.context = std::make_unique<InputMappingContext>(),
			.mode    = mode
		};

		mContexts.emplace_back(std::move(entry));

		return id;
	}

	bool InputMappingSystem::DestroyContext(InputContextId contextId) {
		if (contextId == mDefaultContextId) {
			return false;
		}

		std::erase(mContextStack, contextId);

		const auto oldSize = mContexts.size();

		std::erase_if(
			mContexts,
			[contextId](const ContextEntry& entry) {
				return entry.id == contextId;
			}
		);

		return oldSize != mContexts.size();
	}

	bool InputMappingSystem::PushContext(InputContextId contextId) {
		if (!FindContext(contextId)) {
			return false;
		}

		std::erase(mContextStack, contextId);

		mContextStack.emplace_back(contextId);

		return true;
	}

	bool InputMappingSystem::PopContext(const InputContextId contextId) {
		// 移行期間中はDefaultを残す
		if (contextId == mDefaultContextId) {
			return false;
		}

		const auto oldSize = mContextStack.size();

		std::erase(mContextStack, contextId);

		if (oldSize == mContextStack.size()) {
			return false;
		}

		if (const auto* entry = FindContext(contextId)) {
			// 同期Releaseが元の入力ソースを解除するまで保持
			entry->context->ResetStates(true);
		}

		return true;
	}

	bool InputMappingSystem::IsContextActive(
		const InputContextId contextId
	) const {
		return std::ranges::find(mContextStack, contextId) !=
		       mContextStack.end();
	}

	bool InputMappingSystem::IsContextReceivingInput(
		const InputContextId contextId
	) const {
		bool foundTarget = false;

		for (
			auto it = mContextStack.rbegin();
			it != mContextStack.rend();
			++it
		) {
			const auto* entry = FindContext(*it);

			if (!entry) {
				continue;
			}

			if (entry->id == contextId) {
				foundTarget = true;
				break;
			}

			if (entry->mode == INPUT_CONTEXT_MODE::BLOCK_LOWER) {
				return false;
			}
		}

		return foundTarget;
	}

	InputContextId InputMappingSystem::GetDefaultContextId() const {
		return mDefaultContextId;
	}

	bool InputMappingSystem::SetContextMode(
		const InputContextId contextId, const INPUT_CONTEXT_MODE mode
	) {
		auto* entry = FindContext(contextId);

		if (!entry) {
			return false;
		}

		entry->mode = mode;
		return true;
	}

	INPUT_CONTEXT_MODE InputMappingSystem::GetContextMode(
		const InputContextId contextId
	) const {
		const auto* entry = FindContext(contextId);

		if (!entry) {
			return INPUT_CONTEXT_MODE::PASS_THROUGH;
		}

		return entry->mode;
	}

	std::optional<InputContextId> InputMappingSystem::FindContextIdByName(
		std::string_view name
	) const {
		const auto it = std::ranges::find_if(
			mContexts,
			[name](const ContextEntry& entry) {
				return entry.name == name;
			}
		);

		if (it == mContexts.end()) {
			return std::nullopt;
		}

		return it->id;
	}

	void InputMappingSystem::BindAction(
		const InputContextId contextId, const std::string& action,
		const InputKey&      key, const bool               consumeInput
	) {
		// ReSharper disable once CppLocalVariableMayBeConst
		auto* entry = FindContext(contextId);

		if (!entry) {
			return;
		}

		entry->context->BindAction(action, key, consumeInput);
	}

	void InputMappingSystem::BindAxis1D(
		const InputContextId contextId, const std::string& axis,
		const InputKey&      key, const float              scale,
		const bool           consumeInput
	) {
		// ReSharper disable once CppLocalVariableMayBeConst
		auto* entry = FindContext(contextId);

		if (!entry) {
			return;
		}

		entry->context->BindAxis1D(axis, key, scale, consumeInput);
	}

	void InputMappingSystem::BindAxis2D(
		const InputContextId contextId, const std::string& axis,
		const InputKey&      key, const INPUT_AXIS         axisType,
		const float          scale, const bool             consumeInput
	) {
		// ReSharper disable once CppLocalVariableMayBeConst
		auto* entry = FindContext(contextId);

		if (!entry) {
			return;
		}

		entry->context->BindAxis2D(
			axis,
			key,
			axisType,
			scale,
			consumeInput
		);
	}

	void InputMappingSystem::Unbind(
		const InputContextId contextId, const std::string& target
	) {
		// ReSharper disable once CppLocalVariableMayBeConst
		auto* entry = FindContext(contextId);

		if (!entry) {
			return;
		}

		entry->context->Unbind(target);
	}

	bool InputMappingSystem::IsPressed(
		const InputContextId contextId, const std::string& action
	) const {
		const auto* entry = FindContext(contextId);

		if (!entry || !IsContextReceivingInput(contextId)) {
			return false;
		}

		return entry->context->IsPressed(action);
	}

	bool InputMappingSystem::IsHeld(
		const InputContextId contextId, const std::string& action
	) const {
		const auto* entry = FindContext(contextId);

		if (!entry || !IsContextReceivingInput(contextId)) {
			return false;
		}

		return entry->context->IsHeld(action);
	}

	bool InputMappingSystem::IsReleased(
		const InputContextId contextId, const std::string& action
	) const {
		const auto* entry = FindContext(contextId);

		if (!entry || !IsContextReceivingInput(contextId)) {
			return false;
		}

		return entry->context->IsReleased(action);
	}

	float InputMappingSystem::Axis1D(
		const InputContextId contextId, const std::string& axis
	) const {
		const auto* entry = FindContext(contextId);

		if (!entry || !IsContextReceivingInput(contextId)) {
			return 0.0f;
		}

		return entry->context->Axis1D(axis);
	}

	Vec2 InputMappingSystem::Axis2D(
		const InputContextId contextId, const std::string& axis
	) const {
		const auto* entry = FindContext(contextId);

		if (!entry || !IsContextReceivingInput(contextId)) {
			return Vec2::zero;
		}

		return entry->context->Axis2D(axis);
	}

	void InputMappingSystem::HandleExternalAction(
		const InputContextId contextId, const std::string& action,
		const bool           pressed, const uint64_t sourceId
	) {
		// ReSharper disable once CppLocalVariableMayBeConst
		auto* entry = FindContext(contextId);

		if (!entry) {
			return;
		}

		// Pressはコンテキストによって制限する。Releaseは既存入力状態を解除するために通す。
		if (
			pressed &&
			!IsContextReceivingInput(contextId)
		) {
			return;
		}

		entry->context->HandleExternalAction(action, pressed, sourceId);
	}

	void InputMappingSystem::RegisterConsumedKey(
		InputContextId contextId, const InputKey& key
	) {
		const auto it = std::ranges::find_if(
			mExternalConsumedKeys,
			[contextId](const ExternalConsumedContext& entry) {
				return entry.contextId == contextId;
			}
		);

		if (it != mExternalConsumedKeys.end()) {
			it->keys.emplace(key);
			return;
		}

		ExternalConsumedContext entry = {
			.contextId = contextId
		};

		entry.keys.emplace(key);

		mExternalConsumedKeys.emplace_back(std::move(entry));
	}

	std::vector<InputContextId> InputMappingSystem::
	GetReceivingContextsTopToBottom() const {
		std::vector<InputContextId> result;

		for (
			auto it = mContextStack.rbegin(); it != mContextStack.rend(); ++it
		) {
			const auto* entry = FindContext(*it);

			if (!entry) {
				continue;
			}

			result.emplace_back(entry->id);

			if (entry->mode == INPUT_CONTEXT_MODE::BLOCK_LOWER) {
				break;
			}
		}

		return result;
	}

	void InputMappingSystem::BindAction(
		const std::string& action, const InputKey& key
	) {
		BindAction(mDefaultContextId, action, key);
	}

	void InputMappingSystem::BindAxis1D(
		const std::string& axis, const InputKey& key, const float scale
	) {
		BindAxis1D(mDefaultContextId, axis, key, scale);
	}

	void InputMappingSystem::BindAxis2D(
		const std::string& axis, const InputKey& key, const INPUT_AXIS axisType,
		const float        scale
	) {
		BindAxis2D(mDefaultContextId, axis, key, axisType, scale);
	}

	void InputMappingSystem::Unbind(const std::string& target) {
		Unbind(mDefaultContextId, target);
	}

	bool InputMappingSystem::IsPressed(const std::string& action) const {
		return IsPressed(mDefaultContextId, action);
	}

	bool InputMappingSystem::IsHeld(const std::string& action) const {
		return IsHeld(mDefaultContextId, action);
	}

	bool InputMappingSystem::IsReleased(const std::string& action) const {
		return IsReleased(mDefaultContextId, action);
	}

	float InputMappingSystem::Axis1D(const std::string& axis) const {
		return Axis1D(mDefaultContextId, axis);
	}

	Vec2 InputMappingSystem::Axis2D(const std::string& axis) const {
		return Axis2D(mDefaultContextId, axis);
	}

	void InputMappingSystem::HandleExternalAction(
		const std::string& action, const bool pressed
	) {
		HandleExternalAction(mDefaultContextId, action, pressed);
	}

	InputMappingSystem::ContextEntry* InputMappingSystem::FindContext(
		InputContextId contextId
	) {
		const auto it = std::ranges::find_if(
			mContexts,
			[contextId](const ContextEntry& entry) {
				return entry.id == contextId;
			}
		);

		if (it == mContexts.end()) {
			return nullptr;
		}

		return &*it;
	}

	const InputMappingSystem::ContextEntry* InputMappingSystem::FindContext(
		InputContextId contextId
	) const {
		const auto it = std::ranges::find_if(
			mContexts,
			[contextId](const ContextEntry& entry) {
				return entry.id == contextId;
			}
		);

		if (it == mContexts.end()) {
			return nullptr;
		}

		return &*it;
	}
}
