#include "MeshViewer.h"

using namespace BBSMesh;

void MeshViewer::ProcessGUI()
{
	GUI_MenuBar();

	GUI_SideBar();

	GUI_Modals();
}

void MeshViewer::GUI_MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
				m_modalMessage = std::string("Loading...");
				//ImGui::OpenPopup("MessageModal");
				ScheduleDelayedProcess(&MeshViewer::OpenFile);
				//ScheduleDelayedProcess(&MeshViewer::HideMessageModal);
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
}

void MeshViewer::GUI_SideBar()
{

}

void MeshViewer::GUI_Modals()
{
	if (ImGui::BeginPopupModal("MessageModal"))
	{
		ImGui::Text(m_modalMessage.c_str());
		ImGui::EndPopup();
	}
}

void MeshViewer::HideMessageModal()
{
	// TODO: Delayed funcs are called "outside" of a frame so this doesn't work.
	/*if (ImGui::BeginPopupModal("MessageModal"))
	{
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}*/
}