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
			if (ImGui::BeginMenu("Open..."))
			{
				if (ImGui::MenuItem("Model", "Ctrl+O"))
				{
					m_modalMessage = std::string("Loading...");
					//ImGui::OpenPopup("MessageModal");
					ScheduleDelayedProcess(&MeshViewer::OpenModelFile);
					//ScheduleDelayedProcess(&MeshViewer::HideMessageModal);
				}

				if (ImGui::MenuItem("Anim", "Ctrl+A", nullptr, false))
				{
					//ScheduleDelayedProcess(&MeshViewer::OpenAnimFile);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Export..."))
			{
				// TODO: Export
				ImGui::MenuItem("Model", nullptr, nullptr, false);
				ImGui::MenuItem("Anim", nullptr, nullptr, false);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Close..."))
			{
				// TODO: Close
				ImGui::MenuItem("Model", nullptr, nullptr, false);
				ImGui::MenuItem("Anim", nullptr, nullptr, false);

				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Exit", "Esc"))
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
	if (ImGui::Begin("Skeleton"))
	{
		ImGui::Checkbox("Draw Skeleton", &this->drawSkel);
		ImGui::Separator();

		if (m_guiAnim != nullptr)
			m_guiAnim->GUI_DrawControls();
		else
			ImGui::Text("Nothing loaded.");
	}
	ImGui::End();
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