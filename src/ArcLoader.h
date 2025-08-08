#pragma once
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include "FileTypes/BbsArc.h"

class ArcLoader
{
public:
	ArcLoader(std::filesystem::path filePath);

	void Attatch(std::function<void(std::string, std::ifstream&)> callback);

	bool gui_window();

private:
	std::ifstream file;
	BbsArc arcData;
	std::function<void(std::string, std::ifstream&)> onLoadCallback;
	int selection = 0;
	bool isOpen = true;
};

