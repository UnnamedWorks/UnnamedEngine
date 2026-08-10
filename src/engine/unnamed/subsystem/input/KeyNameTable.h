#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <engine/unnamed/subsystem/input/InputKey.h>

namespace Unnamed {
	/// @brief 入力キー構造体のハッシュ関数
	struct KeyHash {
		size_t operator()(const InputKey& key) const noexcept {
			return static_cast<size_t>(key.device) << 24 ^ key.code;
		}
	};

	/// @brief キー名変換テーブルクラス
	class KeyNameTable {
	public:
		/// @brief 文字列からInputKeyを取得します
		/// @param name キー名の文字列
		/// @return 対応するInputKey（存在しない場合はstd::nullopt）
		static std::optional<InputKey> FromString(std::string_view);

		/// @brief InputKeyから文字列を取得します
		/// @param key 入力キー
		static std::string_view ToString(const InputKey& key);

		/// @brief キー名からInputKeyへのマッピングを取得します
		/// @return キー名からInputKeyへのマッピング
		static const std::unordered_map<std::string, InputKey>& NameToKey();

		/// @brief InputKeyからキー名へのマッピングを取得します
		static const std::unordered_map<
			InputKey, std::string_view, KeyHash>& KeyToName();

	private:
		/// @brief 文字列を正規化します（小文字化）
		/// @param str 正規化する文字列
		/// @return 正規化された文字列
		static std::string Normalize(std::string_view str);

		static const std::unordered_map<std::string, InputKey> kSNameToKey;
		static const std::unordered_map<InputKey, std::string_view, KeyHash>
		kSKeyToName;
	};
}
