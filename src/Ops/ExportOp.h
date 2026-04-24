#pragma once
#include <optional>
#include "ExportOptions.h"
//#include <thread>
#include <future>

namespace BBSMesh
{
	// Because circular defs
	class MeshViewer;

	class ExportOp
	{
	public:
		ExportOp(MeshViewer* app);
		bool gui_window();

	private:
		MeshViewer* app;
		ExportOptions opts;
		int stage = 0;
		std::future<void> work;

		void UpdateFinalPath();
		void StartExport();

		static std::optional<ExportOptions> prevExpOpts;
	};
}