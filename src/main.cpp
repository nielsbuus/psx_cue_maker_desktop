#include <vector>
#include <iostream>

using namespace std;

int main(int argc, const char* argv[]) {
  if (argc > 1) {
    cout << "FILE \"" << argv[0] << "\" BINARY\n";
    cout << "TRACK 01 MODE2/2352\n";
    cout << "INDEX 01 00:00:00\n";
  
    for(int track = 1; track < argc; ++track) {
      cout << "FILE \"" << argv[track] << "\" BINARY\n";
      cout << "TRACK 0" << track << " AUDIO\n";
      cout << "INDEX 00 00:00:00\n";
      cout << "INDEX 01 00:02:00\n";
    }
  }
  else {
    cerr << "Usage: PSXCueMaker binfile1 [binfile2] [...] > cuefile.cue\n";
  }
  
  return 0;
}
