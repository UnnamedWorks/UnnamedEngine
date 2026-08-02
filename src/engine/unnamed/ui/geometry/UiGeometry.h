#pragma once

namespace Unnamed {
	/// @brief UI上の矩形構造体
	struct UiRect final {
		Vec2 pos  = Vec2::zero;
		Vec2 size = Vec2::zero;

		/// @brief 矩形の最小座標を取得します。
		/// @return 矩形の最小座標
		[[nodiscard]] Vec2 GetMin() const noexcept;

		/// @brief 矩形の最大座標を取得します。
		/// @return 矩形の最大座標
		[[nodiscard]] Vec2 GetMax() const noexcept;

		/// @brief 矩形の中心座標を取得します。
		/// @return 矩形の中心座標
		[[nodiscard]] Vec2 GetCenter() const noexcept;

		/// @brief 矩形の半分のサイズを取得します。
		/// @return 矩形の半分のサイズ
		[[nodiscard]] Vec2 GetHalfSize() const noexcept;

		/// @brief 矩形の左側の座標を取得します。
		/// @return 矩形の左側の座標
		[[nodiscard]] float GetLeft() const noexcept;

		/// @brief 矩形の上側の座標を取得します。
		/// @return 矩形の上側の座標
		[[nodiscard]] float GetTop() const noexcept;

		/// @brief 矩形の右側の座標を取得します。
		/// @return 矩形の右側の座標
		[[nodiscard]] float GetRight() const noexcept;

		/// @brief 矩形の下側の座標を取得します。
		/// @return 矩形の下側の座標
		[[nodiscard]] float GetBottom() const noexcept;

		/// @brief 矩形の幅を取得します。
		/// @return 矩形の幅
		[[nodiscard]] float GetWidth() const noexcept;

		/// @brief 矩形の高さを取得します。
		/// @return 矩形の高さ
		[[nodiscard]] float GetHeight() const noexcept;

		/// @brief 矩形が有効かどうかを判定します。
		/// @return 矩形が有効な場合は true、無効な場合は false
		[[nodiscard]] bool IsValid() const noexcept;

		/// @brief 矩形に面積がないかを判定します。
		/// @return 矩形に面積がない場合は true、面積がある場合は false
		[[nodiscard]] bool IsEmpty() const noexcept;

		/// @brief 指定された点が矩形内に含まれているかを判定します。
		/// @param point 判定する点の座標
		/// @return 指定された点が矩形内に含まれている場合は true、含まれていない場合は false
		[[nodiscard]] bool Contains(const Vec2& point) const noexcept;

		/// @brief 最小と最大の座標から矩形を作成します。
		/// @param min 最小座標
		/// @param max 最大座標
		/// @return 最小と最大の座標から作成された矩形
		[[nodiscard]] static UiRect FromMinMax(
			const Vec2& min, const Vec2& max
		) noexcept;
	};

	/// @brief UI上の厚さ構造体
	struct UiThickness final {
		float left   = 0.0f;
		float top    = 0.0f;
		float right  = 0.0f;
		float bottom = 0.0f;

		/// @brief すべての辺の厚さを同じ値に設定した UiThickness を作成します。
		/// @param value すべての辺の厚さに設定する値
		/// @return すべての辺の厚さを同じ値に設定した UiThickness
		[[nodiscard]] static UiThickness Uniform(float value) noexcept;

		/// @brief 左右の厚さを同じ値、上下の厚さを同じ値に設定した UiThickness を作成します。
		/// @param horizontal 左右の厚さに設定する値
		/// @param vertical 上下の厚さに設定する値
		/// @return 左右の厚さを同じ値、上下の厚さを同じ値に設定した UiThickness
		[[nodiscard]] static UiThickness Symmetric(
			float horizontal, float vertical
		) noexcept;

		/// @brief 水平の厚さを取得します。
		/// @return 水平の厚さ（左 + 右）
		[[nodiscard]] float GetHorizontal() const noexcept;

		/// @brief 垂直の厚さを取得します。
		/// @return 垂直の厚さ（上 + 下）
		[[nodiscard]] float GetVertical() const noexcept;

		/// @brief 厚さが有限かどうかを判定します。
		/// @return 厚さが有限な場合は true、無限大または NaN の場合は false
		[[nodiscard]] bool IsFinite() const noexcept;
	};

	/// @brief 角丸矩形の各角の円弧半径を表します。
	struct UiCornerRadii final {
		float topLeft     = 0.0f;
		float topRight    = 0.0f;
		float bottomRight = 0.0f;
		float bottomLeft  = 0.0f;

		/// @brief すべての角の半径を同じ値に設定した UiCornerRadii を作成します。
		/// @param radius すべての角の半径に設定する値
		/// @return すべての角の半径を同じ値に設定した UiCornerRadii
		[[nodiscard]] static UiCornerRadii Uniform(float radius) noexcept;

		/// @brief 指定されたサイズに基づいて、カプセル型の角丸矩形の半径を計算します。
		/// @param size 角丸矩形のサイズ
		/// @return カプセル型の角丸矩形の半径を設定した UiCornerRadii
		[[nodiscard]] static UiCornerRadii Capsule(const Vec2& size) noexcept;

		/// @brief 指定されたサイズに基づいて、角丸矩形の半径を正規化します。
		/// @param size 角丸矩形のサイズ
		/// @return 正規化された角丸矩形の半径を設定した UiCornerRadii
		[[nodiscard]] UiCornerRadii GetNormalized(
			const Vec2& size
		) const noexcept;

		/// @brief 厚さが有限かどうかを判定します。
		/// @return 厚さが有限な場合は true、無限大または NaN の場合は false
		[[nodiscard]] bool IsFinite() const noexcept;
	};

	/// @brief 角丸矩形を表します。
	struct UiRoundedRect final {
		UiRect        rect  = {};
		UiCornerRadii radii = {};

		/// @brief 正規化された角丸矩形を取得します。
		/// @return 正規化された角丸矩形
		[[nodiscard]] UiRoundedRect GetNormalized() const noexcept;

		/// @brief 角丸矩形が有効かどうかを判定します。
		/// @return 有効な場合は true、そうでない場合は false
		[[nodiscard]] bool IsValid() const noexcept;
	};
} // namespace Unnamed
