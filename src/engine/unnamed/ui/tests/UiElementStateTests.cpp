#include "pch.h"
#include "UiElementStateTests.h"

#include "../core/UiElement.h"

namespace Unnamed {
	namespace {
		class TestStateElement final : public UiElement {
		public:
			[[nodiscard]] int GetVisibilityChangedCount() const noexcept {
				return mVisibilityChangedCount;
			}

			[[nodiscard]] int GetEnabledChangedCount() const noexcept {
				return mEnabledChangedCount;
			}

			[[nodiscard]] int GetHitTestChangedCount() const noexcept {
				return mHitTestChangedCount;
			}

			[[nodiscard]] int GetOpacityChangedCount() const noexcept {
				return mOpacityChangedCount;
			}

			[[nodiscard]] UI_VISIBILITY GetOldVisibility() const noexcept {
				return mOldVisibility;
			}

			[[nodiscard]] UI_VISIBILITY GetNewVisibility() const noexcept {
				return mNewVisibility;
			}

			[[nodiscard]] bool GetOldEnabled() const noexcept {
				return mOldEnabled;
			}

			[[nodiscard]] bool GetNewEnabled() const noexcept {
				return mNewEnabled;
			}

			[[nodiscard]] bool GetOldHitTestVisible() const noexcept {
				return mOldHitTestVisible;
			}

			[[nodiscard]] bool GetNewHitTestVisible() const noexcept {
				return mNewHitTestVisible;
			}

			[[nodiscard]] float GetOldOpacity() const noexcept {
				return mOldOpacity;
			}

			[[nodiscard]] float GetNewOpacity() const noexcept {
				return mNewOpacity;
			}

		protected:
			void OnVisibilityChanged(
				const UI_VISIBILITY oldVisibility,
				const UI_VISIBILITY newVisibility
			) override {
				++mVisibilityChangedCount;

				mOldVisibility = oldVisibility;
				mNewVisibility = newVisibility;
			}

			void OnEnabledChanged(
				const bool oldEnabled, const bool newEnabled
			) override {
				++mEnabledChangedCount;

				mOldEnabled = oldEnabled;
				mNewEnabled = newEnabled;
			}

			void OnHitTestVisibleChanged(
				const bool oldHitTestVisible, const bool newHitTestVisible
			) override {
				++mHitTestChangedCount;

				mOldHitTestVisible =
					oldHitTestVisible;

				mNewHitTestVisible =
					newHitTestVisible;
			}

			void OnOpacityChanged(
				const float oldOpacity, const float newOpacity
			) override {
				++mOpacityChangedCount;

				mOldOpacity = oldOpacity;
				mNewOpacity = newOpacity;
			}

		private:
			int mVisibilityChangedCount = 0;
			int mEnabledChangedCount    = 0;
			int mHitTestChangedCount    = 0;
			int mOpacityChangedCount    = 0;

			float mOldOpacity = 1.0f;
			float mNewOpacity = 1.0f;

			UI_VISIBILITY mOldVisibility = UI_VISIBILITY::VISIBLE;

			UI_VISIBILITY mNewVisibility = UI_VISIBILITY::VISIBLE;

			bool mOldEnabled = true;
			bool mNewEnabled = true;

			bool mOldHitTestVisible = true;
			bool mNewHitTestVisible = true;
		};

		void TestDefaultState() {
			const TestStateElement element;

			UASSERT(element.GetVisibility() == UI_VISIBILITY::VISIBLE);

			UASSERT(element.IsEnabled());
			UASSERT(element.IsHitTestVisible());
			UASSERT(element.GetOpacity() == 1.0f);
		}

		void TestSettingSameStateDoesNothing() {
			TestStateElement element;

			element.SetVisibility(UI_VISIBILITY::VISIBLE);

			element.SetEnabled(true);
			element.SetHitTestVisible(true);
			element.SetOpacity(1.0f);

			UASSERT(element.GetVisibilityChangedCount() == 0);

			UASSERT(element.GetEnabledChangedCount() == 0);

			UASSERT(element.GetHitTestChangedCount() == 0);

			UASSERT(element.GetOpacityChangedCount() == 0);
		}

		void TestVisibilityState() {
			TestStateElement element;

			element.SetVisibility(UI_VISIBILITY::HIDDEN);

			UASSERT(element.GetVisibility() == UI_VISIBILITY::HIDDEN);

			UASSERT(element.GetVisibilityChangedCount() == 1);

			UASSERT(element.GetOldVisibility() == UI_VISIBILITY::VISIBLE);

			UASSERT(element.GetNewVisibility() == UI_VISIBILITY::HIDDEN);

			element.SetVisibility(UI_VISIBILITY::COLLAPSED);

			UASSERT(element.GetVisibility() == UI_VISIBILITY::COLLAPSED);

			UASSERT(element.GetVisibilityChangedCount() == 2);

			UASSERT(element.GetOldVisibility() == UI_VISIBILITY::HIDDEN);

			UASSERT(element.GetNewVisibility() == UI_VISIBILITY::COLLAPSED);
		}

		void TestEnabledState() {
			TestStateElement element;

			element.SetEnabled(false);

			UASSERT(!element.IsEnabled());

			UASSERT(element.GetEnabledChangedCount() == 1);

			UASSERT(element.GetOldEnabled());
			UASSERT(!element.GetNewEnabled());

			element.SetEnabled(false);

			UASSERT(element.GetEnabledChangedCount() == 1);

			element.SetEnabled(true);

			UASSERT(element.IsEnabled());

			UASSERT(element.GetEnabledChangedCount() == 2);
		}

		void TestHitTestState() {
			TestStateElement element;

			element.SetHitTestVisible(false);

			UASSERT(!element.IsHitTestVisible());

			UASSERT(element.GetHitTestChangedCount() == 1);

			UASSERT(element.GetOldHitTestVisible());

			UASSERT(!element.GetNewHitTestVisible());

			element.SetHitTestVisible(false);

			UASSERT(element.GetHitTestChangedCount() == 1);
		}

		void TestOpacityState() {
			TestStateElement element;

			element.SetOpacity(0.5f);

			UASSERT(element.GetOpacity() == 0.5f);

			UASSERT(element.GetOpacityChangedCount() == 1);

			UASSERT(element.GetOldOpacity() == 1.0f);

			UASSERT(element.GetNewOpacity() == 0.5f);

			element.SetOpacity(0.5f);

			UASSERT(element.GetOpacityChangedCount() == 1);

			element.SetOpacity(-10.0f);

			UASSERT(element.GetOpacity() == 0.0f);

			UASSERT(element.GetOpacityChangedCount() == 2);

			element.SetOpacity(10.0f);

			UASSERT(element.GetOpacity() == 1.0f);

			UASSERT(element.GetOpacityChangedCount() == 3);
		}
	}

	void RunUiElementStateTests() {
		TestDefaultState();
		TestSettingSameStateDoesNothing();
		TestVisibilityState();
		TestEnabledState();
		TestHitTestState();
		TestOpacityState();
	}
} // namespace Unnamed
