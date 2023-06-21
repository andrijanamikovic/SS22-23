#include "../../inc/linker.h"
#include <iostream>
#include <regex>

using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cout << "Not enough input arguments" << endl;
    return -1;
  }

  list<string> input_files;
  list<string> places;
  string output_file;
  bool has_output_file = false;
  bool hex_output = false;
  bool relocatable_output = false;
  bool error_flag = false;

  string command = argv[0];
  string option;
  string mode = argv[1];
  smatch match;

  if ("./linker" == command)
  {

    regex place_reg("-place=([a-zA-Z][_A-Za-z0-9]*)@0x([0-9A-Fa-f]+)");
    regex input_reg("^([a-zA-Z][_A-Za-z0-9]*)\\.o$");

    if (mode == "-hex")
    {
      hex_output = true;
    }
    else if (mode == "-relocatable")
    {
      relocatable_output = true;
    }
    else if (!hex_output && !relocatable_output)
    {
      error_flag = true;
      cout << "Error ouput mode must be choosen: -hex or -relocatable" << endl;
    }
    else if (hex_output && relocatable_output)
    {
      error_flag = true;
      cout << "-hex and -relocatable options are not allowed together" << endl;
    }
    for (int i = 2; i < argc; i++)
    {
      option = argv[i];

      if (option == "-o")
      {
        has_output_file = true;
      }
      else if (has_output_file)
      {
        cout << "Output file is: " << option << endl;
        output_file = option;
        has_output_file = false;
      }
      else if (regex_match(option, match, place_reg))
      {
        places.push_back(option);
      }
      else if (regex_match(option, match, input_reg))
      {
        cout << "Input file is: " << option << endl;
        input_files.push_back(option);
      }
    }
    if (!error_flag)
    {
      Linker *linker = new Linker();
      linker->link(hex_output, relocatable_output, input_files, places, output_file);
    }
  }
  else
  {
    return -1;
  }
  if (error_flag)
    return -1;
  return 0;
}