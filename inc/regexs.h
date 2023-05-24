#ifndef REGEXS_H
#define REGEXS_H

#include <regex>
#include <string>
using namespace std;

class Regexes
{
public:
  // \s je [\f\n\r\t\v]
  // \S je [^\f\n\r\t\v] znaci da nema  te white space karaktere

  //------------------------------------------------Helpers----------------------------------------------------
  string symbol_regex = "[a-zA-Z][_A-Za-z0-9]*";
  string literal_regex = "(-?[0-9]+)|(0[xX][0-9a-fA-F]+)";
  string register_regex = "(\\%r(1[0-5]|[0-9])|sp|pc)";

  //-------------------------------------------------Common symbols----------------------------------------------
  regex tab = regex("[ \t]+");
  regex symbol = regex("[a-zA-Z][A-Za-z0-9]*");
  regex integer = regex("-?[0-9]+");
  regex hex = regex("0[xX][0-9a-fA-F]+");
  regex newline = regex("\\n");
  regex more_spaces = regex(" {2,}");
  regex any = regex("(.*)");
  regex comma_space = regex(" ?, ?");
  regex label_space = regex(" ?: ?");
  regex boundary_space = regex("^( *)([^ ].*[^ ])( *)$");
  regex square_bracket_space = regex("\\[\\s*(.*?)\\s*\\]");
  regex disp_space = regex("(.*?)\\s*\\+\\s*(.*?)");
  regex comments = regex("#.*");

  //--------------------------------------------------------------------------Directives------------------------

  regex extern_dir = regex("\\.extern (" + symbol_regex + "(," + symbol_regex + ")*)$");
  regex global_dir = regex("\\.global (" + symbol_regex + "(," + symbol_regex + ")*)$");
  regex section_dir = regex("\\.section (" + symbol_regex + ")$");
  // regex word_dir = regex("\\.word ((" + symbol_regex + "|" + literal_regex + ")(,(" + symbol_regex + "|" + literal_regex + "))*)$");
  regex word_dir =  regex("^\\.word ((" + symbol_regex + "|" + literal_regex + ")(,(" + symbol_regex + "|" + literal_regex + "))*)$");
  regex skip_dir = regex("\\.skip (" + literal_regex + ")$");
  regex ascii_dir = regex("\\.ascii (\"" + symbol_regex + "\")$");
  regex end_dir = regex("\\.end$");

  //-----------------------------------------------------------------------Instructions-------------------------

  regex halt_inst = regex ("^(halt)$");
  regex int_inst = regex("^(int) " + register_regex);
  regex iret_inst = regex("^(iret)$");
  regex push_inst = regex("(push) " + register_regex);
  regex pop_inst = regex("^(pop) " + register_regex);
  regex xchg_inst = regex("^(xchg) " + register_regex + "," + register_regex);
  regex add_inst = regex("^(add) " + register_regex + "," + register_regex);
  regex sub_inst = regex("^(sub) " + register_regex + "," + register_regex);
  regex mul_inst = regex("^(mul) " + register_regex + "," + register_regex);
  regex div_inst = regex("^(div) " + register_regex + "," + register_regex);
  regex not_inst = regex("^(not) " + register_regex);
  regex and_inst = regex("^(and) " + register_regex + "," + register_regex);
  regex or_inst = regex("^(or) " + register_regex + "," + register_regex);
  regex xor_inst = regex("^(xor) " + register_regex + "," + register_regex);
  regex shl_inst = regex("^(shl) " + register_regex + "," + register_regex);
  regex shr_inst = regex("^(shr) " + register_regex + "," + register_regex);
  regex ld_inst = regex("^(ld) (.*)");
  regex st_inst = regex("^(st) (.*)");
  regex call_inst = regex("^(call) (.*)");
  regex ret_inst = regex("^(ret)$");
  regex jmp_inst = regex("^(jmp) (.*)");
  regex beq_inst = regex("^(beq) (.*)");
  regex bne_inst = regex("^(bne) (.*)");
  regex bgt_inst = regex("^(bgt) (.*)");
  regex csrrd_inst = regex("^(csrrd) (.*)");
  regex csrwr_inst = regex("^(csrwr) (.*)");

  //--------------------------------------------------Operand---------------------------------------------------
  
  regex reg_regex = regex(register_regex);
  regex sym_regex = regex(symbol_regex);
  regex lit_regex = regex(literal_regex);

  regex absolute_operand_regex = regex("^\\$(" +literal_regex + "|" + symbol_regex + ")$");
  regex memory_direct_operand_regex = regex("^(" + literal_regex + "|" + symbol_regex + ")$");
  regex reg_dir_regex = regex("^("+register_regex+")$");
  regex register_absolute_operand_regex = regex("(^\\[" + register_regex + "\\])$");
  regex register_relative_operand_regex = regex("(^\\[" + register_regex + "\\s*\\+\\s*(" + symbol_regex + "|" + literal_regex + ")\\])$");
  // string operand_regex = absolute_operand_regex + "|" + memory_direct_operand_regex + "|" + pc_relative_operand_regex  + "|" + reg_dir_regex + "|" + register_absolute_operand_regex + "|" + register_relative_operand_regex;
  
  // //-----------------------------------------------Jump----------------------------------------------------------
  
  regex jump_absolute_operand_regex = regex("(^" + literal_regex +"|"+ symbol_regex+")$");
  
  //------------------------------------------------Other--------------------------------------------------------
  regex only_label = regex("^(" + symbol_regex + "):$");
  regex label_with_data = regex("^(" + symbol_regex + "):(.*)$");
};
#endif
