#pragma once
#include <filesystem>

enum class EResourceType
{
	RSRC_FONT,
	RSRC_SHADER,
	RSRC_TEXTURE
};

enum class EFileOpenType
{
	FILE_PMO,
	FILE_PAM,
	FILE_ANY
};

class CFileManager
{
public:
	CFileManager();
	CFileManager(std::string rootDirOverride);
	~CFileManager();

	std::string GetResourcePath(EResourceType type, std::string name);

	// TODO: REFACTOR
	bool OpenFileWindow(std::string& out_path, EFileOpenType type = EFileOpenType::FILE_ANY);
	bool OpenDirectoryWindow(std::string& out_path);

private:
	std::filesystem::path rootDir;
	std::filesystem::path resourcesDir;

	std::filesystem::path FindRootDir();
	std::filesystem::path FindExeDir();
	void ShowInitFailMessageBox();
};

