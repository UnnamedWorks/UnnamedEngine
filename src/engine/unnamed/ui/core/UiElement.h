#pragma once
#include "UiInvalidation.h"
#include "UiVisibility.h"

#include "../geometry/UiGeometry.h"

namespace Unnamed {
	class UiContainer;
	class UiTree;

	struct UiUpdateContext;

	class UiElement {
	public:
		// コンストラクタ・デストラクタ
		UiElement()          = default;
		virtual ~UiElement() = default;

		// コピー・ムーブ禁止
		UiElement(const UiElement&)            = delete;
		UiElement& operator=(const UiElement&) = delete;
		UiElement(UiElement&&)                 = delete;
		UiElement& operator=(UiElement&&)      = delete;

		/// @brief 親要素を取得します。
		/// @return 親要素のポインタ。親要素が存在しない場合は nullptr
		/// を返します。
		[[nodiscard]] UiContainer* GetParent() noexcept;

		/// @brief 親要素を取得します。(const 版)
		/// @return 親要素のポインタ。親要素が存在しない場合は nullptr
		/// を返します。
		[[nodiscard]] const UiContainer* GetParent() const noexcept;

		/// @brief 親要素が存在するかどうかを判定します。
		/// @return 親要素が存在する場合は true、存在しない場合は false
		/// を返します。
		[[nodiscard]] bool HasParent() const noexcept;

		/// @brief 指定された要素がこの要素の祖先であるかどうかを判定します。
		/// @details 循環参照防止に使用します。
		/// @param element 判定対象の要素
		/// @return 指定された要素がこの要素の祖先である場合は
		/// true、そうでない場合は falseを返します。
		[[nodiscard]]
		bool IsAncestorOf(const UiElement& element) const noexcept;

		/// @brief UI要素の表示状態を設定します。
		/// @param visibility 設定する表示状態。
		void SetVisibility(UI_VISIBILITY visibility);

		/// @brief UI要素の表示状態を取得します。
		/// @return 現在の表示状態。
		[[nodiscard]] UI_VISIBILITY GetVisibility() const noexcept;

		/// @brief UI要素の有効状態を設定します。
		/// @param enabled 設定する有効状態。
		void SetEnabled(bool enabled);

		/// @brief UI要素の有効状態を取得します。
		/// @return 現在の有効状態。
		[[nodiscard]] bool IsEnabled() const noexcept;

		/// @brief ヒットテストの対象にするかを設定します。
		/// @param hitTestVisible 設定するヒットテスト状態。
		void SetHitTestVisible(bool hitTestVisible);

		/// @brief ヒットテスト対象か取得します。
		/// @return ヒットテスト対象か?
		[[nodiscard]] bool IsHitTestVisible() const noexcept;

		/// @brief UI要素の不透明度を設定します。
		/// @param opacity 0.0～1.0までの不透明度
		void SetOpacity(float opacity);

		/// @brief UI要素の不透明度を取得します。
		/// @return 現在の不透明度。(0.0～1.0)
		[[nodiscard]] float GetOpacity() const noexcept;

		/// @brief UI要素の無効化状態を設定します。
		/// @param invalidation 設定する無効化状態。
		void Invalidate(UI_INVALIDATION invalidation);

		/// @brief UI要素の無効化状態を取得します。
		/// @return 現在の無効化状態。
		[[nodiscard]] UI_INVALIDATION GetInvalidation() const noexcept;

		/// @brief 指定された UI_INVALIDATION
		/// フラグが設定されているかを判定します。
		/// @param invalidation 判定する UI_INVALIDATION フラグ
		/// @return 指定された UI_INVALIDATION フラグが設定されている場合は
		/// true、未設定の場合は false
		[[nodiscard]] bool
		IsInvalidated(UI_INVALIDATION invalidation) const noexcept;

		/// @brief 利用可能なサイズから、この要素が必要とするサイズを計算します。
		/// @param availableSize 親から与えられた利用可能サイズ。
		/// @return この要素が希望するサイズ。
		[[nodiscard]] Vec2 Measure(const Vec2& availableSize);

		/// @brief 最後に計算された希望サイズを取得します。
		/// @return 最後に計算された希望サイズ。
		[[nodiscard]] const Vec2& GetDesiredSize() const noexcept;

		/// @brief 親から与えられた最終的な矩形に要素を配置します。
		/// @param finalRect 親から与えられた最終的な矩形。
		void Arrange(const UiRect& finalRect);

		/// @brief 確定した配置矩形を取得します。
		/// @note Collapsedの場合、位置は保持されますが、サイズは0になります。
		/// @return 確定した配置矩形。
		[[nodiscard]] const UiRect& GetLayoutRect() const noexcept;

		/// @brief ワールド座標系における配置矩形を取得します。
		/// @return ワールド座標系における配置矩形。
		[[nodiscard]] const UiRect& GetWorldRect() const noexcept;

		/// @brief ワールド矩形が計算済みか判定します。
		/// @return ワールド矩形が計算済みの場合はtrue、そうでない場合はfalse。
		[[nodiscard]] bool HasWorldRect() const noexcept;

	protected:
		/// @brief UI要素固有の更新処理を実行します。
		/// @param context 現在のフレーム情報。
		virtual void OnUpdate(const UiUpdateContext& context) {
			static_cast<void>(context);
		}

