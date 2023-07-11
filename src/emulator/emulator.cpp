#include "../../inc/emulator.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string.h>

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
  this->reg[16] = {0};
  this->status = 7; //?
  this->cause = 0;
  this->handler = 0;
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
  while (this->running)
  {
    //   // loop
    ret = this->read_instruction();
  }
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
      segment.data.push_back((c1 & 0xFFFF));
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
      emulator_help_file << setfill('0') << std::setw(3) << " ";
      i++;
    }
    for (unsigned char c : segment.data)
    {
      if (i % 16 == 0)
      {
        emulator_help_file << std::setfill('0') << std::setw(8) << hex << (0xFFFFFFFF & i) << ": ";
      }

      emulator_help_file << setfill('0') << std::setw(2) << hex << (0xFF & c) << " ";
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
    // cout << "Segment sa pocetnom adresom: " << hex << s.startAddress << " " << setfill('0') << std::setw(2) << hex << (0xFF & s.data[0]) << endl;
    for (int i = 0; i < s.size; i++)
    {
      if (i + s.startAddress > MEM_MAPPED_REGISTERS)
      {
        cout << "Error to big input file" << endl;
        return -1;
      }
      memory[i + s.startAddress] = s.data[i];
    }
  }
  // cout << memory.size() << endl;
  return ret;
}

int Emulator::store_dword(unsigned int address, int value)
{
  if (address + 3 > MEM_MAPPED_REGISTERS)
  {
    return -1;
  }
  memory[address + 3] = ((char)(0xff & value));
  memory[address + 2] = ((char)(0xff & value >> 8));
  memory[address + 1] = ((char)(0xff & value >> 16));
  memory[address] = ((char)(0xff & value >> 24)); //+ ili -?

  return 0;
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
  cout << endl;
  cout << "handler=0x" << std::setfill('0') << std::setw(8) << hex << handler << " ";
  cout << "cause=0x" << std::setfill('0') << std::setw(8) << hex << cause << " ";
  cout << "status=0x" << std::setfill('0') << std::setw(8) << hex << status << " ";
  cout << endl;
}

int Emulator::read_instruction()
{
  int ret = 0;
  int opcode;
  opcode = read_dword(reg[pc]);
  cout << "PC: " << hex << reg[pc] << endl;
  this->print_register_output();
  reg[pc] += 4;
  execute(opcode);
  return ret;
}

