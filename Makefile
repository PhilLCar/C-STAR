INCLUDES=-Iparsing/inc -Iutils/inc -Icisor/inc
OBJECTS =obj
BINARIES=bin
C-FLAGS =-Wall
ifdef mem
	C-FLAGS += -DMEMORY_WATCH
endif
ifdef debug
	C-FLAGS += -DDEBUG -g
endif

UTILS  =$(OBJECTS)/error.o   \
        $(OBJECTS)/array.o   \
        $(OBJECTS)/strings.o \
				$(OBJECTS)/raw.o     \
				$(OBJECTS)/file.o    \
        $(OBJECTS)/diagnostic.o
PARSING=$(OBJECTS)/symbol.o         \
        $(OBJECTS)/parser.o         \
        $(OBJECTS)/tracked_file.o   \
        $(OBJECTS)/tracked_string.o \
        $(OBJECTS)/bnf.o            \
        $(OBJECTS)/ast.o
CISOR  =$(OBJECTS)/preprocessor.o \
        $(OBJECTS)/macro.o        \
				$(OBJECTS)/intermediate.o

cisor: $(BINARIES)/cisor

test: parse-test array-test bnf-test ast-test preprocessor-test

debug: parse-debug array-debug bnf-debug ast-debug preprocessor-debug

%-test: $(BINARIES)/%_test
	@./$<

%-debug: 
	@rm -f $(BINARIES)/$*_test
	@make debug=1 mem=1 $(BINARIES)/$*_test

$(BINARIES)/%: $(OBJECTS)/%.o $(UTILS) $(PARSING) $(CISOR)
	@gcc $(C-FLAGS) $^ -o $@

$(OBJECTS)/%.o: parsing/src/%.c
	@gcc $(C-FLAGS) $(INCLUDES) -c $< -o $@

$(OBJECTS)/%.o: utils/src/%.c
	@gcc $(C-FLAGS) $(INCLUDES) -c $< -o $@

$(OBJECTS)/%.o: cisor/src/%.c
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
