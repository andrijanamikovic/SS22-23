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
  this->status = 5; //? 7
  this->cause = no_interrupts;
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
  // reset_terminal();
  if (config_terminal() == false) {
    cout << "Error while configuring the terminal" << endl;
    return;
  }

  while (this->running)
  {
    //   // loop
    ret = this->read_instruction();
  }
  reset_terminal();
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
  if (address + 3 > MEM_MAPPED_REGISTERS && address != term_in_min && address != term_out_min)
  {
    return -1;
  }
  memory[address] = ((char)(0xff & value));
  memory[address + 1] = ((char)(0xff & value >> 8));
  memory[address + 2] = ((char)(0xff & value >> 16));
  memory[address + 3] = ((char)(0xff & value >> 24)); //+ ili -?
  // cout << "Adresa je: " << hex << address << endl; 
  // terminal
  if (address == term_out_min)
  {
    cout << (char)value << flush;
  }
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
  // cout << "PC: " << hex << reg[pc] << endl;
  // this->print_register_output();
  execute(opcode);
  read_char_in();
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
  int disp = (int)(opcode & 0xFFF);                   // 12-0
  // cout << setfill('0') << std::setw(2) << hex << (0xFF & code) << " ";
  // cout << setfill('0') << std::setw(1) << hex << (0xF & regA) << " ";
  // cout << setfill('0') << std::setw(1) << hex << (0xF & regB) << " ";
  // cout << setfill('0') << std::setw(1) << hex << (0xF & regC) << " ";
  // cout << setfill('0') << std::setw(3) << hex << (0xFFF & disp) << " ";
  // cout << endl;
  if ((disp & 0x800 )== 0x800) {
      disp = -(((~disp) + 1) & 0xFFF);
  }
  switch (code)
  {
  case HALT:
    reg[pc] = reg[pc] + 4;
    // cout << "HALT" << endl;
    running = false;
    return 0;
    break;
  case INT:
    // Izaziva softverski prekid
    reg[pc] = reg[pc] + 4;
    // //push pc;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], reg[pc]);
    // push status;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], status);
    //// cause <= 4;
    cause = software_interrupt;
    // //status <= status &(~0x1);
    status = status & (~0x1);
    // //pc <= handle;
    reg[pc] = handler;
    interrupted = true;
    // cout << "INT handler:" << hex << handler << endl;
    break;
  case IRET:
    // pop status
    status = read_dword(reg[sp]);
    reg[sp] = reg[sp] + 4;
    // pop pc
    reg[pc] = reg[pc] + 4;
    enableInterrupt(Tr);
    // cout << "IRET" << endl;
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
    if (regB != pc && regC != pc)
      reg[pc] += 4;
    // cout << "XCHG" << endl;
    break;
  case ADD:
    // regA <= regB + regC
    // cout << "add " << regA << " = " << regB << " + " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] + reg[regC];
    if (regA != pc)
      reg[pc] += 4;
    // cout << "ADD" << endl;
    break;
  case SUB:
    // regA <= regB - regC
    // cout << "sub " << regA << " = " << regB << " - " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] - reg[regC];
    // cout << "SUB" << endl;
    if (regA != pc)
      reg[pc] += 4;
    break;
  case MUL:
    // regA <= regB * regC
    // cout << "mul " << regA << " = " << regB << " * " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] * reg[regC];
    if (regA != pc)
      reg[pc] += 4;
    // cout << "MUL" << endl;
    break;
  case DIV:
    // regA <= regB / regC
    // cout << "div " << regA << " = " << regB << " / " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] / reg[regC];
    // cout << "DIV" << endl;
    if (regA != pc)
      reg[pc] += 4;
    break;
  case NOT:
    // regA <= ~regB
    // cout << "not " << regA << " = ~" << regB << endl;
    if (regA != 0)
      reg[regA] = ~reg[regB];
    // cout << "NOT" << endl;
    if (regA != pc)
      reg[pc] += 4;
    break;
  case AND:
    // regA <= regB & regC
    // cout << "and " << regA << "= " << regB << " & " << regC << endl;

    if (regA != 0)
      reg[regA] = reg[regB] & reg[regC];
    // cout << "AND" << endl;
    if (regA != pc)
      reg[pc] += 4;
    break;
  case OR:
    // regA <= regB | regC
    // cout << "or " << regA << " = " << regB << " | " << regC << endl;
    if (regA != 0)
      reg[regA] = reg[regB] | reg[regC];
    // cout << "OR" << endl;
    if (regA != pc)
      reg[pc] += 4;
    break;
  case XOR:
    // regA <= regB ^ regC
    // cout << "xor " << reg[regA] << " = " << reg[regB] << " ^ " << reg[regC] << endl;
    if (regA != 0)
      reg[regA] = reg[regB] ^ reg[regC];
    // cout << "XOR" << endl;
    if (regA != pc)
      reg[pc] += 4;
    break;
  case SHL:
    // regA <= regB << regC
    // cout << "shl " << reg[regA] << " = " << reg[regB] << " << " << reg[regC] << endl;
    if (regA != 0)
      reg[regA] = reg[regB] << reg[regC];
    // cout << "SHL" << endl;
    if (regA != pc)
      reg[pc] += 4;
    break;
  case SHR:
    // regA <= regB >> regC
    // cout << "shr " << reg[regA] << " = " << reg[regB] << " >> " << reg[regC] << endl;
    if (regA != 0)
      reg[regA] = reg[regB] >> reg[regC];
    // cout << "SHR" << endl;
    if (regA != pc)
      reg[pc] += 4;
    break;
  case CALL:
    // push pc;
    // pc <= mem[regA + regB + d];
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], reg[pc] + 4);
    address = reg[regA] + reg[regB] + disp;
    // cout << "ovde mi disp brate: " << hex << disp << "pc mi je: " << hex << reg[regA] << " a ovaj drugi " << hex << reg[regB] << endl;
    address = address & 0xFFFFFFFF;
    // cout << hex << address << endl;
    val = read_dword(address) & 0xFFFFFFFF;
    reg[pc] = val;
    // cout << "CALL2 procitano sa adrese " << hex << address << " vrednost: " << hex << val << endl;
    break;
  case JMP1:
    // pc <= regA + d
    reg[pc] = (reg[regA] + disp) & 0xFFFFFFFF;
    // cout << "JMP1 preskacem pool: " << hex << reg[regA] + disp << endl;
    break;
  case JMP:
    // pc <= meme[regA + d]
    reg[pc] = read_dword(reg[regA] + disp) & 0xFFFFFFFF;
    // cout << "JMP procitano sa adrese: " << hex << reg[regA] + disp << endl;
    break;
  case BEQ:
    // if (regB == regC)
    // pc <= meme[regB + d]
    if (reg[regB] == reg[regC])
    {
      // cout << "Prosao je BEQ" << endl;
      reg[pc] = read_dword(reg[regA] + disp) & 0xFFFFFFFF;
    }
    else
    {
      reg[pc] += 4;
    }
    // cout << "BEQ2 procitano sa adrese: " << hex << reg[regA] + disp << endl;
    break;
  case BNE:
    // if (regA != regC)
    // pc <= meme[regA + d]
    if (reg[regB] != reg[regC])
    {
      reg[pc] = read_dword(reg[regA] + disp) & 0xFFFFFFFF;
    }
    else
    {
      reg[pc] += 4;
      // cout << "Jednaki su pici dalje" << endl;
    }
    // this->print_register_output();
    // cout << "BNE2 procitano sa adrese: " << hex <<  reg[regA] + disp<< endl;
    break;
  case BGT:
    // if (regA > regC)
    // pc <= regA + d
    if (reg[regB] > reg[regC])
    {
      reg[pc] = read_dword(reg[regA] + disp) & 0xFFFFFFFF;
    }
    else
    {
      reg[pc] += 4;
    }
    // cout << "BGT2 procitano sa adrese: " << hex <<  reg[regA] + disp << endl;
    break;
  case LD1:
    // regA <= regB + D
    if (regA != 0)
      reg[regA] = reg[regB] + disp;
    // cout << "LD1" << endl << hex << reg[regA] << endl;
    if (regA != pc)
      reg[pc] += 4;
    break;
  case LD2:
    // regA <= mem[regB+regC+d]
    if (regA != 0)
      reg[regA] = read_dword(reg[regB] + reg[regC] + disp) & 0xFFFFFFFF;
    if (regA != pc)
      reg[pc] += 4;
    // cout << "LD2 cita sa adrese: " << hex << reg[regB] + reg[regC] + disp << endl;
    // cout << "Upisao je u: "  << regA << " = " << hex << reg[regA] << endl;
    break;
  case ST1:
    // mem[regA + regB + disp] <= regC
    store_dword(reg[regA] + reg[regB] + disp, reg[regC]);
    // cout << "Adresa za smestanje podatka je: " << hex << reg[regA] + reg[regB] + disp << " a podatak je " << hex << reg[regC] << endl;
    // cout << "ST1" << endl;
    reg[pc] += 4;
    break;
  case ST2:
    // mem[mem[regA + regB + disp]] <= regC
    address = read_dword(reg[regA] + reg[regB] + disp) & 0xFFFFFFFF;
    // cout << "Adresa za smestanje podatka je: " << hex << address << " a podatak je " << hex << reg[regC] << endl;
    store_dword(address, reg[regC]);
    // cout << "ST2" << endl;
    reg[pc] += 4;
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
    reg[pc] += 4;
    // cout << "CSRRD" << endl;
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
      // cout << "U handler je sad: " << hex << reg[regB] << endl;
      break;
    case 2:
      cause = reg[regB];
      break;
    default:
      break;
    }
    reg[pc] += 4;
    // cout << "CSRWR" << endl;
    break;
  case PUSH:
    // regA <= regA + D
    // mem32[regA] <= regC
    // if (regA != 0)
    //   reg[regA] = reg[regA] + disp;
    reg[regA] +=  disp;
    store_dword(reg[regA], reg[regC]);
    // emulator_help_file << "PUSH " << endl;
    reg[pc] += 4;
    // cout << "PUSH" << endl;
    break;
  case POP:
    // regC <= mem[regB]
    // regA <= regA + d;
    reg[regC] = read_dword(reg[regA]) & 0xFFFFFFFF;
    reg[regA] += disp;
    // emulator_help_file << "POP " << endl;
    if (regC != pc)
      reg[pc] += 4;
    // cout << "POP" << endl;
    break;
  default:
    interrupted = true;
    // // push status;
    // reg[sp] = reg[sp] - 4;
    // store_dword(reg[sp], status);
    // // push pc;
    // reg[sp] = reg[sp] - 4;
    // store_dword(reg[sp], reg[pc]);
    cout << "Error" << hex << opcode << endl;
    cause = invalid_instruction;
    break;
  }
  if (interrupted)
  {
    interrupt();
    interrupted = false;
    enableInterrupt(Tr);
  } 
  return ret;
}

