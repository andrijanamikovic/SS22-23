#include "../../inc/emulator.h"

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cout << "Not enough input arguments" << endl;
    return -1;
  }

  string input_file;
  string command = argv[0];
  input_file = argv[1];

  if ("./emulator" == command)
  {

    Emulator *emulator = new Emulator(input_file);
  }
  else
  {
    cout << "Wrong command for starting an emulator" << endl;
    return -1;
  }

  return 0;
}