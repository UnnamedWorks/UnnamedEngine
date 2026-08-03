#include "pch.h"
#include "UiRoot.h"

#include "../utils/UiUtils.h"

namespace Unnamed {
	namespace {
		[[nodiscard]] bool IsValidViewportSize(const Vec2& size) noexcept {
			return size.IsValid() && size.x >= 0.0f && size.y >= 0.0f;
		}

		[[nodiscard]] Vec2 SanitizeViewportSize(Vec2 size) noexcept {
			constexpr uint32_t vec2ElementCount = 2;
			for (uint32_t i = 0; i < vec2ElementCount; ++i) {
				if (!std::isfinite(size[i]) || size[i] < 0.0f) {
					size[i] = 0.0f;
				}
			}

			return size;
		}
	}

	void UiRoot::SetViewportSize(const Vec2& viewportSize) {
		// Viewportサイズは有限かつ非負である必要があります。
		UASSERT(
			IsValidViewportSize(viewportSize) &&
			"UI viewport size must be finite and non-negative."
		);

		const Vec2 sanitizedViewportSize = SanitizeViewportSize(viewportSize);

		// 既に設定済みのサイズと同じ場合は何もしません。
		if (
			mHasViewportSize &&
			AreUiVectorsExactlyEqual(
				mViewportSize,
				sanitizedViewportSize
			)
		) {
			return;
		}

		mViewportSize = sanitizedViewportSize;

		mHasViewportSize = true;

		// Viewportサイズ変更によって、RootとMeasure制約と最終配置サイズが変わる
		Invalidate(UI_INVALIDATION::MEASURE);
	}

	const Vec2& UiRoot::GetViewportSize() const noexcept {
		return mViewportSize;
	}

	bool UiRoot::HasViewportSize() const noexcept {
		return mHasViewportSize;
	}

	bool UiRoot::IsLayoutInvalidated() const noexcept {
		return IsInvalidated(
			UI_INVALIDATION::MEASURE | UI_INVALIDATION::ARRANGE
		);
	}

	UiLayoutPassResult UiRoot::PerformLayout() {
		// Viewportサイズが設定されていない場合は、レイアウトを実行できません。
		UASSERT(
			mHasViewportSize &&
			"SetViewportSize() must be called before PerformLayout()."
		);

		// Viewportサイズが設定されていない場合は、レイアウトを実行できません。
		if (!mHasViewportSize) {
			return UiLayoutPassResult{
				.passCount       = 0,
				.stabilized      = false,
				.performedLayout = false
			};
		}

		// 再帰的な PerformLayout() 呼び出しは許可されません。
		UASSERT(
			!mIsPerformingLayout &&
			"Recursive UiRoot::PerformLayout() call detected."
		);

		if (mIsPerformingLayout) {
			return UiLayoutPassResult{
				.passCount       = 0,
				.stabilized      = false,
				.performedLayout = false
			};
		}

		// MeasureまたはArrangeが必要ない場合は、レイアウトを実行しません。
		if (!IsLayoutInvalidated()) {
			return UiLayoutPassResult{
				.passCount       = 0,
				.stabilized      = true,
				.performedLayout = false
			};
		}

		mIsPerformingLayout = true;

		UiLayoutPassResult result{
			.passCount       = 0,
			.stabilized      = false,
			.performedLayout = true
		};

		const UiRect rootRect{
			.pos  = Vec2::zero,
			.size = mViewportSize
		};

		// MeasureとArrangeを繰り返し、レイアウトが安定するまで実行します。
		for (uint32_t passIndex = 0; passIndex < kMaxLayoutPassCount; ++
		     passIndex) {
			++result.passCount;

			// MeasureがDirtyでない場合は内部キャッシュが使われる。Viewportサイズが変化していれば制約変更として再計算される。
			static_cast<void>(Measure(mViewportSize));

			// Rootは常にViewport全体へ配置する。
			Arrange(rootRect);

			if (!IsLayoutInvalidated()) {
				result.stabilized = true;
				break;
			}
		}

		mIsPerformingLayout = false;

		// レイアウトが安定しなかった場合は警告を出力します。
		if (!result.stabilized) {
			Warning(
				"UI",
				"UI layout did not stabilize after {} passes.",
				kMaxLayoutPassCount
			);
		}
		return result;
	}
}
