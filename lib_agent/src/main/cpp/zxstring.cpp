#include "zxstring.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "system.h"

#define SIZE_EXPAND_MAX 1024
#define SIZE_EXPAND_MID 128
#define SIZE_EXPAND_MIN 32

typedef struct _zx_string
{
	unsigned char *str;
	unsigned int str_size;
	unsigned int str_len;
}zx_string;

unsigned int string_get_dest_len( zx_string *zx_s, unsigned int add_len )
{
	unsigned int dest_len = zx_s->str_len + add_len;
	if ( dest_len >= SIZE_EXPAND_MAX ) {
		return (dest_len / SIZE_EXPAND_MAX + 1) * SIZE_EXPAND_MAX;
	} else if ( dest_len >= SIZE_EXPAND_MID ) {
		return (dest_len / SIZE_EXPAND_MID + 1) * SIZE_EXPAND_MID;
	} else {
		return (dest_len / SIZE_EXPAND_MIN + 1) * SIZE_EXPAND_MIN;
	}
}

void string_expand( zx_string *zx_s, unsigned int dest_len )
{
	if ( dest_len <= zx_s->str_size ) return;

	unsigned char *tmp = ( unsigned char *)malloc( dest_len );

	if ( tmp != NULL ) {
		if ( NULL != zx_s->str ) memcpy( tmp, zx_s->str, zx_s->str_len );
		memset( tmp + zx_s->str_len, 0, dest_len - zx_s->str_len );
		if ( NULL != zx_s->str ) free( zx_s->str );

		zx_s->str = tmp;
		zx_s->str_size = dest_len;
	}
}

zx_string *string_create()
{
	return string_create_size(SIZE_EXPAND_MIN);
}

zx_string *string_create_chars(const char *szStr)
{
	zx_string *zx_s = string_create_size(strlen(szStr) + 1);
	if (NULL == zx_s) {
		return NULL;
	}

	if (STRING_OK != string_add_chars(zx_s, szStr)) {
		string_release(zx_s);
		return NULL;
	}
	
	return zx_s;
}

zx_string*  string_create_bin(const unsigned char *bin, unsigned int n)
{
	zx_string *zx_s = string_create_size(n + 1);
	if (NULL == zx_s) {
		return NULL;
	}

	if (STRING_OK != string_add_bin(zx_s, bin, n)) {
		string_release(zx_s);
		return NULL;
	}

	return zx_s;
}

zx_string*  string_create_zxs(zx_string *_zx_s)
{
	if (NULL == _zx_s) return NULL;
	zx_string *zx_s_in = (zx_string *)_zx_s;

	zx_string *zx_s_ret = string_create_size(zx_s_in->str_len + 1);
	if (NULL == zx_s_ret) {
		return NULL;
	}

	if (STRING_OK != string_add_bin(zx_s_ret, zx_s_in->str, zx_s_in->str_len)) {
		string_release(zx_s_ret);
		return NULL;
	}

	return zx_s_ret;
}

zx_string*  string_create_size(unsigned int size /*= 0*/)
{
	zx_string *zx_s = (zx_string*)malloc(sizeof(zx_string));
	if (NULL == zx_s) return NULL;
	memset(zx_s, 0, sizeof(zx_string));

	string_expand(zx_s, size);
	if (zx_s->str_size < size) {
		//malloc size 长度失败
		free(zx_s);
		zx_s = NULL;
	}
	return zx_s;
}

void  string_release( zx_string *_zx_s )
{
	if ( NULL == _zx_s ) return;
	zx_string *zx_s = (zx_string *)_zx_s;

	if ( 0 == zx_s->str_size ) return ;
	if ( NULL != zx_s->str ) free( zx_s->str );

	free( zx_s );
}

int string_add_chars( zx_string *_zx_s, const char *str )
{
	return string_add_bin( _zx_s, (const unsigned char *)str, strlen(str) );
}

