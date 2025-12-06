#!/bin/bash
gcc -o bin/$1 src/$1.c
read -p "Run build? (Y/N): " answer

case "$answer" in
	[Nn]* ) break;;
	* ) ./bin/shell;;
	esac

