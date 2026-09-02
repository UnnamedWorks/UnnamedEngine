#include "ConsoleScriptParser.h"

#include <fstream>

#include <core/string/StrUtil.h>
#include <core/filesystem/Path.h>

#include <engine/unnamed/subsystem/console/Log.h>

namespace Unnamed {
	static constexpr std::string_view kChannel = "ConScrP";

	namespace {
		// 行内のコメントを取り除く
		std::string RemoveInLineComment(const std::string& line) {
			const size_t commentPos = line.find("//");
			if (commentPos != std::string::npos) {
				return line.substr(0, commentPos);
			}
			return line;
		}
	}

	// TODO: 仮想パスに対応していないので要修正

	void ConsoleScriptParser::ParseAndExecute(const Path& path) {
		const auto& native = path.Native();

		// ファイルを開く
		std::ifstream inputFile(native);

		// 存在しない場合は作る
		if (!inputFile) {
			Msg(
				kChannel,
				"Script file not found. Creating a new one: {}",
				std::string(path)
			);
			std::ofstream outputFile(native);
			if (!outputFile) {
				Error(
					kChannel,
					"Failed to create script file: {}",
					std::string(path)
				);
				throw std::runtime_error("Failed to create script file");
			}
			outputFile.close();
			inputFile.open(native);
		}

		if (!inputFile.is_open()) {
			Error(
				kChannel,
				"Failed to open script file: {}",
				std::string(path)
			);
			throw std::runtime_error("Failed to open script file");
		}

		Msg(kChannel, "Executing script file: {}", std::string(path));

		std::string line;
		while (std::getline(inputFile, line)) {
			line = StrUtil::TrimSpaces(line); // 不要な空白を削除
			
			line = RemoveInLineComment(line); // 行内のコメントを削除

			// 空行をスキップ
			if (line.empty()) {
				continue;
			}
			
			// コマンドを実行
			ServiceLocator::Get<ConsoleSystem>()->ExecuteCommand(line);
		}
	}
}