int string_add_uint( zx_string *_zx_s, unsigned int n )
{
	char str[16] = {0};
	sprintf( str, "%u", n );
	return string_add_bin( _zx_s, (unsigned char *)str, strlen(str) );
}

int string_add_int(zx_string *_zx_s, int n)
{
	char str[16] = { 0 };
	sprintf(str, "%d", n);
	return string_add_bin(_zx_s, (unsigned char *)str, strlen(str));
}

int string_add_bin( zx_string *_zx_s, const unsigned char *bin, unsigned int n )
{
	//校验
	if ( NULL == _zx_s ) return STRING_ERROR;
	zx_string *zx_s = (zx_string *)_zx_s;

	//扩充结构体
	if ( zx_s->str_size <= zx_s->str_len + n ) {
		string_expand( zx_s, string_get_dest_len( zx_s, n ) );
	}

	//增加数据
	if ( zx_s->str_size <= zx_s->str_len + n ) return STRING_ERROR;
	memcpy( zx_s->str + zx_s->str_len, bin, n );
	zx_s->str_len += n;

	return STRING_OK;
}

int string_add_zxs(zx_string *zx_s_dest, zx_string *zx_s_src)
{
	//校验
	if (NULL == zx_s_dest || NULL == zx_s_src) return STRING_ERROR;

	return string_add_bin(zx_s_dest, zx_s_src->str, zx_s_src->str_len);
}

int string_clear(zx_string *_zx_s)
{
	//校验
	if (NULL == _zx_s) return STRING_ERROR;
	zx_string *zx_s = (zx_string *)_zx_s;

	if (NULL != zx_s->str) {
		free(zx_s->str);
		memset(zx_s, 0, sizeof(zx_string));
	}
	string_expand(zx_s, SIZE_EXPAND_MIN);
	return STRING_OK;
}

char* string_detach(zx_string **_p_zx_s)
{
	char *szRet = NULL;

	if (_p_zx_s == NULL || *_p_zx_s == NULL) return NULL;
	zx_string *zx_s = (zx_string *)(*_p_zx_s);
	
	szRet = (char *)zx_s->str;
	free(zx_s);
	*_p_zx_s = NULL;
	
	return szRet;
}

char *string_get_data( zx_string *_zx_s )
{
	//校验
	if ( NULL == _zx_s ) return NULL;
	zx_string *zx_s = (zx_string *)_zx_s;

	return (char *)zx_s->str;
}

unsigned int string_get_len( zx_string *_zx_s )
{
	//校验
	if ( NULL == _zx_s ) return STRING_ERROR;
	zx_string *zx_s = (zx_string *)_zx_s;

	return zx_s->str_len;
}



//** 扩展函数  **//

int string_tolower( zx_string *_zx_s )
{
	int i = 0, len = 0;
	char *tmp = NULL;
	//校验
	if ( NULL == _zx_s ) return STRING_ERROR;
	zx_string *zx_s = (zx_string *)_zx_s;

	tmp = string_get_data( zx_s );
	len = string_get_len( zx_s );
	for ( ; i < len; i++ ) {
		tmp[i] = tolower( tmp[i] );
	}
	return STRING_OK;
}

int string_toupper( zx_string *_zx_s )
{
	int i = 0, len = 0;
	char *tmp = NULL;
	//校验
	if ( NULL == _zx_s ) return STRING_ERROR;
	zx_string *zx_s = (zx_string *)_zx_s;

	tmp = string_get_data( zx_s );
	len = string_get_len( zx_s );
	for (; i < len; i++) {
		tmp[i] = toupper( tmp[i] );
	}
	return STRING_OK;

}

char* string_strstr( const char *s1, unsigned int n1, const char *s2, unsigned int n2 )
{
	int n = 0, i = 0;

	if ( NULL == s1 || NULL == s2 ) return NULL;
	if ( n2 > n1 ) return NULL;
	while( n<n1 ) {
		if ( n1-n < n2 ) break; //原字符长度不足
		for ( i = 0; *(s1 + n + i ) == *(s2 + i); i++)
		{
			if ( (i+1) == n2 )
				return (char *)s1 + n;
		}
		n++;
	}
	return NULL;
}

