#pragma once
#include <span>

#include "UiElement.h"

namespace Unnamed {
	class UiContainer : public UiElement {
	public:
		UiContainer() = default;
		~UiContainer() override;

		/// @brief 子要素を追加します。
		/// @tparam T 追加する子要素の型(UiElement の継承必須)
		/// @param args 追加する子要素のコンストラクタ引数
		/// @return 追加された子要素の参照を返します。
		template <std::derived_from<UiElement> T, typename... Args>
		T& CreateChild(Args&&... args) {
			auto child = std::make_unique<T>(std::forward<Args>(args)...);

			T& result = *child;

			AddChild(std::move(child));

			return result;
		}

		/// @brief 子要素を追加します。
		/// @param child 追加する子要素の std::unique_ptr
		/// @return 追加された子要素の参照を返します。
		UiElement& AddChild(std::unique_ptr<UiElement> child);

		/// @brief 子要素を削除します。
		/// @param child 削除する子要素の参照
		/// @return 削除された子要素の std::unique_ptr を返します。
		[[nodiscard]] std::unique_ptr<UiElement> RemoveChild(UiElement& child);

		/// @brief 子要素を削除します。
		/// @param child 削除する子要素の参照
		/// @return 削除に成功したか?
		bool DestroyChild(UiElement& child);

		/// @brief 子要素をすべて削除します。
		void ClearChildren() noexcept;

		/// @brief 子要素の数を取得します。
		/// @return 子要素の数。
		[[nodiscard]] size_t GetChildCount() const noexcept;

		/// @brief 指定されたインデックスの子要素を取得します。
		/// @param index 取得する子要素のインデックス
		/// @return 指定されたインデックスの子要素の参照。
		[[nodiscard]] UiElement& GetChild(size_t index);

		/// @brief 指定されたインデックスの子要素を取得します。(const 版)
		/// @param index 取得する子要素のインデックス
		/// @return 指定されたインデックスの子要素の参照。(const 版)
		[[nodiscard]] const UiElement& GetChild(size_t index) const;

		/// @brief 子要素の配列を取得します。
		/// @return 子要素の配列。
		[[nodiscard]] std::span<const std::unique_ptr<UiElement>>
		GetChildren() const noexcept;

	protected:
		/// @brief 子要素が変更されたときに呼び出されます。
		virtual void OnChildrenChanged() noexcept {
		}

	private:
		/// @brief 子要素の数を取得します。(UiElement のオーバーライド)
		/// @return ツリー走査対象の子要素数。
		[[nodiscard]] size_t GetTraversalChildCount() const noexcept override;

		/// @brief 子要素を取得します。(UiElement のオーバーライド)
		/// @param index 取得する子要素のインデックス
		/// @return 指定されたインデックスの子要素の参照。
		[[nodiscard]] UiElement& GetTraversalChild(size_t index) override;

		/// @brief 子要素の変更を通知します。
		void NotifyChildrenChanged() noexcept;

		//---------------------------------------------------------------------

		std::vector<std::unique_ptr<UiElement>> mChildren;
	};
} // namespace Unnamed
