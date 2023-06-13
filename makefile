INCLUDE_DIR = inc
SOURCE_DIR = src
TEST_DIR = tests
ASSEMBLER_DIR = ./src/assembler
LINKER_DIR = ./src/linker
EMULATOR_DIR = ./src/emulator


PROGRAM = assembler


# LINKER_SCRIPT = linker_script.ld

all : AS LD EM

AS_all: AS1 AS2 AS3 AS4 AS5 AS6 AS7

AS1: assembler.o
	./assembler -o main.o main.s

AS2: assembler.o
	./assembler -o handlers.o handlers.s

AS3: assembler.o
	./assembler -o math.o math.s

AS4: assembler.o
	./assembler -o isr_reset.o isr_reset.s

AS5: assembler.o
	./assembler -o isr_terminal.o isr_terminal.s

AS6: assembler.o
	./assembler -o isr_timer.o isr_timer.s

AS7: assembler.o
	./assembler -o isr_software.o isr_software.s

AS8: assembler.o
	./assembler -o test.o test.s

assembler.o:	$(ASSEMBLER_DIR)/main.cpp $(ASSEMBLER_DIR)/assembler.cpp $(ASSEMBLER_DIR)/tables.cpp
	g++ -g -gdwarf-2 -fdebug-prefix-map==../ -o assembler  $(ASSEMBLER_DIR)/main.cpp $(ASSEMBLER_DIR)/assembler.cpp $(ASSEMBLER_DIR)/tables.cpp

LD: linker.o
	./linker -hex -o program.hex ivt.o math.o main.o isr_reset.o isr_terminal.o isr_timer.o isr_user0.o

linker.o:	$(LINKER_DIR)/main.cpp $(LINKER_DIR)/linker.cpp $(ASSEMBLER_DIR)/tables.cpp
	g++ -g -o linker  $(LINKER_DIR)/main.cpp $(LINKER_DIR)/linker.cpp $(ASSEMBLER_DIR)/tables.cpp

EM:


assembler.tab.c ./inc/assembler.tab.h: $(ASSEMBLER_DIR)/assembler.y
	bison -d $(ASSEMBLER_DIR)/assembler.y

assembler.tab.o: assembler.tab.c
	g++ -c $(<) -o $(@)

lex.yy.c: $(ASSEMBLER_DIR)/assembler.l ./inc/assembler.tab.h
	flex $(<)

lex.yy.o: lex.yy.c
	g++ -c $(<) -o $(@)


clean :
	rm *_linker.txt

clean_as:
	rm *.o*

run_as:
	./src/assembler/assembler
