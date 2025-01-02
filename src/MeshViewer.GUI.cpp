#include "MeshViewer.h"

using namespace BBSMesh;

void MeshViewer::ProcessGUI()
{
	GUI_MenuBar();

	GUI_SideBar();

	GUI_Modals();

	GUI_ExportOptions();

	GUI_CamWindow();
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

		if (ImGui::BeginMenu("Camera"))
		{
			if (ImGui::MenuItem("Reset Camera"))
				m_currentCamera->Reset(glm::vec3(0.0f, 1.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.f, 0.f);
			if (ImGui::MenuItem("Show Camera Window"))
				showCamWindow = true;

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

	if (ImGui::Begin("Model"))
	{
		if (m_model == nullptr)
		{
			ImGui::Text("Nothing loaded.");
		}
		else
		{
			ImGui::Text("Model num %d group %d", (int)m_model->num, (int)m_model->group);
			ImGui::Text("Textures %d Triangles %d Verts %d", m_model->textureNames.size(), m_model->fileTriCount, m_model->fileVertCount);
			ImGui::Text("Scale %f Sec0: %d Sec1: %d", m_model->scale, m_model->sections.size(), m_model->transSections.size());
			if (m_model->textureNames.size() > 0)
			{
				if (ImGui::TreeNode("Textures"))
				{
					for (const std::string& texName : m_model->textureNames)
						ImGui::Text(texName.c_str());
					ImGui::TreePop();
				}
			}
			if (m_model->sections.size() > 0)
			{
				if (ImGui::TreeNode("Sections 0"))
				{
					for (int i = 0; i < m_model->sections.size(); i++)
					{
						ImGui::Text("V %d T %d G %d F [%s]", m_model->sections[i]->vertexCount, m_model->sections[i]->textureIndex, m_model->sections[i]->group, m_model->sections[i]->formatStr.c_str());
					}
					ImGui::TreePop();
				}
			}
			if (m_model->transSections.size() > 0)
			{
				if (ImGui::TreeNode("Sections 1"))
				{
					for (int i = 0; i < m_model->transSections.size(); i++)
					{
						ImGui::Text("V %d T %d G %d F [%s]", m_model->transSections[i]->vertexCount, m_model->transSections[i]->textureIndex, m_model->transSections[i]->group, m_model->transSections[i]->formatStr.c_str());
					}
					ImGui::TreePop();
				}
			}
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

void MeshViewer::GUI_CamWindow()
{
	if (showCamWindow)
	{
		if (ImGui::Begin("Camera", &showCamWindow))
		{
			CCamera* camera = m_currentCamera.get();
			ImGui::Text("Avg. %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Pos { %.2f, %.2f, %.2f }", camera->Position.x, camera->Position.y, camera->Position.z);
			ImGui::Text("Pitch %.2f Yaw %.2f", camera->Pitch, camera->Yaw);
		}
		ImGui::End();
	}
}