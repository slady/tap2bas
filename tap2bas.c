#include <stdio.h>

const char *errIn = "Cannot open input file: %s\n";
const char *errOut = "Cannot open output file: %s\n";

const int FIRST_COMMAND = 0xA5;
const char *COMMAND_LIST[] = { "RND", "INKEY$", "PI", "FN", "POINT", "SCREEN$", "ATTR", "AT", "TAB", "VAL$", "CODE",
  "VAL", "LEN", "SIN", "COS", "TAN", "ASN", "ACS", "ATN", "LN", "EXP", "INT", "SQR", "SGN", "ABS", "PEEK", "IN",
  "USR", "STR$", "CHR$", "NOT", "BIN", "OR", "AND", "<=", ">=", "<>", "LINE", "THEN", "TO", "STEP", "DEF FN", "CAT",
  "FORMAT", "MOVE", "ERASE", "OPEN #", "CLOSE #", "MERGE", "VERIFY", "BEEP", "CIRCLE", "INK", "PAPER", "FLASH", "BRIGHT", "INVERSE", "OVER", "OUT",
  "LPRINT", "LLIST", "STOP", "READ", "DATA", "RESTORE", "NEW", "BORDER", "CONTINUE", "DIM", "REM", "FOR", "GO TO", "GO SUB", "INPUT", "LOAD",
  "LIST", "LET", "PAUSE", "NEXT", "POKE", "PRINT", "PLOT", "RUN", "SAVE", "RANDOMIZE", "IF", "CLS", "DRAW", "CLEAR", "RETURN", "COPY", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", };

typedef enum { LINE_START, LINE_READ } position;

void esc(int ch, FILE *fout)
{
  fprintf(fout, "`%02x;", ch);
}

void process(FILE *fin, FILE *fout)
{
  int c, lineNo;
  position p = LINE_START;

  fseek(fin, 24L, SEEK_SET);

  while ((c = getc(fin)) != EOF) {
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
      putc(c, fout);
    } else if (c < FIRST_COMMAND) {
      esc(c, fout);
    } else {
      //putc('\n', fout);
      putc(' ', fout);
      fputs(COMMAND_LIST[c - FIRST_COMMAND], fout);
      putc(' ', fout);
    }
  }
}

int main(int ac, char **argv)
{
  FILE *fin, *fout;

  switch (ac) {
    case 1:
      process(stdin, stdout);
      break;

    case 2:
      fin = fopen(argv[1], "r");

      if (NULL == fin) {
       fprintf(stderr, errIn, argv[1]);
       return 2;
      }

      process(fin, stdout);
      fclose(fin);
      break;

    case 3:
      fin = fopen(argv[1], "r");

      if (NULL == fin) {
       fprintf(stderr, errIn, argv[1]);
       return 2;
      }

      fout = fopen(argv[2], "w");

      if (NULL == fout) {
       fprintf(stderr, errOut, argv[2]);
       return 3;
      }

      process(fin, fout);
      fclose(fin);
      fclose(fout);
      break;

    default:
      printf("Use: tap2bas [<input.tap> [<output.bas]]\n");
      return 1;
      break;
  }

  return 0;
}