		/// @brief 親要素が変更されたときに呼び出されます。
		/// @param oldParent 変更前の親要素のポインタ。
		/// @param newParent 変更後の親要素のポインタ。
		virtual void OnParentChanged(
			UiContainer* oldParent, UiContainer* newParent
		) noexcept {
			static_cast<void>(oldParent);
			static_cast<void>(newParent);
		}

		/// @brief 表示状態が変更されたときに呼び出されます。
		/// @param oldVisibility 変更前の表示状態
		/// @param newVisibility 変更後の表示状態
		virtual void OnVisibilityChanged(
			const UI_VISIBILITY oldVisibility,
			const UI_VISIBILITY newVisibility
		) {
			static_cast<void>(oldVisibility);
			static_cast<void>(newVisibility);
		}

		/// @brief 有効状態が変更されたときに呼び出されます。
		/// @param oldEnabled 変更前の有効状態
		/// @param newEnabled 変更後の有効状態
		virtual void OnEnabledChanged(
			const bool oldEnabled, const bool newEnabled
		) {
			static_cast<void>(oldEnabled);
			static_cast<void>(newEnabled);
		}

		/// @brief ヒットテスト対象状態が変更されたときに呼び出されます。
		/// @param oldHitTestVisible 変更前のヒットテスト対象状態
		/// @param newHitTestVisible 変更後のヒットテスト対象状態
		virtual void OnHitTestVisibleChanged(
			const bool oldHitTestVisible, const bool newHitTestVisible
		) {
			static_cast<void>(oldHitTestVisible);
			static_cast<void>(newHitTestVisible);
		}

		/// @brief 不透明度が変更されたときに呼び出されます。
		/// @param oldOpacity 変更前の不透明度
		/// @param newOpacity 変更後の不透明度
		virtual void OnOpacityChanged(
			const float oldOpacity, const float newOpacity
		) {
			static_cast<void>(oldOpacity);
			static_cast<void>(newOpacity);
		}

		/// @brief 無効化状態が変更されたときに呼び出されます。
		/// @param invalidation 変更された無効化状態
		void ClearInvalidation(UI_INVALIDATION invalidation) noexcept;

		/// @brief 無効化状態が変更されたときに呼び出されます。
		/// @param newInvalidation 変更された無効化状態
		virtual void OnInvalidated(const UI_INVALIDATION newInvalidation) {
			static_cast<void>(newInvalidation);
		}

		/// @brief UI要素の希望サイズを計算します。
		/// @param availableSize 親から与えられた利用可能サイズ。
		[[nodiscard]] virtual Vec2 OnMeasure(const Vec2& availableSize);

		/// @brief 固有の配置処理を実行します。
		/// @note コンテナ系の派生クラスは、この関数内で子要素のArrange()を呼び出します。
		virtual void OnArrange(const UiRect& finalRect) {
			static_cast<void>(finalRect);
		}

		/// @brief ワールド矩形が変化した際に呼び出されます。
		/// @param oldWorldRect 変更前のワールド矩形
		/// @param newWorldRect 変更後のワールド矩形
		virtual void OnWorldRectChanged(
			const UiRect& oldWorldRect, const UiRect& newWorldRect
		) {
			static_cast<void>(oldWorldRect);
			static_cast<void>(newWorldRect);
		}

	private:
		friend class UiContainer;
		friend class UiTree;

		/// @brief 親要素を設定します。
		/// @param parent 設定する親要素のポインタ。取り除く場合は nullptr
		/// を指定します。
		void SetParent(UiContainer* parent) noexcept;

		/// @brief ツリー走査対象の子要素数を取得します。
		/// @return ツリー走査対象の子要素数。
		[[nodiscard]] virtual size_t GetTraversalChildCount() const noexcept;

		/// @brief ツリー走査対象となる子要素を取得します。
		/// @param index 取得する子要素のインデックス
		/// @return 指定されたインデックスの子要素の参照。
		[[nodiscard]] virtual UiElement& GetTraversalChild(size_t index);

		//---------------------------------------------------------------------

		UiContainer* mParent = nullptr; // 親コンテナへのポインタ

		UiRect mLayoutRect      = {}; // 確定した配置矩形
		UiRect mLastArrangeRect = {}; // 最後に計算された配置矩形
		UiRect mWorldRect       = {}; // ワールド矩形

		Vec2 mDesiredSize              = Vec2::zero; // 希望サイズ
		Vec2 mLastMeasureAvailableSize = Vec2::zero; // 最後に計算された利用可能サイズ

		float mOpacity = 1.0f; // 不透明度

		UI_VISIBILITY   mVisibility   = UI_VISIBILITY::VISIBLE; // 表示状態
		UI_INVALIDATION mInvalidation = UI_INVALIDATION::ALL;   // 無効化状態

		bool mEnabled          = true;  // 有効状態
		bool mHitTestVisible   = true;  // ヒットテスト対象か?
		bool mHasMeasureResult = false; // Measure() が呼ばれたか?
		bool mIsMeasuring      = false; // Measure() の実行中か?
		bool mHasArrangeResult = false; // Arrange() が呼ばれたか?
		bool mIsArranging      = false; // Arrange() の実行中か?
		bool mHasWorldRect     = false; // ワールド矩形が計算済みか?
	};
} // namespace Unnamed
