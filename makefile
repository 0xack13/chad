MAKEFLAGS += --silent

test_file = $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))

all_tests = $(wildcard test/*.chad)

sources = $(wildcard c/*.c c/*/*.c)

objects = $(wildcard target/*.o)

all: make_dir ./target/chad

debug: make_dir ./target/chad-debug

chad: make_dir ./target/chad

make_dir:
	mkdir -p target
	
install:
	make all
	sudo cp ./target/chad /usr/local/bin

uninstall:
	sudo rm /usr/local/bin/chad

install-debug:
	make debug
	sudo cp ./target/chad-debug /usr/local/bin

uninstall-debug:
	sudo rm /usr/local/bin/chad-debug

help:
	make all
	./target/chad -h

version:
	make all
	./target/chad -v

clean: make_dir
	rm target/*

precompile:
	cd rust/libchad && cargo build --quiet --release
	gcc -DPRECOMPILE -w -Os -o ./target/chad $(sources) -lcurl -L./rust/libchad/target/release -llibchad
	strip ./target/chad

	xxd -i ./target/chad > ./target/chad_precompiled.h


./target/chad: make_dir ./c/include/*/*.h $(sources) precompile
	cd rust/libchad && cargo build --quiet --release
	gcc -w -Os -o $@ $(sources) -lcurl -L./rust/libchad/target/release -llibchad
	strip $@
 
./target/chad-debug: make_dir ./c/include/*/*.h $(sources)
	cd rust/libchad && cargo build --quiet
	gcc -w -g -o $@ $(sources) -lcurl -L./rust/libchad/target/debug -llibchad

run: make_dir ./target/chad
	./target/chad run $(test_file)

test: make_dir ./target/chad
	make all
	./target/chad run ./test/test.chad

gdb:
	make install-debug
	gdb -q -x ./gdb.conf