#include "../../inc/emulator.h"
#include <iostream>
#include <iomanip>

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
  this->reg[pc] = START_ADDRESS;
  this->reg[sp] = MEM_MAPPED_REGISTERS;
  memory.assign(MEM_SIZE, 0);
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
  // while (this->running)
  // {
  //   // loop
  ret = this->read_instruction();
  // }
  this->emulator_help_file.close();
  this->print_register_output();
}

int Emulator::load_data_for_emulator()
{
  char c1;
  int filesize = 0;
  int val = 0;
  long i;
  input_data.read((char *)&val, sizeof(val));
  this->emulator_help_file << "Number of sections: " << hex << val << endl;
  for (int k = 0; k < val; k++)
  {
    Segment segment;
    input_data.read((char *)&segment.startAddress, sizeof(segment.startAddress));
    input_data.read((char *)&segment.size, sizeof(segment.size));
    for (int j = 0; j < segment.size; j++)
    {

      input_data.read((char *)&c1, sizeof(c1));
      segment.data.push_back((c1 &0xFFFF));
    }
    segments.push_back(segment);
    this->emulator_help_file << "Start address: " << hex << segment.startAddress << endl;
    this->emulator_help_file << "With data size: " << hex << segment.size << endl;
    // cout << hex << c - 0 << " ";
    // filesize++;
    i = segment.startAddress;
    while (i % 16 != 0)
    {
      i--;
    }

    while (i < segment.startAddress)
    {
      if (i % 16 == 0)
      {
        emulator_help_file << std::setfill('0') << std::setw(8) << hex << (0xFFFFFFFF & i) << ": ";
      }
      emulator_help_file << setfill('0') << std::setw(5) << " ";
      i++;
    }
    for (unsigned char c : segment.data)
    {
      if (i % 16 == 0)
      {
        emulator_help_file << std::setfill('0') << std::setw(8) << hex << (0xFFFFFFFF & i) << ": ";
      }

      emulator_help_file << setfill('0') << std::setw(4) << hex << (0xFFFF & c) << " ";
      if (++i % 16 == 0)
      {
        emulator_help_file << endl;
      }
    }
    emulator_help_file << endl;
  }
  return load_to_memory();
}

int Emulator::load_to_memory()
{
  int ret = 0;
  for (Segment s : segments)
  {
    for (int i = 0; i < s.size; i++)
    {
      if (i + s.startAddress > MEM_MAPPED_REGISTERS)
      {
        cout << "Error to big input file" << endl;
        return -1;
      }
      memory[i + s.startAddress] = s.data[i];
      cout << "Upisujem u memoriju na adresi: " << hex << i+s.startAddress << " = " << hex << setfill('0') << setw(4) <<  s.data[i] << endl;
    }
  }
  cout << memory.size() << endl;
  return ret;
}

void Emulator::print_register_output()
{
  cout << "------------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state:" << endl;

  cout << "r0=0x" << std::setfill('0') << std::setw(8) << hex << reg[0] << " ";
  cout << "r1=0x" << std::setfill('0') << std::setw(8) << hex << reg[1] << " ";
  cout << "r2=0x" << std::setfill('0') << std::setw(8) << hex << reg[2] << " ";
  cout << "r3=0x" << std::setfill('0') << std::setw(8) << hex << reg[3] << " ";
  cout << endl;
  cout << "r4=0x" << std::setfill('0') << std::setw(8) << hex << reg[4] << " ";
  cout << "r5=0x" << std::setfill('0') << std::setw(8) << hex << reg[5] << " ";
  cout << "r6=0x" << std::setfill('0') << std::setw(8) << hex << reg[6] << " ";
  cout << "r7=0x" << std::setfill('0') << std::setw(8) << hex << reg[7] << " ";
  cout << endl;
  cout << "r8=0x" << std::setfill('0') << std::setw(8) << hex << reg[8] << " ";
  cout << "r9=0x" << std::setfill('0') << std::setw(8) << hex << reg[9] << " ";
  cout << "r10=0x" << std::setfill('0') << std::setw(8) << hex << reg[10] << " ";
  cout << "r11=0x" << std::setfill('0') << std::setw(8) << hex << reg[11] << " ";
  cout << endl;
  cout << "r12=0x" << std::setfill('0') << std::setw(8) << hex << reg[12] << " ";
  cout << "r13=0x" << std::setfill('0') << std::setw(8) << hex << reg[13] << " ";
  cout << "r14=0x" << std::setfill('0') << std::setw(8) << hex << reg[14] << " ";
  cout << "r15=0x" << std::setfill('0') << std::setw(8) << hex << reg[15] << " ";
}

long Emulator::read_instruction()
{
  long ret = 0;
  long opcode;
  for (int i = 0; i < 30; i++)
  {
    cout << "Procitano: " << hex << read_dword(reg[pc]) << endl;
    reg[pc] += 4;
  }
  return ret;
}

int Emulator::execute()
{
  int ret = 0;
  return ret;
}

int Emulator::interrupt()
{
  int ret = 0;
  return ret;
}

long Emulator::read_dword(int address)
{
  long ret = 0;
  char byte1;
  char byte2;
  char byte3;
  char byte4;
  cout << "Adressa pc-a" << hex << address << endl
       << endl;
  byte1 = memory[address] & 0xFFFF;
  cout << "byte1: " << byte1 << " ";
  byte2 = memory[++address] & 0xFFFF ;
  cout << "byte2: " << byte2 << " ";
  byte3 = memory[++address] & 0xFFFF ;
  cout << "byte3: " << byte3 << " ";
  byte4 = memory[++address] & 0xFFFF;
  cout << "byte4: " << byte4 << endl;
  cout << hex << (byte4 << 32) << " " << hex << (byte3 << 16) << " " <<hex << ((char)(byte2 << 8))<< " " << hex << (byte1 & 0xff)<< endl;
  ret = (long)((char)(byte4 << 32) | (char)(byte3 << 16) | (char)(byte2 << 8) | (char)(byte1 & 0xff));
  return ret;
}