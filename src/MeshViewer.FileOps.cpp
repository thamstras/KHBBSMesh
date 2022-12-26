#include "MeshViewer.h"
#include "FileTypes/BbsPmo.h"

#include "BBS/CTextureInfo.h"
#include "BBS/CTextureObject.h"

using namespace BBSMesh;

void MeshViewer::OpenFile()
{
	std::string path;
	if (!m_fileManager->OpenFileWindow(path))
		return;

	std::ifstream fs = std::ifstream(path, std::ifstream::binary);
	if (!fs.is_open())
	{
		// TODO: ShowError("Failed to open file");
		return;
	}

	PmoFile newFile = PmoFile::ReadPmoFile(fs, 0);
	if (!(newFile.hasMesh0() || newFile.hasMesh1()))
	{
		// TODO: ShowError("Error reading file");
		return;
	}

	if (m_model != nullptr) delete m_model;
	m_model = new BBS::CModelObject();
	m_model->LoadPmo(newFile, true);

	
}