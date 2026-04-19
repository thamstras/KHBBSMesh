#pragma once
#include <optional>
#include "ExportOptions.h"

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

		void UpdateFinalPath();

		static std::optional<ExportOptions> prevExpOpts;
	};
}