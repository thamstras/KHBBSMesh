#include "Common.h"
#include <stdexcept>
#include "ArcLoader.h"

ArcLoader::ArcLoader(std::filesystem::path filePath)
{
	file.open(filePath, std::ios_base::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file!");
	}

	arcData = BbsArc::ReadArcFile(file);
}

void ArcLoader::Attatch(std::function<void(std::string, std::ifstream&)> callback)
{
	onLoadCallback = callback;
}

bool ArcLoader::gui_window()
{
	if (ImGui::Begin("Arc Load", &isOpen))
	{
		if (ImGui::BeginListBox("##empty", ImVec2(-FLT_MIN, 0.0f)))
		{
			for (int i = 0; i < arcData.entries.size(); i++)
			{
				if (ImGui::Selectable(arcData.entries[i].fileName, (i == selection)))
				{
					selection = i;
				}
			}
			ImGui::EndListBox();
		}
		
		ArcEntry& currEntry = arcData.entries[selection];
		ImGui::Text("Name: %s", currEntry.fileName);
		ImGui::Text("Size: %d", currEntry.dataLength);

		std::filesystem::path namep = std::filesystem::path(currEntry.fileName);
		auto ext = namep.extension();
		bool disabled = false;
		if ((ext != ".pmo" && ext != ".pam") || currEntry.IsLink())
			disabled = true;

		ImGui::BeginDisabled(disabled);
		if (ImGui::Button("Load this"))
		{
			if (onLoadCallback)
			{
				file.seekg(currEntry.dataOffset);
				std::string strName = std::string(currEntry.fileName);
				onLoadCallback(strName, file);
			}
		}
		ImGui::EndDisabled();
	}
	ImGui::End();

	return isOpen;
}