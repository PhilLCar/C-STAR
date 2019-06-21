INCLUDES=-Iparsing/inc -Iutils/inc
OBJECTS =obj
BINARIES=bin
C-FLAGS =-Wall

ERROR_O=$(OBJECTS)/error.o
PARSE_O=$(OBJECTS)/symbol.o $(OBJECTS)/generic_parser.o $(OBJECTS)/tracked_file.o
BNF_O  =$(OBJECTS)/bnf_parser.o

test: parse-test bnf-test

parse-test: clean $(BINARIES)/parse_test
	@./$(BINARIES)/parse_test

bnf-test: clean $(BINARIES)/bnf_test
	@./$(BINARIES)/bnf_test

$(BINARIES)/parse_test: $(OBJECTS)/parse_test.o $(ERROR_O) $(PARSE_O)
	@gcc $(C-FLAGS) $^ -o $@

$(BINARIES)/bnf_test:  $(OBJECTS)/bnf_test.o $(ERROR_O) $(PARSE_O) $(BNF_O)
	@gcc $(C-FLAGS) $^ -o $@

$(OBJECTS)/%.o: parsing/src/%.c
	@gcc $(C-FLAGS) $(INCLUDES) -c $< -o $@

$(OBJECTS)/%.o: test/%.c
	@gcc $(C-FLAGS) $(INCLUDES) -c $< -o $@

clean-objects:
	@rm -f $(OBJECTS)/*

clean-binaries:
	@rm -f $(BINARIES)/*

clean: clean-objects clean-binaries
	@find . -name "*~" -delete
	@find . -name "#*#" -delete