int Emulator::execute(int opcode)
{
  int ret = 0;
  unsigned int address = 0;
  int val;
  unsigned char code = (unsigned char)((opcode >> 24) & 0xFF); // 32-24
  unsigned char regA = (unsigned char)((opcode >> 20) & 0xF);  // 24-20
  unsigned char regB = (unsigned char)((opcode >> 16) & 0xF);  // 20-16
  unsigned char regC = (unsigned char)((opcode >> 12) & 0xF);  // 16-12
  int disp = (unsigned int)(opcode & 0xFFF);          // 12-0
  cout << setfill('0') << std::setw(2) << hex << (0xFF & code) << " ";
  cout << setfill('0') << std::setw(1) << hex << (0xF & regA) << " ";
  cout << setfill('0') << std::setw(1) << hex << (0xF & regB) << " ";
  cout << setfill('0') << std::setw(1) << hex << (0xF & regC) << " ";
  cout << setfill('0') << std::setw(3) << hex << (0xFFF & disp) << " ";
  cout << endl;
  bool interrupted = false;
  switch (code)
  {
  case HALT:
    emulator_help_file << "HALT" << endl;
    running = false;
    return 0;
    break;
  case INT:
    // Izaziva softverski prekid
    // //push status;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], status);
    // //push pc;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], reg[pc]);
    //// cause <= 4;
    cause = 4;
    // //status <= status &(~0x1);
    status = status & (~0x1);
    // //pc <= handle;
    reg[pc] = handler;
    // interrupted = true;
    cout << "INT " << endl;
    break;
  case XCHG:
    // temp <= regB
    // regB <= regC
    // gerC <= temp
    this->emulator_help_file << "XCHG " << regB << " i " << regC << endl;
    temp = reg[regB];
    if (regB != 0)
      reg[regB] = reg[regC];
    if (regC != 0)
      reg[regC] = temp;
    cout << "XCHG" << endl;
    break;
  case ADD:
    // regA <= regB + regC
    this->emulator_help_file << "add " << regA << " = " << regB << " + " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] - reg[regC];
    cout << "ADD" << endl;
    break;
  case SUB:
    // regA <= regB - regC
    this->emulator_help_file << "sub " << regA << " = " << regB << " - " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] - reg[regC];
    cout << "SUB" << endl;
    break;
  case MUL:
    // regA <= regB * regC
    this->emulator_help_file << "mul " << regA << " = " << regB << " * " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] * reg[regC];
    cout << "MUL" << endl;
    break;
  case DIV:
    // regA <= regB / regC
    this->emulator_help_file << "div " << regA << " = " << regB << " / " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] / reg[regC];
    cout << "DIV" << endl;
    break;
  case NOT:
    // regA <= ~regB
    this->emulator_help_file << "not " << regA << " = ~" << regB << endl;
    if (regA != 0)
      reg[regA] = ~reg[regB];
    cout << "NOT" << endl;
    break;
  case AND:
    // regA <= regB & regC
    this->emulator_help_file << "and " << regA << "= " << regB << " & " << regC << endl;

    if (regA != 0)
      reg[regA] = reg[regB] & reg[regC];
    cout << "AND" << endl;
    break;
  case OR:
    // regA <= regB | regC
    this->emulator_help_file << "or " << regA << " = " << regB << " | " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] | reg[regC];
    cout << "OR" << endl;
    break;
  case XOR:
    // regA <= regB ^ regC
    this->emulator_help_file << "xor " << regA << " = " << regB << " ^ " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] ^ reg[regC];
    cout << "XOR" << endl;
    break;
  case SHL:
    // regA <= regB << regC
    this->emulator_help_file << "shl " << regA << " = " << regB << " << " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] << reg[regC];
    cout << "SHL" << endl;
    break;
  case SHR:
    // regA <= regB >> regC
    this->emulator_help_file << "shr " << regA << " = " << regB << " >> " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] >> reg[regC];
    cout << "SHR" << endl;
    break;
  case CALL:
    // push pc;
    // pc <= mem[regA + regB + d];
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], reg[pc]);
    address = reg[regA] + reg[regB] + disp;
    cout << "ovde mi disp brate: " << hex << disp << "pc mi je: " << hex << reg[regA] << " a ovaj drugi " << hex << reg[regB] << endl;
    address = address & 0xFFFFFFFF;
    cout << hex << address << endl;
    val = read_dword(address) & 0xFFFFFFFF;
    reg[pc] = val;
    cout << "CALL2 procitano sa adrese " << hex << address << " vrednost: " << hex << val << endl;
    break;
  case JMP:
    // pc <= meme[regA + d]
    reg[pc] = read_dword(reg[regA] + disp) & 0xFFFFFFFF;
    cout << "JMP2 procitano sa adrese: " << hex << reg[regA] + disp << endl;
    break;
  case BEQ:
    // if (regA == regC)
    // pc <= meme[regA + d]
    if (reg[regA] == reg[regC])
    {
      reg[pc] = reg[regA] + disp;
    }
    cout << "BEQ2 procitano sa adrese: " << hex << regA + disp << endl;
    break;
  case BNE:
    // if (regA != regC)
    // pc <= meme[regA + d]
    if (reg[regA] != reg[regC])
    {
      reg[pc] = reg[regA] + disp;
    }
    cout << "BNE2 procitano sa adrese: " << hex << regA + disp << endl;
    break;
  case BGT:
    // if (regA > regC)
    // pc <= regA + d
    if (reg[regA] > reg[regC])
    {
      reg[pc] = reg[regA] + disp;
    }
    cout << "BGT2 procitano sa adrese: " << hex << regA + disp << endl;
    break;
  case LD1:
    // regA <= regB + D
    if (regA != 0)
      reg[regA] = reg[regB] + disp;
    cout << "LD1" << endl;
    break;
  case LD2:
    // regA <= mem[regB+regC+d]
    if (regA != 0)
      reg[regA] = read_dword(reg[regB] + reg[regC] + disp) & 0xFFFFFFFF;
    cout << "LD2 cita sa adrese: " << hex << reg[regB] + reg[regC] + disp << endl;
    cout << "Upisao je u: " << hex << regA << " = " << hex << reg[regA] << endl;
    break;
  case ST1:
    // mem[regA + regB + disp] <= regC
    store_dword(reg[regA] + reg[regB] + disp, reg[regC]);
   cout << "Adresa za smestanje podatka je: " << hex << reg[regA] + reg[regB] + disp << " a podatak je " << hex << reg[regC] << endl;
    cout << "ST1" << endl;
    break;
  case ST2:
    // mem[mem[regA + regB + disp]] <= regC
    address = read_dword(reg[regA] + reg[regB] + disp) & 0xFFFFFFFF;
    cout << "Adresa za smestanje podatka je: " << hex << address << " a podatak je " << hex << reg[regC] << endl;
    store_dword(address, reg[regC]);
    cout << "ST2" << endl;
    break;
  case CSRRD:
    // regA <= csr[B]
    if (regA != 0)
      switch (regB)
      {
      case 0:
        reg[regA] = status;
        break;
      case 1:
        reg[regA] = handler;
        break;
      case 2:
        reg[regA] = cause;
        break;
      default:
        break;
      }
    cout << "CSRRD" << endl;
    break;
  case CSRWR:
    // csr[A] <= regB
    switch (regA)
    {
    case 0:
      status = reg[regB];
      break;
    case 1:
      handler = reg[regB];
      cout << "U handler je sad: " << hex << reg[regB] << endl;
      break;
    case 2:
      cause = reg[regB];
      break;
    default:
      break;
    }
    cout << "CSRWR" << endl;
    break;
  case PUSH:
    // regA <= regA + D
    // mem32[regA] <= regC
    if (regA != 0)
      reg[regA] = reg[regA] + disp;
    store_dword(reg[regA], reg[regC]);
    cout << "PUSH" << endl;
    break;
  case POP:
    // regA <= mem[regB]
    // regB <= regB + d;
    if (regA != 0)
      reg[regA] = read_dword(regB) & 0xFFFFFFFF;
    regB = regB + disp;
    cout << "POP" << endl;
    break;
  default:
    interrupted = true;
    // push status;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], status);
    // push pc;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], reg[pc]);
    emulator_help_file << "Error" << endl;
    cause = 1;
    return 1;
    break;
  }
  if (interrupted)
  {
    interrupt();
  }
  return ret;
}

