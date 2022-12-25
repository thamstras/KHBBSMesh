#pragma once
#include <filesystem>

enum class EResourceType
{
	RSRC_FONT,
	RSRC_SHADER,
	RSRC_TEXTURE
};

class CFileManager
{
public:
	CFileManager();
	CFileManager(std::string rootDirOverride);
	~CFileManager();

	std::string GetResourcePath(EResourceType type, std::string name);

	// TODO: REFACTOR
	bool OpenFileWindow(std::string& out_path);
	bool OpenDirectoryWindow(std::string& out_path);

private:
	std::filesystem::path rootDir;
	std::filesystem::path resourcesDir;

	std::filesystem::path FindRootDir();
	std::filesystem::path FindExeDir();
	void ShowInitFailMessageBox();
};

