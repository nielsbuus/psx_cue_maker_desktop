#include <cstdlib>

#include <vector>
#include <sstream>
#include <experimental/optional>

#include "windows.h"
#include "Objbase.h"
#include "Shlobj.h"

using namespace std;
using namespace std::experimental;

TCHAR select_directory_path[MAX_PATH];

// TODO: A lot of error detection and handling is missing.

class COM {
public:
  COM() {
  if (CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED) != S_OK)
    throw runtime_error("Failed to initialize COM.");
  }
  ~COM() {
    CoUninitialize();
  }
};

optional<string> select_directory() {
  BROWSEINFO browseinfo {};
  browseinfo.pszDisplayName = select_directory_path;
  browseinfo.lpszTitle = "Please select directory containing the bin files.";
  browseinfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
  PIDLIST_ABSOLUTE idlist = SHBrowseForFolder(&browseinfo);
  if (idlist == nullptr) {
    return {};
  }
  else {
    if (!SHGetPathFromIDList(idlist, select_directory_path)) {
      CoTaskMemFree(idlist);
      throw runtime_error("SHGetPathFromIDList failed.");
    };
    CoTaskMemFree(idlist);
    return string(select_directory_path);
  }
}

vector<string> find_bin_files(string directory) {
  vector<string> result;
  
  string search_path(directory);
  search_path += "\\*.bin";

  WIN32_FIND_DATA search_data {};
  HANDLE search_handle = FindFirstFile(search_path.c_str(), &search_data);
  if (GetLastError() != ERROR_FILE_NOT_FOUND) {
    result.emplace_back(search_data.cFileName);
    while (FindNextFile(search_handle, &search_data)) {
      result.emplace_back(search_data.cFileName);
    }
    FindClose(search_handle);
  }
  
  return result;
}

string generate_cuesheet(vector<string> files) {
  stringstream ss;

  if (files.size() > 0) {
    ss << "FILE \"" << files.at(0) << "\" BINARY\n";
    ss << "  TRACK 01 MODE2/2352\n";
    ss << "    INDEX 01 00:00:00\n";
    for(size_t track = 1; track < files.size(); ++track) {
      ss << "FILE \"" << files.at(track) << "\" BINARY\n";
      ss << "  TRACK 0" << track << " AUDIO\n";
      ss << "   INDEX 00 00:00:00\n";
      ss << "   INDEX 01 00:02:00\n";
    };
  }
  
  return ss.str();
}

string generate_cuesheet_filename(vector<string> files) {
  return "Cuesheet.cue";
}

bool file_exists(string filename) {
  WIN32_FIND_DATA find_data {};
  HANDLE search_handle = FindFirstFile(filename.c_str(), &find_data);
  auto error_code = GetLastError();
  FindClose(search_handle);

  if (error_code == ERROR_FILE_NOT_FOUND) return false;
  if (error_code == ERROR_NO_MORE_FILES) return true;
  throw error_code;
}

int main(int argc, const char* argv[]) {
  try {
    COM com;

    auto dir = select_directory();
    if (dir) {
      auto files = find_bin_files(*dir);
      if (files.empty()) throw runtime_error("No bin files found in the selected directory.");
      auto cuesheet = generate_cuesheet(files);
      string filename = generate_cuesheet_filename(files);
      string full_filename = *dir + '\\' + filename;

      bool write_file = true;
      if (file_exists(full_filename) &&
        (MessageBox(nullptr, "A cuesheet file already exists. Do you want to overwrite it?", "File exists", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDNO)) {
          write_file = false;
        }

      if (write_file) MessageBox(nullptr, cuesheet.c_str(), "Cuesheet", MB_OK | MB_ICONINFORMATION);
    }
  }
  catch (const exception& e) {
    MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
    return EXIT_FAILURE;
  }
  catch (DWORD error_code) {
    LPSTR buffer = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
      error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&buffer), 0, nullptr);
    MessageBox(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
    LocalFree(buffer);
  }
  
  return EXIT_SUCCESS;
}
