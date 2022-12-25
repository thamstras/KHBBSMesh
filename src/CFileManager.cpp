#include "CFileManager.h"
#include <iostream>

namespace fs = std::filesystem;

CFileManager::CFileManager()
{
	fs::path root  = FindRootDir();

	rootDir = fs::canonical(root);
	resourcesDir = fs::canonical(root.append("resources"));

	std::cout << "[FS] Root found at " << rootDir << std::endl;
	std::cout << "[FS] Resources found at " << resourcesDir << std::endl;
}

CFileManager::CFileManager(std::string rootDirOverride)
{
	// TODO: Check override exists
	fs::path root = fs::path(rootDirOverride);

	rootDir = fs::canonical(root);
	resourcesDir = fs::canonical(root.append("resources"));

	std::cout << "[FS] Root override at " << rootDir << std::endl;
	std::cout << "[FS] Resources override at " << resourcesDir << std::endl;
}

CFileManager::~CFileManager()
{
	// TODO?
}

bool dirContains(const fs::path& directory, const std::string& name)
{
	if (!fs::is_directory(directory)) return false;

	for (auto dirItr = fs::directory_iterator(directory); dirItr != fs::directory_iterator(); dirItr++)
	{
		if (dirItr->is_directory() && dirItr->path().filename() == name)
		{
			return true;
		}
	}

	return false;
}

fs::path CFileManager::FindRootDir()
{
	fs::path currentPath = fs::current_path();
	std::cout << "[FS] Starting from path " << currentPath << std::endl;

	if (currentPath.filename() == "bin")
	{
		currentPath = currentPath.parent_path();
		std::filesystem::current_path(currentPath);
	}

	bool foundResources = dirContains(currentPath, "resources");
	
	if (!foundResources)
	{
		fs::path trialPath = FindExeDir();
		if (trialPath.filename() == "bin")
		{
			trialPath = trialPath.parent_path();
		}
		if (dirContains(trialPath, "resources"))
		{
			currentPath = trialPath;
			fs::current_path(trialPath);
			foundResources = true;
		}

	}

	if (foundResources)
	{
		return currentPath;
	}
	else
	{
		ShowInitFailMessageBox();
		std::exit(-1);
	}
}

const char* ResourceType(EResourceType type)
{
	switch (type)
	{
	case EResourceType::RSRC_FONT:
		return "FONT";
	case EResourceType::RSRC_SHADER:
		return "SHADER";
	case EResourceType::RSRC_TEXTURE:
		return "TEXTURE";
	default:
		return "";
	}
}

std::string CFileManager::GetResourcePath(EResourceType type, std::string name)
{
	fs::path target = resourcesDir;
	switch (type)
	{
	case EResourceType::RSRC_FONT:
		target.append("fonts");
		break;
	case EResourceType::RSRC_SHADER:
		target.append("shaders");
		break;
	case EResourceType::RSRC_TEXTURE:
		target.append("textures");
		break;
	}

	target.append(name);
	if (fs::exists(target))
		return target.string();

	std::cerr << "[FS] FAILED TO FIND RSRC " << ResourceType(type) << " " << name << std::endl;
	return std::string("");
}