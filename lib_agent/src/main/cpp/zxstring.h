
#ifndef __ZX_STRING__
#define __ZX_STRING__


#define STRING_ERROR	-1
#define STRING_OK		0
#define STRING_NPOS		0xFFFFFFFF

typedef struct _zx_string zx_string;

zx_string*	string_create();
zx_string*	string_create_chars(const char *);
zx_string*  string_create_size(unsigned int size = 0);
zx_string*  string_create_bin(const unsigned char *, unsigned int);
zx_string*  string_create_zxs(zx_string *);
void		string_release(zx_string *);
int			string_add_chars(zx_string *, const char *);
int			string_add_uint(zx_string *, unsigned int);
int			string_add_int(zx_string *, int);
int			string_add_bin(zx_string *, const unsigned char *, unsigned int);
int			string_add_zxs(zx_string *, zx_string *);
int			string_clear(zx_string *);
char*		string_detach(zx_string **);
char*		string_get_data(zx_string *);
unsigned int string_get_len(zx_string *);

int			string_tolower(zx_string *);
int			string_toupper(zx_string *);
unsigned int string_find_chars(zx_string *, const char *, unsigned int pos = 0);
unsigned int string_find_char(zx_string *, const char, unsigned int pos = 0);
zx_string*	string_substr_begin(zx_string *, unsigned int);
zx_string*	string_substr_begin_cnt(zx_string *, unsigned int, unsigned int);
int			string_swap(zx_string *, zx_string *);
int			string_to_uint(zx_string *, unsigned int *n);


#endif	//__ZX_STRING__
