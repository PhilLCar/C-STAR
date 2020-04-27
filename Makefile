INCLUDES=-Iparsing/inc -Iutils/inc
OBJECTS =obj
BINARIES=bin
C-FLAGS =-Wall
ifdef mem
	C-FLAGS += -DMEMORY_WATCH
endif

UTILS  =$(OBJECTS)/error.o $(OBJECTS)/array.o $(OBJECTS)/strings.o $(OBJECTS)/diagnostic.o
PARSING=$(OBJECTS)/symbol.o $(OBJECTS)/generic_parser.o $(OBJECTS)/tracked_file.o $(OBJECTS)/bnf.o $(OBJECTS)/ast.o

test: parse-test array-test bnf-test ast-test

%-test: $(BINARIES)/%_test
	@./$<

$(BINARIES)/%: $(OBJECTS)/%.o $(UTILS) $(PARSING)
	@gcc $(C-FLAGS) $^ -o $@

$(OBJECTS)/%.o: parsing/src/%.c
	@gcc $(C-FLAGS) $(INCLUDES) -c $< -o $@

$(OBJECTS)/%.o: utils/src/%.c
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