int Emulator::interrupt()
{
  int ret = 0;
  if (cause == 1)
  {
    running = false;
  }

  // if (cause != 4)
  // {
  //   // //push pc;
  //   reg[sp] = reg[sp] - 4;
  //   store_dword(reg[sp], reg[pc]);
  //   //  //push status;
  //   reg[sp] = reg[sp] - 4;
  //   store_dword(reg[sp], status);
  // }
  // cout << "Usao je u neki prekid: " << hex << cause << endl;
  if (cause == terminal_interrupt && (status & 2 == 0))
  {
    // terminal
    // //push pc;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], reg[pc]);
    //  //push status;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], status);
    terminalIntterupt = false;
    reg[pc] = handler;
    // cout << "Prekid od terminala, nije maskiran" << endl;
    maskInterrupts();
  } else if (cause == timer_interrupt) {
    // cout << "Prekid od timera?" << endl;
    return ret;
  }
  else if (cause != software_interrupt && !(cause == terminal_interrupt && !terminalIntterupt))
  {
    // //push pc;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], reg[pc]);
    //  //push status;
    reg[sp] = reg[sp] - 4;
    store_dword(reg[sp], status);
    reg[pc] = handler;
  } else {
    // emulator_help_file << "Prekid od terminala, maskiran je" << endl;
  }
  return ret;
}

