#include "MeshViewer.h"

using namespace BBSMesh;

void MeshViewer::ProcessGUI()
{
	GUI_MenuBar();

	GUI_SideBar();

	GUI_Modals();

	GUI_ExportOptions();
}

void MeshViewer::GUI_MenuBar()
{
	bool openAbout = false;
	bool openExport = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Open..."))
			{
				if (ImGui::MenuItem("Model", "Ctrl+O"))
				{
					//m_modalMessage = std::string("Loading...");
					//ImGui::OpenPopup("MessageModal");
					ScheduleDelayedProcess(&MeshViewer::OpenModelFile);
					//ScheduleDelayedProcess(&MeshViewer::HideMessageModal);
				}

				if (ImGui::MenuItem("Anim", "Ctrl+A"))
				{
					ScheduleDelayedProcess(&MeshViewer::OpenAnimFile);
				}

				ImGui::EndMenu();
			}

			/*if (ImGui::BeginMenu("Export..."))
			{
				// TODO: Export
				ImGui::MenuItem("Model", nullptr, nullptr, false);
				ImGui::MenuItem("Anim", nullptr, nullptr, false);

				ImGui::EndMenu();
			}*/
			if (ImGui::MenuItem("Export", nullptr))
			{
				openExport = true;
			}

			if (ImGui::BeginMenu("Close..."))
			{
				if (ImGui::MenuItem("Model", nullptr))
					ScheduleDelayedProcess(&MeshViewer::CloseModelFile);
				if (ImGui::MenuItem("Anim", nullptr))
					ScheduleDelayedProcess(&MeshViewer::CloseAnimFile);
					
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
				openAbout = true;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (openAbout) ImGui::OpenPopup("AboutWindow");
	if (openExport) ImGui::OpenPopup("ExportModal");
}

void MeshViewer::GUI_SideBar()
{
	if (ImGui::Begin("Skeleton"))
	{
		ImGui::Checkbox("Draw Skeleton", &this->drawSkel);
		ImGui::Separator();

		if (ImGui::BeginCombo("Anim Mode", currAnim == AnimType::GuiAnim ? "Manual" : "Anim File"))
		{
			if (ImGui::Selectable("Manual", currAnim == AnimType::GuiAnim))
				SetAnimType(AnimType::GuiAnim);
			if (ImGui::Selectable("Anim File", currAnim == AnimType::FileAnim))
				SetAnimType(AnimType::FileAnim);
			ImGui::EndCombo();
		}

		if (currAnim == AnimType::GuiAnim)
		{
			if (m_guiAnim != nullptr)
				m_guiAnim->GUI_DrawControls();
			else
				ImGui::Text("Nothing loaded.");
		}
		else
		{
			if (m_anims != nullptr)
				m_anims->GUI_DrawControls();
			else
				ImGui::Text("Nothing loaded.");
		}
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

	if (ImGui::IsPopupOpen("AboutWindow"))
	{
		auto wind = ImGui::GetWindowSize();
		ImGui::SetNextWindowPos(ImVec2(0.5f * wind.x, 0.5f * wind.y), ImGuiCond_Appearing);
	}
	if (ImGui::BeginPopup("AboutWindow"))
	{
		ImGui::Text("KH BBS Mesh");
		ImGui::TextDisabled("By Thamstras");
		ImGui::TextDisabled("Special Thanks: Revel8ion, Akderebur");
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

void MeshViewer::GUI_ExportOptions()
{
	if (ImGui::BeginPopupModal("ExportModal"))
	{
		if (!gotFormats)
		{
			exportFormats = GetExportOptions();
			gotFormats = true;
			for (auto& format : exportFormats)
			{
				if (format.id == "fbx")
				{
					currFormat = format;
					break;
				}
			}
		}

		ImGui::Text("Export Options");

		ImGui::Separator();

		ImGui::InputText("Folder", pathBuf, 260);
		ImGui::Text("Will append model name to path");

		ImGui::Separator();

		if (ImGui::BeginCombo("Format", currFormat->id.c_str()))
		{
			for (auto& format : exportFormats)
			{
				if (ImGui::Selectable(format.desc.c_str(), currFormat->id == format.id))
					currFormat = format;
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Export"))
		{
			ImGui::CloseCurrentPopup();
			ExportAnimFile();
		}

		ImGui::EndPopup();
	}
}