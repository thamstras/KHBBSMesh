#pragma once
#include <string>

enum NameAppendMode
{
	No = 0,
	Folder = (1 << 0),
	File = (1 << 1),
	Both = Folder | File
};

struct ExportOptions
{
	int model_idx = 0;
	std::string model_name;
	int anim_idx = 0;
	std::string anim_name;
	static constexpr int path_buf_len = 260;
	char path_buf[path_buf_len] = ".\\resources\\export";
	static constexpr int name_buf_len = 60;
	char name_buf[name_buf_len] = "";
	std::string model_format_id = "fbx";
	NameAppendMode append_model_name = NameAppendMode::Both;
	NameAppendMode append_anim_name = NameAppendMode::No;
	std::string final_folder;
	std::string final_path;
	// TODO: bone scale handling mode
	//bool add_extra_root_bone = false; We don't need this, root motion is working now.
	bool skip_geom = false;
	bool write_texture_files = true;
};