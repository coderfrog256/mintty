#ifndef TERMLINE_H
#define TERMLINE_H

typedef struct {
 /*
  * Any code in terminal.c which definitely needs to be changed
  * when extra fields are added here is labelled with a comment
  * saying FULL-TERMCHAR.
  */
  uint chr;
  uint attr;

 /*
  * The cc_next field is used to link multiple termchars
  * together into a list, so as to fit more than one character
  * into a character cell (Unicode combining characters).
  * 
  * cc_next is a relative offset into the current array of
  * termchars. I.e. to advance to the next character in a list,
  * one does `tc += tc->next'.
  * 
  * Zero means end of list.
  */
  int cc_next;
} termchar;

typedef struct {
  ushort lattr;
  int cols;     /* number of real columns on the line */
  int size;     /* number of allocated termchars
                 * (cc-lists may make this > cols) */
  int temporary;        /* true if decompressed from scrollback */
  int cc_free;  /* offset to first cc in free list */
  termchar *chars;
} termline;

typedef struct {
  int width;
  termchar *chars;
  int *forward, *backward;      /* the permutations of line positions */
} bidi_cache_entry;

termline *newline(int cols, int bce);
void freeline(termline *);
void resizeline(termline *, int);

int sblines(void);
termline *lineptr(int y);
void unlineptr(termline *);

int termchars_equal(termchar *a, termchar *b);
int termchars_equal_override(termchar *a, termchar *b, uint bchr, uint battr);

void copy_termchar(termline *destline, int x, termchar *src);
void move_termchar(termline *line, termchar *dest, termchar *src);

void add_cc(termline *, int col, uint chr);
void clear_cc(termline *, int col);

uchar *compressline(termline *);
termline *decompressline(uchar *, int *bytes_used);

termchar *term_bidi_line(termline *, int scr_y);

#define scrlineptr(x) (lineptr)(x)

/*
 * Fetch the character at a particular position in a line array,
 * for purposes of `wordtype'. The reason this isn't just a simple
 * array reference is that if the character we find is UCSWIDE,
 * then we must look one space further to the left.
 */
#define UCSGET(a, x) \
    ( (x)>0 && (a)[(x)].chr == UCSWIDE ? (a)[(x)-1].chr : (a)[(x)].chr )

/*
 * Detect the various aliases of U+0020 SPACE.
 */
#define IS_SPACE_CHR(chr) \
	((chr) == 0x20 || (DIRECT_CHAR(chr) && ((chr) & 0xFF) == 0x20))

/*
 * Spot magic CSETs.
 */
#define CSET_OF(chr) (DIRECT_CHAR(chr)||DIRECT_FONT(chr) ? (chr)&CSET_MASK : 0)

#endif
