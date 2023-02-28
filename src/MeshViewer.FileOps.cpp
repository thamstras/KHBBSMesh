#include "MeshViewer.h"
#include "FileTypes/BbsPmo.h"
#include "FileTypes/BbsPam.h"

#include "BBS/CTextureInfo.h"
#include "BBS/CTextureObject.h"

using namespace BBSMesh;

void MeshViewer::OpenModelFile()
{
	std::string path;
	if (!m_fileManager->OpenFileWindow(path, EFileOpenType::FILE_PMO))
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

	if (m_model != nullptr) CloseModelFile();

	m_model = new BBS::CSkelModelObject();
	//m_model = new BBS::CModelObject();
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

	m_guiAnim = new CGUIAnimationProvider(*m_model->skel);
	m_model->SetAnimDriver(new CAnimationDriver(*m_model->skel));
	m_model->animDriver->SetAnimation(m_guiAnim);
	SetAnimType(AnimType::GuiAnim);
}

void MeshViewer::CloseModelFile()
{
	if (m_anims != nullptr) CloseAnimFile();

	if (m_model != nullptr) delete m_model;
	m_model = nullptr;
	for (auto tex : m_textures) if (tex != nullptr) delete tex;
	m_textures.clear();
	m_rootRenderContext->render.textureLibrary->PruneTextures();
	if (m_guiAnim) delete m_guiAnim;
}

void MeshViewer::OpenAnimFile()
{
	if (m_model == nullptr)
		return;
	
	std::string path;
	if (!m_fileManager->OpenFileWindow(path, EFileOpenType::FILE_PAM))
		return;

	std::ifstream fs = std::ifstream(path, std::ifstream::binary);
	if (!fs.is_open())
	{
		// TODO: ShowError("Failed to open file");
		return;
	}

	PamFile file = PamFile::ReadPamFile(fs, 0);


	// TODO: CHECK SKELETON

	if (m_anims != nullptr) CloseAnimFile();
	m_anims = new BBS::CBBSAnimSet(file, *m_model->skel);
	SetAnimType(AnimType::FileAnim);
}

void MeshViewer::CloseAnimFile()
{
	if (currAnim == AnimType::FileAnim) SetAnimType(AnimType::GuiAnim);

	if (m_anims != nullptr) delete m_anims;
	m_anims = nullptr;
}