#pragma once

namespace Unnamed {
	/// @brief UI要素の再計算が必要な処理
	enum class UI_INVALIDATION: uint8_t {
		NONE = 0,

		// 希望サイズの再計算が必要
		MEASURE = 1u << 0,

		// 最終位置とサイズの再配置が必要
		ARRANGE = 1u << 1,

		// ワールド座標や変換行列の再計算が必要
		TRANSFORM = 1u << 2,

		// 描画コマンドの再生成が必要
		PAINT = 1u << 3,

		// HitTestの再計算が必要
		HIT_TEST = 1u << 4,

		// 子要素または描画順が変更されている
		CHILDREN = 1u << 5,

		// すべて
		ALL = MEASURE | ARRANGE | TRANSFORM | PAINT | HIT_TEST | CHILDREN
	};

	/// @brief UI_INVALIDATION フラグのビットORを行います。
	/// @param lhs 左辺
	/// @param rhs 右辺
	/// @return ビットORされた UI_INVALIDATION フラグ
	[[nodiscard]] constexpr UI_INVALIDATION operator|(
		const UI_INVALIDATION lhs, const UI_INVALIDATION rhs
	) noexcept {
		using Underlying = std::underlying_type_t<UI_INVALIDATION>;

		return static_cast<UI_INVALIDATION>(
			static_cast<Underlying>(lhs) | static_cast<Underlying>(rhs)
		);
	}

	/// @brief UI_INVALIDATION フラグのビットANDを行います。
	/// @param lhs 左辺
	/// @param rhs 右辺
	/// @return ビットANDされた UI_INVALIDATION フラグ
	[[nodiscard]] constexpr UI_INVALIDATION operator&(
		const UI_INVALIDATION lhs, const UI_INVALIDATION rhs
	) noexcept {
		using Underlying = std::underlying_type_t<UI_INVALIDATION>;

		return static_cast<UI_INVALIDATION>(
			static_cast<Underlying>(lhs) & static_cast<Underlying>(rhs)
		);
	}

	/// @brief UI_INVALIDATION フラグのビット反転を行います。
	/// @param value 反転する UI_INVALIDATION フラグ
	/// @return 反転された UI_INVALIDATION フラグ
	[[nodiscard]] constexpr UI_INVALIDATION operator~(
		const UI_INVALIDATION value
	) noexcept {
		using Underlying = std::underlying_type_t<UI_INVALIDATION>;

		return static_cast<UI_INVALIDATION>(~static_cast<Underlying>(value));
	}

	/// @brief UI_INVALIDATION フラグのビットOR代入を行います。
	/// @param lhs 左辺
	/// @param rhs 右辺
	/// @return ビットOR代入された UI_INVALIDATION フラグ
	constexpr UI_INVALIDATION& operator|=(
		UI_INVALIDATION& lhs, const UI_INVALIDATION rhs
	) noexcept {
		lhs = lhs | rhs;
		return lhs;
	}

	/// @brief UI_INVALIDATION フラグのビットAND代入を行います。
	/// @param lhs 左辺
	/// @param rhs 右辺
	/// @return ビットAND代入された UI_INVALIDATION フラグ
	constexpr UI_INVALIDATION& operator&=(
		UI_INVALIDATION& lhs, const UI_INVALIDATION rhs
	) noexcept {
		lhs = lhs & rhs;
		return lhs;
	}

	/// @brief 指定された UI_INVALIDATION フラグのいずれかが設定されているかを判定します。
	/// @param value 判定する UI_INVALIDATION フラグ
	/// @param flags 判定する UI_INVALIDATION フラグの集合
	/// @return 指定された UI_INVALIDATION フラグのいずれかが設定されている場合は true、すべて未設定の場合は false
	[[nodiscard]] constexpr bool HasAnyInvalidation(
		const UI_INVALIDATION value, const UI_INVALIDATION flags
	) noexcept {
		return (value & flags) != UI_INVALIDATION::NONE;
	}

	/// @brief 指定された UI_INVALIDATION フラグのすべてが設定されているかを判定します。
	/// @param value 判定する UI_INVALIDATION フラグ
	/// @param flags 判定する UI_INVALIDATION フラグの集合
	/// @return 指定された UI_INVALIDATION フラグのすべてが設定されている場合は true、いずれかが未設定の場合は false
	[[nodiscard]] constexpr bool HasAllInvalidation(
		const UI_INVALIDATION value, const UI_INVALIDATION flags
	) noexcept {
		return (value & flags) == flags;
	}

	/// @brief Invalidation間の依存関係を展開します。
	/// @note Measureが必要な場合、その結果を使用するArrangeも必要になります。
	/// @param invalidation 展開する UI_INVALIDATION フラグ
	/// @return 展開された UI_INVALIDATION フラグ
	[[nodiscard]] constexpr UI_INVALIDATION ExpandUiInvalidationDependencies(
		UI_INVALIDATION invalidation
	) noexcept {
		if (HasAnyInvalidation(invalidation, UI_INVALIDATION::MEASURE)) {
			invalidation |= UI_INVALIDATION::ARRANGE;
		}

		return invalidation;
	}
}
