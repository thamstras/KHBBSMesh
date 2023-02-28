#include "MeshViewer.h"

int main(int argc, char** argv)
{
	BBSMesh::MeshViewer app = BBSMesh::MeshViewer(argc, argv);
	try
	{
		app.Run();
		return EXIT_SUCCESS;
	}
	catch (std::exception ex)
	{
		std::cerr << "FATAL: " << ex.what() << std::endl;
		// TODO: MessageBox(something)?
		CFileManager::ShowMessageBox(ex.what());
		return EXIT_FAILURE;
	}
}