#include "../../inc/linker.h"
#include <iostream>
#include <regex>

using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 2) {
    cout << "Not enough input arguments" << endl;
    return -1;
  }

  list<string> input_files;
  string output_file;
  bool has_output_file = false;
  bool hex_output = false;

  string command = argv[0];
  string option;


  if ("./linker" == command) {

    regex place_reg("-place=([a-zA-Z][_A-Za-z0-9]*)@0x([0-9A-Fa-f]+)"); //Za B nivo

    for (int i = 1; i< argc; i++){
      option = argv[i];

      if (option == "-o"){
        has_output_file = true;
      } else if ( option == "-hex") {
        hex_output = true;
      } else if (has_output_file)
        {
            cout << "Output file is: "<< option << endl;
            output_file = option;
            has_output_file = false;
        }
        else
        {
            cout << "Input file is: "<< option << endl;
            input_files.push_back(option);
        }
    }
    Linker* linker = new Linker();
    linker->link(hex_output, input_files, output_file);
  } else {
    return -1;
  }
    return 0;

}