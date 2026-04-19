#include "Common.h"
#include "ExportOp.h"
#include "MeshViewer.h"

using namespace BBSMesh;

std::optional<ExportOptions> ExportOp::prevExpOpts;

ExportOp::ExportOp(MeshViewer* app) : app(app)
{
	opts.model_name = app->CurrModelName();

	if (prevExpOpts.has_value() && prevExpOpts->model_name == opts.model_name)
	{
		opts = *prevExpOpts;
	}
	else
	{
		do
		{
			opts.anim_name = app->AnimName(opts.anim_idx).c_str();
			if (opts.anim_name[0] == '\0') opts.anim_idx++;
		} while (opts.anim_name[0] == '\0' && opts.anim_idx < app->LoadedAnimCount());
	}
	UpdateFinalPath();
}

const char* NameAppendModeStr[] = {
	"None", "Folder", "File", "Both"
};

bool AppendWidget(const char* label, NameAppendMode* option)
{
	bool changed = false;
	int iOpt = (int)*option;
	if (ImGui::BeginCombo(label, NameAppendModeStr[iOpt]))
	{
		for (int i = 0; i < 4; i++)
		{
			if (ImGui::Selectable(NameAppendModeStr[i], i == iOpt))
			{
				*option = (NameAppendMode)i;
				changed = true;
			}
			if (i == iOpt) ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}
	return changed;
}

bool ExportOp::gui_window()
{
	bool stayOpen = true;
	if (ImGui::Begin("Export Options", &stayOpen))
	{
		bool updatePath = false;
		ImGui::Text("Selected Model %s", opts.model_name.c_str());
		if (ImGui::BeginCombo("Selected Anim", opts.anim_name.c_str()))
		{
			for (int i = 0; i < app->LoadedAnimCount(); i++)
			{
				bool thisOne = i == opts.anim_idx;
				std::string name = app->AnimName(i);
				if (name[0] == '\0') continue;
				if (ImGui::Selectable(name.c_str(), thisOne))
				{
					opts.anim_idx = i;
					opts.anim_name = name.c_str();	// The anim names are pulled from fixed sized buffers which tends to lead to excess "\0".
				}
				if (thisOne)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::Separator();
		updatePath |= ImGui::InputText("Folder", opts.path_buf, opts.path_buf_len);
		updatePath |= ImGui::InputText("File", opts.name_buf, opts.name_buf_len);
		updatePath |= AppendWidget("Append Model Name", &opts.append_model_name);
		updatePath |= AppendWidget("Append Anim Name", &opts.append_anim_name);
		if (updatePath) UpdateFinalPath();
		ImGui::Text("Final file path: %s", opts.final_path.c_str());
		ImGui::Separator();
		ImGui::Checkbox("Skip Geometry", &opts.skip_geom);
		ImGui::SetItemTooltip("If set: Only write skeleton + animation data to file; don't write vertex data.\nUseful to save space if exporting multiple animations.");
		ImGui::SameLine();
		ImGui::Checkbox("Write Textures", &opts.write_texture_files);
		ImGui::SetItemTooltip("If set, writes pmo texture data to separate files.\nIf not set: Skips that.");
		ImGui::Separator();
		if (ImGui::Button("Cancel")) stayOpen = false;
		ImGui::SameLine();
		ImGui::Button("Export");
	}
	ImGui::End();
	return stayOpen;
}

void ExportOp::UpdateFinalPath()
{
	std::string result = opts.path_buf;
	if (!result.ends_with('\\')) result += "\\";
	if (opts.append_model_name & NameAppendMode::Folder)
		result += opts.model_name;
	if (opts.append_anim_name & NameAppendMode::Folder)
	{
		if (opts.append_model_name & NameAppendMode::Folder)
			result += "-";
		result += opts.anim_name;
	}
	if (!result.ends_with('\\')) result += "\\";
	bool emptyName = opts.name_buf[0] == '\0';
	if (!emptyName) result += opts.name_buf;
	if (opts.append_model_name & NameAppendMode::File)
	{
		if (!emptyName) result += "-";
		result += opts.model_name;
	}
	if (opts.append_anim_name & NameAppendMode::File)
	{
		if (!emptyName || (opts.append_model_name & NameAppendMode::File))
			result += "-";
		result += opts.anim_name;
	}
	// TODO
	result += ".fbx";
	opts.final_path = result;
}