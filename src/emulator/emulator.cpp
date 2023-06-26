#include "../../inc/emulator.h"
#include <iostream>

Emulator::Emulator(string input_file)
{
  this->input_file = input_file;
  input_data.open(input_file);

  if (!input_data.is_open())
  {
    cout << "Error while opening binary input: " + input_file << endl;
    return;
  }

  this->emulator_help_file.open("emulator.txt", ios::out);
  if (!emulator_help_file.is_open())
  {
    cout << "Error while opening help file output: " << endl;
    return;
  }
  this->running = true;
  this->emulate();
}

void Emulator::stop()
{
  this->running = false;
  return;
}

void Emulator::emulate()
{
  int ret = 0;
  this->emulator_help_file << "Loading data for emulator" << endl;
  ret = this->load_data_for_emulator();
  if (ret < 0)
  {
    cout << "Error while reading data for emulator" << endl;
    return;
  }
  // while (this->running){
  //   //loop
  // }
  this->emulator_help_file.close();
}

int Emulator::load_data_for_emulator(){
  unsigned char c;
  int filesize = 0;
  while(1){
    input_data.read((char*)&c, sizeof(c));
    if (c == '\n') break;
    //cout << hex << c - 0 << " ";
    filesize++;
  }

  if (filesize < MEM_SIZE - 256){
    input_data.seekg(0, input_data.beg);
    input_data.read((char*)&memory, filesize);
  }else{
    cout << "Error to big input file" << endl;
    return -1;
  }
  // for (int i=0; i<filesize; i++){
  //   cout << hex << (unsigned char)memory[i] - 0 << " ";
  // }  
  return 0;
}