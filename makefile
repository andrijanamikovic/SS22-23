INCLUDE_DIR = inc
SOURCE_DIR = src
TEST_DIR_A = /A
ASSEMBLER_DIR = ./src/assembler
LINKER_DIR = ./src/linker
EMULATOR_DIR = ./src/emulator
TEST_DIR_B = /B

TEST_DIR = $(TEST_DIR_A)

PROGRAM = assembler

# LINKER_SCRIPT = linker_script.ld

all : AS LD EM

AS_all: AS1 AS2 AS3 AS4 AS5 AS6 
ASB: AS1 AS2 AS4 AS5

AS1: assembler.o
	./assembler -o main.o $(TEST_DIR)/main.s

AS2: assembler.o
	./assembler -o handler.o $(TEST_DIR)/handler.s

AS3: assembler.o
	./assembler -o math.o $(TEST_DIR)/math.s

AS4: assembler.o
	./assembler -o isr_terminal.o $(TEST_DIR)/isr_terminal.s

AS5: assembler.o
	./assembler -o isr_timer.o $(TEST_DIR)/isr_timer.s

AS6: assembler.o
	./assembler -o isr_software.o $(TEST_DIR)/isr_software.s

AS7: assembler.o
	./assembler -o test.o $(TEST_DIR)/test.s

assembler.o:	$(ASSEMBLER_DIR)/main.cpp $(ASSEMBLER_DIR)/assembler.cpp $(ASSEMBLER_DIR)/tables.cpp
	g++ -g -gdwarf-2 -fdebug-prefix-map==../ -o assembler  $(ASSEMBLER_DIR)/main.cpp $(ASSEMBLER_DIR)/assembler.cpp $(ASSEMBLER_DIR)/tables.cpp

LD: linker.o
	./linker -hex -o program.hex main.o handler.o math.o isr_terminal.o isr_timer.o isr_software.o

LD1: linker.o
	./linker -hex -place=my_code@0x40000000  -o program.hex main.o

LD2: linker.o
	./linker \
	-hex \
  -place=my_code@0x40000000 -place=math@0xF0000000 \
  -o program.hex \
  handler.o math.o main.o isr_terminal.o isr_timer.o isr_software.o
LD3: linker.o
	./linker \
	-relocatable \
	-o program.o main.o handler.o math.o isr_terminal.o isr_timer.o isr_software.o

LD4: linker.o
	./linker -hex -place=my_code@0x40000000 -o programtest.hex test.o

LDB: linker.o
	./linker \
	-hex \
  -place=my_code@0x40000000 \
  -o program.hex \
  main.o isr_terminal.o isr_timer.o handler.o

linker.o:	$(LINKER_DIR)/main.cpp $(LINKER_DIR)/linker.cpp $(ASSEMBLER_DIR)/tables.cpp
	g++ -g -o linker  $(LINKER_DIR)/main.cpp $(LINKER_DIR)/linker.cpp $(ASSEMBLER_DIR)/tables.cpp

EM: emulator.exe
	./emulator program.hex

EM4: emulator.exe
	./emulator programtest.hex

emulator.exe: $(EMULATOR_DIR)/main.cpp $(EMULATOR_DIR)/emulator.cpp $(ASSEMBLER_DIR)/tables.cpp
	g++ -g -o emulator $(EMULATOR_DIR)/main.cpp $(EMULATOR_DIR)/emulator.cpp $(ASSEMBLER_DIR)/tables.cpp

assembler.tab.c ./inc/assembler.tab.h: $(ASSEMBLER_DIR)/assembler.y
	bison -d $(ASSEMBLER_DIR)/assembler.y

assembler.tab.o: assembler.tab.c
	g++ -c $(<) -o $(@)

lex.yy.c: $(ASSEMBLER_DIR)/assembler.l ./inc/assembler.tab.h
	flex $(<)

lex.yy.o: lex.yy.c
	g++ -c $(<) -o $(@)


clean :
	rm  linker.txt *_linker.txt *.hex hex_help.txt reloc_help.txt

clean_as:
	rm *.o*

run_as:
	./src/assembler/assembler