unsigned int Emulator::read_dword(unsigned int address)
{
  unsigned int ret = 0;
  unsigned char byte1;
  unsigned char byte2;
  unsigned char byte3;
  unsigned char byte4;
  // cout << "Adresa za citanje je: " << hex << address << endl;
  byte1 = (unsigned char)(memory[address + 3] & 0xFFFF);
  byte2 = (unsigned char)(memory[address + 2] & 0xFFFF);
  byte3 = (unsigned char)(memory[address + 1] & 0xFFFF);
  byte4 = (unsigned char)(memory[address] & 0xFFFF);
  // cout << "byte1: " << setfill('0') << std::setw(4) << hex << (0xFFFF & byte1) << " ";
  // cout << "byte2: " << setfill('0') << std::setw(4) << hex << (0xFFFF & byte2) << " ";
  // cout << "byte3: " << setfill('0') << std::setw(4) << hex << (0xFFFF & byte3) << " ";
  // cout << "byte4: " << setfill('0') << std::setw(4) << hex << (0xFFFF & byte4) << " ";
  ret = (unsigned int)(((byte1 & 0xFFFF) << 24) | ((byte2 & 0xFFFF) << 16) | ((byte3 & 0xFFFF) << 8) | (byte4 & 0xFFFF));
  // cout << "Procitano: " << hex << ret << endl;
  return ret;
}