int Emulator::interrupt()
{
  int ret = 0;
  cout << "Poziv prekidne rutine: " << endl;
  return ret;
}

unsigned int Emulator::read_dword(unsigned int address)
{
  unsigned int ret = 0;
  unsigned char byte1;
  unsigned char byte2;
  unsigned char byte3;
  unsigned char byte4;
  cout << "Adresa za citanje je: " << hex << address << endl;
  byte1 = (unsigned char)(memory[address] & 0xFFFF);
  byte2 = (unsigned char)(memory[address + 1] & 0xFFFF);
  byte3 = (unsigned char)(memory[address + 2] & 0xFFFF);
  byte4 = (unsigned char)(memory[address + 3] & 0xFFFF);
  cout << "byte1: " << setfill('0') << std::setw(4) << hex << (0xFFFF & byte1) << " ";
  cout << "byte2: " <<  setfill('0') << std::setw(4) << hex << (0xFFFF & byte2) << " ";
  cout << "byte3: " <<  setfill('0') << std::setw(4) << hex << (0xFFFF & byte3) << " ";
  cout << "byte4: " <<  setfill('0') << std::setw(4) << hex << (0xFFFF & byte4) << " ";
  ret = (unsigned int)(((byte1 & 0xFFFF) << 24) | ((byte2 & 0xFFFF) << 16) | ((byte3 & 0xFFFF) << 8) | (byte4 & 0xFFFF));
  cout << "Procitano: " << hex << ret << endl;
  return ret;
}