@echo off

SET libgumbo_la_CFLAGS= -Wall -std=c99 
SET libgumbo_la_LDFLAGS= -version-info 1:0:0 -no-undefined
SET libgumbo_la_SOURCES= ^
				src/attribute.c ^
				src/char_ref.c ^
				src/error.c ^
				src/parser.c ^
				src/string_buffer.c ^
				src/string_piece.c ^
				src/tag.c ^
				src/tokenizer.c ^
				src/utf8.c ^
				src/util.c ^
				src/vector.c ^
				../texout.c
SET include_HEADERS= src/gumbo.h src/tag_enum.h


pushd gumbo-parser
gcc %libgumbo_la_CFLAGS% %libgumbo_la_SOURCES% -o ../TeXout
popd

REM echo %Apples%