//-----------------------------------------------------------Terminal---------------------------------------------------------------

struct termios terminalSettings; // backup for standard input; in order to save settings before emulator execution
/*volatile bool user_interrupt_emulation = false;*/

void backup_stdin_settings()
{
  // https://pubs.opengroup.org/onlinepubs/7908799/xsh/tcsetattr.html
  // At the end of emulation, this function will return initial settings (before emulation)
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminalSettings);
  // TCSAFLUSH option makes that all changes waits for all characters to be written to STDIN (fd), but all characters waiting to be read will be discarded
}

bool Emulator::config_terminal()
{
  // https://pubs.opengroup.org/onlinepubs/7908799/xsh/tcgetattr.html
  // fetch data from STDIN_FILENO to fill backup structure
  if (tcgetattr(STDIN_FILENO, &terminalSettings) < 0)
  {
    cout << "Cannot fetch settings from STDIN_FILENO to save them!" << endl;
    return false;
  }

  // https://man7.org/linux/man-pages/man3/termios.3.html
  // This settings will be changed. In backup_stdi_settings are data from STDIN_FILENO
  // c_lflag - local model:
  // canceled: ECHO - output and ECHONL - output new line on display is canceled because all output should be done in interrupt routine if user programm implies that
  // canceled: ICANON - canonical form is canceled so the current form is noncanonical and no constaint is implemented (c_cc[VMIN] and c_cc[VTIME] need sto be set)
  // canceled: IEXTEN - external mode
  static struct termios changed_settings = terminalSettings;
  changed_settings.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
  // those are set because of canceling ICANON
  // this is called polling read -> read(int fd, void* buf, size_t count) is immediatelly called
  changed_settings.c_cc[VMIN] = 0;  // minimal character that should be waited for read
  changed_settings.c_cc[VTIME] = 0; // timeout between two character that should be waited (in decisecconds)

  // c_cflag - control model:
  // canceled: CSIZE - predefined size of mask (number of bits)
  // set:      CS8 - defined size of mask to 8b
  // canceled: PARENB - parity bits set and check
  changed_settings.c_cflag &= ~(CSIZE | PARENB);
  changed_settings.c_cflag |= CS8;

    // Set function that is finished after exiting from emulator
    // This function will restore settings from STDIN which were set before changes
    if (atexit(backup_stdin_settings) != 0)
    {
        cout <<  "Cannot backup settings to STDIN_FILENO!";
        return false;
    }


  /*atexit(backup_stdin_settings);*/
  // SIGINT is interrupt number for user generate interrupts
  // this will set handler for this interrupt which will restore settings calling backup_stdin_settings function and then set marker user_interrupt_emulation to true
  /*signal(SIGINT, restore_settings_on_user_interrupt);*/

  // at the end fill STDIN_FILENO with changed data
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &changed_settings))
  {
    cout << "Cannot set changed settings to STDIN_FILENO" << endl;
    return false;
  }
  return true;
}

void Emulator::reset_terminal()
{
   struct termios terminalSettings;

    // Fetch the original terminal settings
    if (tcgetattr(STDIN_FILENO, &terminalSettings) < 0) {
        std::cerr << "Error fetching terminal settings" << std::endl;
        return;
    }

    // Restore the original settings
    if (tcsetattr(STDIN_FILENO, TCSANOW, &terminalSettings) < 0) {
        std::cerr << "Error restoring terminal settings" << std::endl;
    }
}

void Emulator::read_char_in()
{
  char c;
  ssize_t bytesRead = read(STDIN_FILENO, &c, 1);
  // cout << "Procitano bitova: " << bytesRead << endl;
  if (bytesRead == 1)
  {
    // cout << "Read from terminal: ";
    // emulator_help_file << c;
    if (store_dword(term_in_min, c) < 0) {
      cout << "ERRRORRRR" << endl;
    }
    terminalIntterupt = true;
    interrupted = true;
    cause = terminal_interrupt;
    // cout << "PREKID OD TERMINALA" << endl;
    // cout << " ..=" << c;
  }
}

void Emulator::maskInterrupts()
{
  status = 7; // lower 3 bits = 1 <=> interrupts disabled
}

void Emulator::enableInterrupt(FlagsStatus interrupt)
{
  // status &= ~interrupt; // set masked bit to 0, enable interrupt
  return;
}