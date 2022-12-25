#include "MeshViewer.h"

using namespace BBSMesh;

void MeshViewer::ProcessGUI()
{
	// #### MENU BAR ####
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
				// TODO: Open
			}

			if (ImGui::MenuItem("Export", "Ctrl+E", nullptr, false))
			{
				// TODO: Export
			}

			if (ImGui::MenuItem("Close"))
			{
				// TODO: Close
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Exit", "Alt+F4"))
			{
				m_shouldQuit = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				// TODO:
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// #### SIDEBAR ####
	// TODO
}