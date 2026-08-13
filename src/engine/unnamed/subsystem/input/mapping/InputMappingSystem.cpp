#include "pch.h"
#include "InputMappingSystem.h"

namespace Unnamed {
	void InputMappingSystem::BeginFrame() {
		mDefaultContext.BeginFrame();
	}

	void InputMappingSystem::Evaluate(const InputSnapshot& snapshot) {
		mDefaultContext.Evaluate(snapshot);
	}

	void InputMappingSystem::BindAction(
		const std::string& action, const InputKey& key
	) {
		mDefaultContext.BindAction(action, key);
	}

	void InputMappingSystem::BindAxis1D(
		const std::string& axis, const InputKey& key, const float scale
	) {
		mDefaultContext.BindAxis1D(axis, key, scale);
	}

	void InputMappingSystem::BindAxis2D(
		const std::string& axis, const InputKey& key, const INPUT_AXIS axisType,
		const float              scale
	) {
		mDefaultContext.BindAxis2D(axis, key, axisType, scale);
	}

	void InputMappingSystem::Unbind(const std::string& target) {
		mDefaultContext.Unbind(target);
	}

	void InputMappingSystem::HandleExternalAction(
		const std::string& action, const bool pressed
	) {
		mDefaultContext.HandleExternalAction(action, pressed);
	}

	bool InputMappingSystem::IsPressed(const std::string& action) const {
		return mDefaultContext.IsPressed(action);
	}

	bool InputMappingSystem::IsHeld(const std::string& action) const {
		return mDefaultContext.IsHeld(action);
	}

	bool InputMappingSystem::IsReleased(const std::string& action) const {
		return mDefaultContext.IsReleased(action);
	}

	float InputMappingSystem::Axis1D(const std::string& axis) const {
		return mDefaultContext.Axis1D(axis);
	}

	Vec2 InputMappingSystem::Axis2D(const std::string& axis) const {
		return mDefaultContext.Axis2D(axis);
	}

	void InputMappingSystem::ResetStates() {
		mDefaultContext.ResetStates();
	}
}
