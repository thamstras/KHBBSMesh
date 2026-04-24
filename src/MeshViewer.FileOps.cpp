#include "MeshViewer.h"
#include "FileTypes/BbsPmo.h"
#include "FileTypes/BbsPam.h"

#include "BBS/CTextureInfo.h"
#include "BBS/CTextureObject.h"
#include "AssimpExporter.h"

using namespace BBSMesh;

void MeshViewer::OpenModelFile()
{
	std::string path;
	if (!m_fileManager->OpenFileWindow(path, EFileOpenType::FILE_PMO))
		return;

	std::ifstream fs = std::ifstream(path, std::ifstream::binary);
	if (!fs.is_open())
	{
		CFileManager::ShowMessageBox("Failed to open file!");
		return;
	}

	OpenModelFile(path, fs);
}

void MeshViewer::OpenModelFile(std::string path, std::ifstream& fs)
{
	std::streamoff base = fs.tellg();

	PmoFile newFile;
	try
	{
		newFile = PmoFile::ReadPmoFile(fs, base);
	}
	catch (std::exception ex)
	{
		CFileManager::ShowMessageBox(ex.what());
		return;
	}

	if (!(newFile.hasMesh0() || newFile.hasMesh1()))
	{
		CFileManager::ShowMessageBox("Error reading file");
		return;
	}

	if (m_model != nullptr) CloseModelFile();

	m_model = new BBS::CSkelModelObject();
	m_model->LoadPmo(newFile, false);

	auto texMap = std::unordered_map<std::string, BBS::CTextureInfo*>();
	for (auto& texInfo : newFile.textures)
	{
		std::streamoff texBase = base + texInfo.dataOffset;
		fs.seekg(texBase);
		Tm2File texFile = Tm2File::ReadTm2File(fs, texBase);
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

	modelName = std::filesystem::path(path).stem().string();

	m_rootRenderContext->debug.hide_flags = 0;
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
	m_guiAnim = nullptr;
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
		CFileManager::ShowMessageBox("Failed to open file!");
		return;
	}

	OpenAnimFile(path, fs);
}

void MeshViewer::OpenAnimFile(std::string path, std::ifstream& fs)
{
	if (m_model == nullptr)
		return;

	std::streamoff base = fs.tellg();

	PamFile file;
	try
	{
		file = PamFile::ReadPamFile(fs, base);
	}
	catch (std::exception ex)
	{
		CFileManager::ShowMessageBox(ex.what());
		return;
	}

	// TODO: CHECK SKELETON

	if (m_anims != nullptr) CloseAnimFile();
	m_anims = new BBS::CBBSAnimationProvider(file, *m_model->skel);
	SetAnimType(AnimType::FileAnim);
}

void MeshViewer::CloseAnimFile()
{
	if (currAnim == AnimType::FileAnim) SetAnimType(AnimType::GuiAnim);

	if (m_anims != nullptr) delete m_anims;
	m_anims = nullptr;
}

void MeshViewer::OpenArcFile()
{
	std::string path;
	if (!m_fileManager->OpenFileWindow(path, EFileOpenType::FILE_ARC))
		return;

	if (m_arcLoader != nullptr)
	{
		delete m_arcLoader;
		m_arcLoader = nullptr;
	}

	try
	{
		m_arcLoader = new ArcLoader(path);
	}
	catch (std::exception ex)
	{
		CFileManager::ShowMessageBox(ex.what());
		return;
	}

	using namespace std::placeholders;
	m_arcLoader->Attatch(std::bind(&MeshViewer::LoadFromArcFile, this, _1, _2));
}

void MeshViewer::LoadFromArcFile(std::string name, std::ifstream& fs)
{
	std::filesystem::path namep = std::filesystem::path(name);
	auto ext = namep.extension();
	if (ext == ".pmo")
		OpenModelFile(name, fs);
	else if (ext == ".pam")
		OpenAnimFile(name, fs);
}

void MeshViewer::ExportAnimFile()
{
	//AssimpAnimExporter::ExportSkelScene(m_model, m_anims, modelName, std::string(pathBuf), *currFormat);
}