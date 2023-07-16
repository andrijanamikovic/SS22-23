#ifndef EMULATOR_H
#define EMHULATOR_H

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include "tables.h"

//Za B nivo imam i terminal

using namespace std;

const long MEM_MAPPED_REGISTERS = 0xFFFFFF00; 
const long MAPPED_REG_SIZE = 256;
const long START_ADDRESS = 0x40000000;
const unsigned int term_out_min = 0xFFFFFF00;
const unsigned int term_out_max = 0xFFFFFF03;
const unsigned int term_in_min = 0xFFFFFF04;
const unsigned int term_in_max = 0xFFFFFF07;
const unsigned long MEM_SIZE = 0xFFFFFFFF;
const int BYTE = 1;
const int WORD = 2;
const int DWORD = 4;

class Emulator {

enum Registers {
  r0 = 0,
  r1 = 1,
  r2 = 2,
  r3 = 3,
  r4 = 4,
  r5 = 5,
  r6 = 6,
  r7 = 7,
  r8 = 8,
  r9 = 9,
  r10 = 10, 
  r11 = 11,
  r12 = 12,
  r13 = 13,
  r14 = 14,
  r15 = 15,
  pc = r15,
  sp = r14
};

enum FlagsStatus {
  Tr = 1,
  Tl = 1 << 1,
  I = 1 << 2
};

enum AddressType {
  imm,
  mem_dir,
  reg_dir,
  reg_ind,
  pc_rel
};

enum Interrupts {
  invalid_instruction = 1,
  timer_interrupt = 2,
  terminal_interrupt = 3,
  software_interrupt = 4,
  no_interrupts = 0
};

struct Segment{
  long startAddress;
  int size;
  vector<char> data;
};



public:
  Emulator(string input_file);
  void emulate();
  void stop();
private:

  string input_file;
  ifstream input_data;
  ofstream emulator_help_file;
  vector<Segment> segments;
  vector<char> memory;
  bool running;
  bool interrupted = false;
  int reg[16]; 
  int status;
  int handler;
  int cause;
  int temp;
  int load_data_for_emulator();
  int load_to_memory();
  int read_instruction();
  int execute(int opcode);
  int interrupt();
  void print_register_output();
  unsigned int read_dword(unsigned int address);
  int store_dword(unsigned int address, int value);
  int get_reg_num(unsigned char reg);

  //terminal
  bool config_terminal();
  void reset_terminal();
  void read_char_in();
  void convert_to_bigendian(vector<char> *data);

  void maskInterrupts();
  void enableInterrupt(FlagsStatus interrupt);
  bool terminalIntterupt = false;
};

#endif