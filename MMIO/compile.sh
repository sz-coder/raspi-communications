#!/bin/bash -eux

rm -f example

gcc \
	-Wall \
	-Wextra \
	-Wpedantic \
	src/*.c \
	example.c \
	-o example