unsigned int string_find_chars( zx_string *_zx_s, const char *str, unsigned int pos /* = 0*/)
{
	//校验
	if ( NULL == _zx_s ) return STRING_NPOS;
	zx_string *zx_s = (zx_string *)_zx_s;

	if (zx_s->str_len <= pos) return STRING_NPOS;
	char *p = string_strstr( (const char *)(zx_s->str + pos), zx_s->str_len - pos, str, strlen(str) );
	if ( NULL == p ) return STRING_NPOS;

	return p - string_get_data( zx_s );
}

unsigned int string_find_char( zx_string *_zx_s, const char c, unsigned int pos /* = 0*/)
{
	//校验
	if ( NULL == _zx_s ) return STRING_NPOS;
	zx_string *zx_s = (zx_string *)_zx_s;

	if (zx_s->str_len <= pos) return STRING_NPOS;
	char *p = string_strstr((const char *)(zx_s->str + pos), zx_s->str_len - pos, &c, 1 );
	if ( NULL == p ) return STRING_NPOS;

	return p - string_get_data( zx_s );
}

zx_string *string_substr_begin(zx_string *_zx_s, unsigned int begin) 
{
	//校验
	if ( NULL == _zx_s ) return NULL;

	unsigned int len = string_get_len(_zx_s);
	return string_substr_begin_cnt(_zx_s, begin, (len >= begin) ? (len - begin) : 0);
}


zx_string *string_substr_begin_cnt( zx_string *_zx_s, unsigned int _begin, unsigned int _cnt )
{
	unsigned int cnt = _cnt;
	zx_string *zx_s_ret = NULL;
	//校验
	if ( NULL == _zx_s ) return NULL;
	zx_string *zx_s = (zx_string *)_zx_s;

	if (_begin > zx_s->str_len) {
		return NULL;
	}
	if ( zx_s->str_len < _begin + _cnt  ) cnt = zx_s->str_len - _begin;

	zx_s_ret = string_create();
	if (NULL == zx_s_ret) return NULL;

	int ret = string_add_bin( zx_s_ret, zx_s->str + _begin, cnt );
	if (ret != STRING_OK) {
		string_release(zx_s_ret);
		zx_s_ret = NULL;
	}
	return zx_s_ret;
}

int string_swap(zx_string *_zxs_left, zx_string *_zxs_right)
{
	zx_string zxsTmp;
	if (NULL == _zxs_left || NULL == _zxs_right) return STRING_ERROR;

	zxsTmp.str		= _zxs_left->str;
	zxsTmp.str_len	= _zxs_left->str_len;
	zxsTmp.str_size = _zxs_left->str_size;

	_zxs_left->str		= _zxs_right->str;
	_zxs_left->str_len	= _zxs_right->str_len;
	_zxs_left->str_size	= _zxs_right->str_size;

	_zxs_right->str		= zxsTmp.str;
	_zxs_right->str_len	= zxsTmp.str_len;
	_zxs_right->str_size = zxsTmp.str_size;
	return STRING_OK;
}

int string_to_uint(zx_string *_zx_s, unsigned int * n)
{
	unsigned int i = 0, len = 0;
	char *szbuf = NULL;
	//校验
	if (NULL == _zx_s) return STRING_ERROR;	
	if (_zx_s->str_len == 0) return STRING_ERROR;
	zx_string *zx_s = (zx_string *)_zx_s;

	for (; i < zx_s->str_len; i++) {
		if (0 == isdigit(zx_s->str[i])) {
			return STRING_ERROR;
			break;
		}
	}
	
	sscanf((char *)zx_s->str, "%u", n);

	return STRING_OK;
}
