parse-test: clean bin/parse_test
	@./bin/parse_test

bin/parse_test: obj/parse_test.o obj/generic_parser.o obj/tracked_file.o obj/error.o
	@gcc -Wall obj/parse_test.o obj/generic_parser.o obj/tracked_file.o obj/error.o -o bin/parse_test

obj/parse_test.o:
	@gcc -Wall -c -Iparsing/inc -Iutils/inc test/parse_test.c -o obj/parse_test.o

obj/generic_parser.o:
	@gcc -Wall -c -Iparsing/inc parsing/src/generic_parser.c -o obj/generic_parser.o

obj/tracked_file.o:
	@gcc -Wall -c -Iparsing/inc parsing/src/tracked_file.c -o obj/tracked_file.o

obj/error.o:
	@gcc -Wall -c -Iparsing/inc -Iutils/inc parsing/src/error.c -o obj/error.o

clean-objects:
	@rm -f obj/*

clean-binaries:
	@rm -f bin/*

clean: clean-objects clean-binaries
	@find . -name "*~" -delete
	@find . -name "#*#" -delete
