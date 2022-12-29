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
	for (auto tex : m_textures) if (tex != nullptr) delete tex;
	m_textures.clear();
	m_rootRenderContext->render.textureLibrary->PruneTextures();

	m_model = new BBS::CModelObject();
	m_model->LoadPmo(newFile, false);

	 auto texMap = std::unordered_map<std::string, BBS::CTextureInfo*>();
	for (auto& texInfo : newFile.textures)
	{
		fs.seekg(texInfo.dataOffset);
		Tm2File texFile = Tm2File::ReadTm2File(fs, texInfo.dataOffset);
		BBS::CTextureObject* texObj = new BBS::CTextureObject();
		texObj->LoadTM2(texFile);
		texObj->CreateTexture();

		m_textures.push_back(new BBS::CTextureInfo(texInfo, texObj));
		texMap.emplace(std::string(texInfo.resourceName, 12), m_textures.back());
		// TODO: When the CTextureObject is deleted it deletes the CTexture which breaks the shared_ptr
		// So we'll need to rejig CTextureObject
		//m_rootRenderContext->render.textureLibrary->AddTexture(std::string(texInfo.resourceName), texObj->texture);
	}

	m_model->LinkExtTextures(texMap);
	m_model->BuildMesh();
	//m_model->LinkExtTextures(m_rootRenderContext->render.textureLibrary.get());
}