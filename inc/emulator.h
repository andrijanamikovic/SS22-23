#ifndef EMULATOR_H
#define EMHULATOR_H

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <termios.h>

//Za B nivo imam i terminal

using namespace std;

const int MEM_MAPPED_REGISTERS = 0xFFFFFF00; 
const int MAPPED_REG_SIZE = 256;
const int START_ADDRESS = 0x40000000;
const int term_out_min = 0xFFFFFF00;
const int term_out_max = 0xFFFFFF03;
const int term_in_min = 0xFFFFFF04;
const int term_in_max = 0xFFFFFF07;
const int MEM_SIZE = 0x100000;

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
  sp = r14,
  status,
  handler,
  cause
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

};



public:
  Emulator(string input_file);
  void emulate();
  void stop();
private:

  string input_file;
  ifstream input_data;
  ofstream emulator_help_file;
  char memory[MEM_SIZE];
  bool running;
  int load_data_for_emulator();
};

#endif