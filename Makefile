INCLUDES=-Iparsing/inc -Iutils/inc
OBJECTS=obj
BINARIES=bin
C-FLAGS=-Wall

parse-test: clean bin/parse_test
	@./$(BINARIES)/parse_test

$(BINARIES)/parse_test: obj/parse_test.o obj/symbol.o obj/generic_parser.o obj/tracked_file.o obj/error.o
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
