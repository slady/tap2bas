#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

const int IN_BUFFER_SIZE = 1024;
const int OUT_BUFFER_SIZE = 0x10000;

const char *errIn = "Cannot open input file: %s\n";
const char *errOut = "Cannot open output file: %s\n";
const char *errMem = "Error: Not enough memory!\n";
const char *errProg = "Internal program error!\n";
const char *errSyntLineNum = "Syntax error: Line number\n";
const char *errPos = "Location: line %d column %d\n";

const int FIRST_COMMAND = 0xA5;
const char *COMMAND_LIST[] = { "RND", "INKEY$", "PI", "FN", "POINT", "SCREEN$", "ATTR", "AT", "TAB", "VAL$", "CODE",
  "VAL", "LEN", "SIN", "COS", "TAN", "ASN", "ACS", "ATN", "LN", "EXP", "INT", "SQR", "SGN", "ABS", "PEEK", "IN",
  "USR", "STR$", "CHR$", "NOT", "BIN", "OR", "AND", "<=", ">=", "<>", "LINE", "THEN", "TO", "STEP", "DEFFN", "CAT",
  "FORMAT", "MOVE", "ERASE", "OPEN#", "CLOSE#", "MERGE", "VERIFY", "BEEP", "CIRCLE", "INK", "PAPER", "FLASH", "BRIGHT", "INVERSE", "OVER", "OUT",
  "LPRINT", "LLIST", "STOP", "READ", "DATA", "RESTORE", "NEW", "BORDER", "CONTINUE", "DIM", "REM", "FOR", "GOTO", "GOSUB", "INPUT", "LOAD",
  "LIST", "LET", "PAUSE", "NEXT", "POKE", "PRINT", "PLOT", "RUN", "SAVE", "RANDOMIZE", "IF", "CLS", "DRAW", "CLEAR", "RETURN", "COPY" };
// changed key words:
// "DEF FN", "OPEN #", "CLOSE #", "GO TO", "GO SUB"

typedef enum { LINE_START, LINE_NUMBER, COMMAND_EXPECTED, OK } state;

void writeByte(const int value, int *checksumPointer, FILE *fout)
{
  putc(value, fout);
  *checksumPointer ^= value;
}

void writeDoubleByte(const int size, int *checksumPointer, FILE *fout)
{
  writeByte(size & 0xFF, checksumPointer, fout);
  writeByte((size / 0x100) & 0xFF, checksumPointer, fout);
}

void writeFile(const int size, const char *name, const char *buf, FILE *fout)
{
  int len = strlen(name), i, checksum;

  // writing the header packet
  writeByte(0x13, &checksum, fout);
  checksum = 0;
  writeByte(0, &checksum, fout);
  writeByte(0, &checksum, fout);
  writeByte(0, &checksum, fout);

  for (i = 0; i < len && i < 10; i++) {
    writeByte(name[i], &checksum, fout);
  }

  for (i = len; i < 10; i++) {
    writeByte(' ', &checksum, fout);
  }

  writeDoubleByte(size, &checksum, fout);
  writeDoubleByte(0x8000, &checksum, fout);
  writeDoubleByte(size, &checksum, fout);
  writeByte(checksum, &checksum, fout);

  // writing data packet, because the header ended
  writeDoubleByte(size + 2, &checksum, fout);
  checksum = 0;
  writeByte(0xFF, &checksum, fout);

  // writing raw data
  for (i = 0; i < size; i++) {
    writeByte(buf[i], &checksum, fout);
  }

  writeByte(checksum, &checksum, fout);
}

void clearBuf(char *ibuf)
{
  memset(ibuf, 0, IN_BUFFER_SIZE);
}

