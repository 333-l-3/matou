#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

namespace matou::tool::file {
	inline std::string getExecutableDir() {
		namespace fs = std::filesystem;
#ifdef _WIN32
		char buf[MAX_PATH];
		DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
		if (len == 0) return fs::current_path().string();
		return fs::path(std::string(buf, buf + len)).remove_filename().string();
#else
		try {
			return fs::current_path().string();
		} catch (...) {
			return std::string();
		}
#endif
	}

	// 尝试向上查找项目根目录：寻找常见标记（.git, res, src, .vscode, read.md）
	inline std::string getProjectDir() {
		namespace fs = std::filesystem;
		fs::path dir = fs::path(getExecutableDir());
		const std::vector<std::string> markers = {".git", "res", "src", ".vscode", "read.md"};
		for (int depth = 0; depth < 10; ++depth) {
			for (auto &m : markers) {
				fs::path candidate = dir / m;
				if (fs::exists(candidate)) return dir.string();
			}
			if (!dir.has_parent_path()) break;
			dir = dir.parent_path();
		}
		// 没找到标记则退回到可执行目录
		return getExecutableDir();
	}

	inline std::string resourcePath(const std::string& relPath) {
		namespace fs = std::filesystem;
		fs::path p = fs::path(getProjectDir()) / fs::path(relPath);
		return p.string();
	}

	inline bool exists(const std::string& path) {
		return std::filesystem::exists(std::filesystem::path(path));
	}

} // namespace matou::tool::file

#endif

