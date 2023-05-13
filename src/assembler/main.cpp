#include "../../inc/assembler.h"

int main(int argc, char *argv[])
{
  if (argc < 2) {
    cout << "Not enough input arguments" << endl;
    return -1;
  }

  string input_file;
  string output_file;
  string command = argv[0];
  string option = argv[1];

  if ("./assembler" == command) {

    if (option != "-o") {
        cout << "Wrong option format" << endl;
        return -1;
    }

    if (argc < 3) {
      cout << "Input and output file are not specificated" << endl;
      return -1;
    } else if (argc < 4) {
      cout << "Input file is not specificated" << endl;
      return -1;
    }
    
    input_file = argv[3];
    output_file = argv[2];

    Assembler* assembler = new Assembler();

    assembler->assemble(input_file, output_file);
  }


  return 0;
}