int parseFile(FILE *fin, char *obuf, char *ibuf)
{
  // c - character read from the input file
  // p - position in obuf
  // b - position in ibuf
  // l - line reading position in the input file
  // x - column reading position in the input file
  // n - a number being read from the input file
  int c, p = 0, b = 0, l = 1, x = 1, n;
  // s - reading state
  state s = LINE_START;

  clearBuf(ibuf);

  while ((c = getc(fin)) != EOF) {
    switch (s) {
      case LINE_START:
	if (isdigit(c)) {
	  s = LINE_NUMBER;
	  ibuf[b++] = c;
	} else if (!isspace(c)) {
	  fputs(errSyntLineNum, stderr);
	  fprintf(stderr, errPos, l, x);
	  return -1;
	}
	break;

      case LINE_NUMBER:
	if (isdigit(c)) {
	  ibuf[b++] = c;
	} else if (isspace(c)) {
	  sscanf(ibuf, "%d", &n);
	  obuf[p++] = n & 0xFF;
	  obuf[p++] = (n / 0x100) & 0xFF;
	  clearBuf(ibuf);
	  b = 0;
	  s = COMMAND_EXPECTED;
	} else {
	  fputs(errSyntLineNum, stderr);
	  fprintf(stderr, errPos, l, x);
	  return -1;
	}
	break;

      default:
	fputs(errProg, stderr);
	return -1;
	break;
    }

    // line counting
    if ('\n' == c) {
      l++;
      x = 1;
    } else {
      x++;
    }
  }

  return p;
}

void process(char *name, FILE *fin, FILE *fout)
{
  int result;
  char *obuf = malloc(OUT_BUFFER_SIZE + 10), *ibuf;

  if (NULL == obuf) {
    fputs(errMem, stderr);
    return;
  }

  ibuf = malloc(IN_BUFFER_SIZE);

  if (NULL == ibuf) {
    free(obuf);
    fputs(errMem, stderr);
    return;
  }

  result = parseFile(fin, obuf, ibuf);

  free(ibuf);

  if (result >= 0) {
    writeFile(result, name, obuf, fout);
  }

  free(obuf);
/*
  if (1 + 1 > 0) return;

    if (LINE_START == p) {
      if ('U' == c) {
	break;
      }

      lineNo = c * 256 + getc(fin);
      fprintf(fout, "%5d", lineNo);
      // lineLen
      getc(fin);
      getc(fin);
      p = LINE_READ;
      continue;
    }

    if (0x0D == c) {
      // new command line
      fputs("\n", fout);
      p = LINE_START;
    } else if (0x0E == c) {
      // read number and throw it away
      getc(fin);
      getc(fin);
      getc(fin);
      getc(fin);
      getc(fin);
    } else if (c < ' ') {
      esc(c, fout);
    } else if (c < '~') {
      //writeByte(c, fout);
    } else if (c < FIRST_COMMAND) {
      esc(c, fout);
    } else {
      //writeByte('\n', fout);
      //writeByte(' ', fout);
      //fputs(COMMAND_LIST[c - FIRST_COMMAND], fout);
      //writeByte(' ', fout);
    }
  }*/
}

int main(int ac, char **argv)
{
  FILE *fin, *fout;

  switch (ac) {
    case 2:
      process(argv[1], stdin, stdout);
      break;

    case 3:
      fin = fopen(argv[2], "r");

      if (NULL == fin) {
	fprintf(stderr, errIn, argv[2]);
	return 2;
      }

      process(argv[1], fin, stdout);
      fclose(fin);
      break;

    case 4:
      fin = fopen(argv[2], "r");

      if (NULL == fin) {
	fprintf(stderr, errIn, argv[2]);
	return 2;
      }

      fout = fopen(argv[3], "w");

      if (NULL == fout) {
	fprintf(stderr, errOut, argv[3]);
	return 3;
      }

      process(argv[1], fin, fout);
      fclose(fin);
      fclose(fout);
      break;

    default:
      printf("Use: bas2tap <name> [<input.bas> [<output.tap]]\n");
      return 1;
      break;
  }

  return 0;
}
