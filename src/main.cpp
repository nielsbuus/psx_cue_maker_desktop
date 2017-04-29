#include <vector>
#include <iostream>

#include "Objbase.h"
#include "Shlobj.h"

using namespace std;

TCHAR select_directory_path[MAX_PATH];

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

// Opens directory selection dialog box.
// Returns true when a directory is selected, returns false when cancelled.
bool select_directory() {
  BROWSEINFO browseinfo {};
  browseinfo.pszDisplayName = select_directory_path;
  browseinfo.lpszTitle = "Please select directory containing the bin files.";
  browseinfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
  PIDLIST_ABSOLUTE idlist = SHBrowseForFolder(&browseinfo);
  if (idlist == nullptr) {
    return false;
  }
  else {
    if (!SHGetPathFromIDList(idlist, select_directory_path)) {
      CoTaskMemFree(idlist);
      throw runtime_error("SHGetPathFromIDList failed.");
    };
    CoTaskMemFree(idlist);
    return true;
  }
}

// TODO: A lot of error detection and handling is missing.

int main(int argc, const char* argv[]) {  
  COM com;  
  if (select_directory()) {
    string search_path(select_directory_path);
    search_path += "\\*.bin";

    WIN32_FIND_DATA search_data {};
    int track_count = 0;
    HANDLE search_handle = FindFirstFile(search_path.c_str(), &search_data);
    cout << "FILE \"" << search_data.cFileName << "\" BINARY\n";
    cout << "  TRACK 01 MODE2/2352\n";
    cout << "    INDEX 01 00:00:00\n";
    while (FindNextFile(search_handle, &search_data)) {
      ++track_count;
      cout << "FILE \"" << search_data.cFileName << "\" BINARY\n";
      cout << "  TRACK 0" << track_count << " AUDIO\n";
      cout << "   INDEX 00 00:00:00\n";
      cout << "   INDEX 01 00:02:00\n";
    }
    FindClose(search_handle);
  }
  
  return 0;
}
