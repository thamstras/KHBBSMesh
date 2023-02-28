#include <Windows.h>
#include "Common.h"

#include "CFileManager.h"

#pragma comment(lib, "Comdlg32") // Needed for GetOpenFileName

namespace fs = std::filesystem;

std::filesystem::path CFileManager::FindExeDir()
{
	std::filesystem::path exePath;
	
	// Finding the exe path is a non-trival problem
	wchar_t pathBuf[MAX_PATH];
	GetModuleFileName(NULL, pathBuf, MAX_PATH);
	exePath = std::filesystem::path(pathBuf).parent_path();

	return exePath;
}

void CFileManager::ShowInitFailMessageBox()
{
	MessageBox(NULL, TEXT("FATAL ERROR: Failed to find resources path\nPlease check working directory."), TEXT("FATAL ERORR"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
}

LPCTSTR GetFileTypeFilter(EFileOpenType type)
{

	switch (type)
	{
	case EFileOpenType::FILE_PMO:
		return TEXT("PMO Files\0*.pmo\0All Files\0*.*\0\0");
	case EFileOpenType::FILE_PAM:
		return TEXT("PAM Files\0*.pam\0All Files\0*.*\0\0");
	case EFileOpenType::FILE_ANY:
	default:
		return TEXT("All Files\0*.*\0\0");
	}
}

bool CFileManager::OpenFileWindow(std::string& out_path, EFileOpenType type)
{
	fs::path orignalPath = fs::current_path();

	TCHAR filename[MAX_PATH];
	OPENFILENAME ofn;

	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = GetFileTypeFilter(type);
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = resourcesDir.c_str();
	ofn.lpstrTitle = TEXT("Select a File");
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = TEXT("pmp");

	if (GetOpenFileName(&ofn))
	{
		//out_filePath = std::string(filename);
		fs::path openPath = fs::path(filename);
		openPath = fs::absolute(openPath);
		out_path = openPath.string();
		fs::current_path(orignalPath);
		std::cout << "[FS] Open file: " << openPath << std::endl;
		return true;
	}
	else
	{
		switch (CommDlgExtendedError())
		{
		case CDERR_DIALOGFAILURE: std::cout << "[FS] CDERR_DIALOGFAILURE\n"; break;
		case CDERR_FINDRESFAILURE: std::cout << "[FS] CDERR_FINDRESFAILURE\n"; break;
		case CDERR_INITIALIZATION: std::cout << "[FS] CDERR_INITIALIZATION\n"; break;
		case CDERR_LOADRESFAILURE: std::cout << "[FS] CDERR_LOADRESFAILURE\n"; break;
		case CDERR_LOADSTRFAILURE: std::cout << "[FS] CDERR_LOADSTRFAILURE\n"; break;
		case CDERR_LOCKRESFAILURE: std::cout << "[FS] CDERR_LOCKRESFAILURE\n"; break;
		case CDERR_MEMALLOCFAILURE: std::cout << "[FS] CDERR_MEMALLOCFAILURE\n"; break;
		case CDERR_MEMLOCKFAILURE: std::cout << "[FS] CDERR_MEMLOCKFAILURE\n"; break;
		case CDERR_NOHINSTANCE: std::cout << "[FS] CDERR_NOHINSTANCE\n"; break;
		case CDERR_NOHOOK: std::cout << "[FS] CDERR_NOHOOK\n"; break;
		case CDERR_NOTEMPLATE: std::cout << "[FS] CDERR_NOTEMPLATE\n"; break;
		case CDERR_STRUCTSIZE: std::cout << "[FS] CDERR_STRUCTSIZE\n"; break;
		case FNERR_BUFFERTOOSMALL: std::cout << "[FS] FNERR_BUFFERTOOSMALL\n"; break;
		case FNERR_INVALIDFILENAME: std::cout << "[FS] FNERR_INVALIDFILENAME\n"; break;
		case FNERR_SUBCLASSFAILURE: std::cout << "[FS] FNERR_SUBCLASSFAILURE\n"; break;
		default: std::cout << "[FS] File open cancelled.\n";
		}

		fs::current_path(orignalPath);
		return false;
	}
}

// TODO: An actual dialog. COM needed?
bool CFileManager::OpenDirectoryWindow(std::string& out_path)
{
	fs::path originalPath = fs::current_path();

	fs::path outPath = resourcesDir;
	outPath.append("export");

	out_path = outPath.string();

	return true;
}