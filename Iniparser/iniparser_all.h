/*Copyright(c) 2000 - 2012 by Nicolas Devillard.
MIT License

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files(the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions :

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.*/

#ifndef _INIPARSER_ALL_H_
#define _INIPARSER_ALL_H_
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctype.h>
#include <cstdarg>
#include "../Shared/globals.h"
#ifdef __linux__
#include <unistd.h>
#else 
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#define ASCIILINESZ         (1024)
#define INI_INVALID_KEY     ((char*)-1)

typedef struct _dictionary_ {
	int             n;     /** Number of entries in dictionary */
	ssize_t         size;  /** Storage size */
	char        **  val;   /** List of string values */
	char        **  key;   /** List of string keys */
	unsigned     *  hash;  /** List of hash values for keys */
} dictionary;

/** Maximum value size for integers and doubles. */
#define MAXVALSZ    1024

/** Minimal allocated number of entries in a dictionary */
#define DICTMINSZ   128

/** Invalid key token */
#define DICT_INVALID_KEY    ((char*)-1)

static char * xstrdup(const char * s)
{
	char * t;
	size_t len;
	if (!s)
		return NULL;

	len = strlen(s) + 1;
	t = (char*)malloc(len);
	if (t) {
		memcpy(t, s, len);
	}
	return t;
}

static int dictionary_grow(dictionary * d)
{
	char        ** new_val;
	char        ** new_key;
	unsigned     * new_hash;

	new_val = (char**)calloc(d->size * 2, sizeof *d->val);
	new_key = (char**)calloc(d->size * 2, sizeof *d->key);
	new_hash = (unsigned*)calloc(d->size * 2, sizeof *d->hash);
	if (!new_val || !new_key || !new_hash) {
		/* An allocation failed, leave the dictionary unchanged */
		if (new_val)
			free(new_val);
		if (new_key)
			free(new_key);
		if (new_hash)
			free(new_hash);
		return -1;
	}
	/* Initialize the newly allocated space */
	memcpy(new_val, d->val, d->size * sizeof(char *));
	memcpy(new_key, d->key, d->size * sizeof(char *));
	memcpy(new_hash, d->hash, d->size * sizeof(unsigned));
	/* Delete previous data */
	free(d->val);
	free(d->key);
	free(d->hash);
	/* Actually update the dictionary */
	d->size *= 2;
	d->val = new_val;
	d->key = new_key;
	d->hash = new_hash;
	return 0;
}

unsigned dictionary_hash(const char * key)
{
	size_t      len;
	unsigned    hash;
	size_t      i;

	if (!key)
		return 0;

	len = strlen(key);
	for (hash = 0, i = 0; i<len; i++) {
		hash += (unsigned)key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

dictionary * dictionary_new(size_t size)
{
	dictionary  *   d;

	/* If no size was specified, allocate space for DICTMINSZ */
	if (size<DICTMINSZ) size = DICTMINSZ;

	d = (dictionary*)calloc(1, sizeof *d);

	if (d) {
		d->size = size;
		d->val = (char**)calloc(size, sizeof *d->val);
		d->key = (char**)calloc(size, sizeof *d->key);
		d->hash = (unsigned*)calloc(size, sizeof *d->hash);
	}
	return d;
}

void dictionary_del(dictionary * d)
{
	ssize_t  i;

	if (d == NULL) return;
	for (i = 0; i<d->size; i++) {
		if (d->key[i] != NULL)
			free(d->key[i]);
		if (d->val[i] != NULL)
			free(d->val[i]);
	}
	free(d->val);
	free(d->key);
	free(d->hash);
	free(d);
	return;
}

const char * dictionary_get(const dictionary * d, const char * key, const char * def)
{
	unsigned    hash;
	ssize_t      i;

	hash = dictionary_hash(key);
	for (i = 0; i<d->size; i++) {
		if (d->key[i] == NULL)
			continue;
		/* Compare hash */
		if (hash == d->hash[i]) {
			/* Compare string, to avoid hash collisions */
			if (!strcmp(key, d->key[i])) {
				return d->val[i];
			}
		}
	}
	return def;
}

int dictionary_set(dictionary * d, const char * key, const char * val)
{
	ssize_t         i;
	unsigned       hash;

	if (d == NULL || key == NULL) return -1;

	/* Compute hash for this key */
	hash = dictionary_hash(key);
	/* Find if value is already in dictionary */
	if (d->n>0) {
		for (i = 0; i<d->size; i++) {
			if (d->key[i] == NULL)
				continue;
			if (hash == d->hash[i]) { /* Same hash value */
				if (!strcmp(key, d->key[i])) {   /* Same key */
												 /* Found a value: modify and return */
					if (d->val[i] != NULL)
						free(d->val[i]);
					d->val[i] = (val ? xstrdup(val) : NULL);
					/* Value has been modified: return */
					return 0;
				}
			}
		}
	}
	/* Add a new value */
	/* See if dictionary needs to grow */
	if (d->n == d->size) {
		/* Reached maximum size: reallocate dictionary */
		if (dictionary_grow(d) != 0)
			return -1;
	}

	/* Insert key in the first empty slot. Start at d->n and wrap at
	d->size. Because d->n < d->size this will necessarily
	terminate. */
	for (i = d->n; d->key[i]; ) {
		if (++i == d->size) i = 0;
	}
	/* Copy key */
	d->key[i] = xstrdup(key);
	d->val[i] = (val ? xstrdup(val) : NULL);
	d->hash[i] = hash;
	d->n++;
	return 0;
}

void dictionary_unset(dictionary * d, const char * key)
{
	unsigned    hash;
	ssize_t      i;

	if (key == NULL || d == NULL) {
		return;
	}

	hash = dictionary_hash(key);
	for (i = 0; i<d->size; i++) {
		if (d->key[i] == NULL)
			continue;
		/* Compare hash */
		if (hash == d->hash[i]) {
			/* Compare string, to avoid hash collisions */
			if (!strcmp(key, d->key[i])) {
				/* Found key */
				break;
			}
		}
	}
	if (i >= d->size)
		/* Key not found */
		return;

	free(d->key[i]);
	d->key[i] = NULL;
	if (d->val[i] != NULL) {
		free(d->val[i]);
		d->val[i] = NULL;
	}
	d->hash[i] = 0;
	d->n--;
	return;
}

void dictionary_dump(const dictionary * d, FILE * out)
{
	ssize_t  i;

	if (d == NULL || out == NULL) return;
	if (d->n<1) {
		fprintf(out, "empty dictionary\n");
		return;
	}
	for (i = 0; i<d->size; i++) {
		if (d->key[i]) {
			fprintf(out, "%20s\t[%s]\n",
				d->key[i],
				d->val[i] ? d->val[i] : "UNDEF");
		}
	}
	return;
}

typedef enum _line_status_ {
	LINE_UNPROCESSED,
	LINE_ERROR,
	LINE_EMPTY,
	LINE_COMMENT,
	LINE_SECTION,
	LINE_VALUE
} line_status;

/*-------------------------------------------------------------------------*/
/**
@brief    Convert a string to lowercase.
@param    in   String to convert.
@param    out Output buffer.
@param    len Size of the out buffer.
@return   ptr to the out buffer or NULL if an error occured.

This function convert a string into lowercase.
At most len - 1 elements of the input string will be converted.
*/
/*--------------------------------------------------------------------------*/
static const char * strlwc(const char * in, char *out, unsigned len)
{
	unsigned i;

	if (in == NULL || out == NULL || len == 0) return NULL;
	i = 0;
	while (in[i] != '\0' && i < len - 1) {
		out[i] = (char)tolower((int)in[i]);
		i++;
	}
	out[i] = '\0';
	return out;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Remove blanks at the beginning and the end of a string.
@param    str  String to parse and alter.
@return   unsigned New size of the string.
*/
/*--------------------------------------------------------------------------*/
static unsigned int strstrip(char * s)
{
	char *last = NULL;
	char *dest = s;

	if (s == NULL) return 0;

	last = s + strlen(s);
	while (isspace((int)*s) && *s) s++;
	while (last > s) {
		if (!isspace((int)*(last - 1)))
			break;
		last--;
	}
	*last = (char)0;

	memmove(dest, s, last - s + 1);
	return (unsigned int) (last - s);
}

/*-------------------------------------------------------------------------*/
/**
@brief    Default error callback for iniparser: wraps `fprintf(stderr, ...)`.
*/
/*--------------------------------------------------------------------------*/
static int default_error_callback(const char *format, ...)
{
	int ret;
	va_list argptr;
	va_start(argptr, format);
	ret = vfprintf(stderr, format, argptr);
	va_end(argptr);
	return ret;
}

static int(*iniparser_error_callback)(const char*, ...) = default_error_callback;

/*-------------------------------------------------------------------------*/
/**
@brief    Configure a function to receive the error messages.
@param    errback  Function to call.

By default, the error will be printed on stderr. If a null pointer is passed
as errback the error callback will be switched back to default.
*/
/*--------------------------------------------------------------------------*/
void iniparser_set_error_callback(int(*errback)(const char *, ...))
{
	if (errback) {
		iniparser_error_callback = errback;
	}
	else {
		iniparser_error_callback = default_error_callback;
	}
}

/*-------------------------------------------------------------------------*/
/**
@brief    Get number of sections in a dictionary
@param    d   Dictionary to examine
@return   int Number of sections found in dictionary

This function returns the number of sections found in a dictionary.
The test to recognize sections is done on the string stored in the
dictionary: a section name is given as "section" whereas a key is
stored as "section:key", thus the test looks for entries that do not
contain a colon.

This clearly fails in the case a section name contains a colon, but
this should simply be avoided.

This function returns -1 in case of error.
*/
/*--------------------------------------------------------------------------*/
int iniparser_getnsec(const dictionary * d)
{
	int i;
	int nsec;

	if (d == NULL) return -1;
	nsec = 0;
	for (i = 0; i<d->size; i++) {
		if (d->key[i] == NULL)
			continue;
		if (strchr(d->key[i], ':') == NULL) {
			nsec++;
		}
	}
	return nsec;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Get name for section n in a dictionary.
@param    d   Dictionary to examine
@param    n   Section number (from 0 to nsec-1).
@return   Pointer to char string

This function locates the n-th section in a dictionary and returns
its name as a pointer to a string statically allocated inside the
dictionary. Do not free or modify the returned string!

This function returns NULL in case of error.
*/
/*--------------------------------------------------------------------------*/
const char * iniparser_getsecname(const dictionary * d, int n)
{
	int i;
	int foundsec;

	if (d == NULL || n<0) return NULL;
	foundsec = 0;
	for (i = 0; i<d->size; i++) {
		if (d->key[i] == NULL)
			continue;
		if (strchr(d->key[i], ':') == NULL) {
			foundsec++;
			if (foundsec>n)
				break;
		}
	}
	if (foundsec <= n) {
		return NULL;
	}
	return d->key[i];
}

/*-------------------------------------------------------------------------*/
/**
@brief    Dump a dictionary to an opened file pointer.
@param    d   Dictionary to dump.
@param    f   Opened file pointer to dump to.
@return   void

This function prints out the contents of a dictionary, one element by
line, onto the provided file pointer. It is OK to specify @c stderr
or @c stdout as output files. This function is meant for debugging
purposes mostly.
*/
/*--------------------------------------------------------------------------*/
void iniparser_dump(const dictionary * d, FILE * f)
{
	int     i;

	if (d == NULL || f == NULL) return;
	for (i = 0; i<d->size; i++) {
		if (d->key[i] == NULL)
			continue;
		if (d->val[i] != NULL) {
			fprintf(f, "[%s]=[%s]\n", d->key[i], d->val[i]);
		}
		else {
			fprintf(f, "[%s]=UNDEF\n", d->key[i]);
		}
	}
	return;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Get the string associated to a key
@param    d       Dictionary to search
@param    key     Key string to look for
@param    def     Default value to return if key not found.
@return   pointer to statically allocated character string

This function queries a dictionary for a key. A key as read from an
ini file is given as "section:key". If the key cannot be found,
the pointer passed as 'def' is returned.
The returned char pointer is pointing to a string allocated in
the dictionary, do not free or modify it.
*/
/*--------------------------------------------------------------------------*/
const char * iniparser_getstring(const dictionary * d, const char * key, const char * def)
{
	const char * lc_key;
	const char * sval;
	char tmp_str[ASCIILINESZ + 1];

	if (d == NULL || key == NULL)
		return def;

	lc_key = strlwc(key, tmp_str, sizeof(tmp_str));
	sval = dictionary_get(d, lc_key, def);
	return sval;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Finds out if a given entry exists in a dictionary
@param    ini     Dictionary to search
@param    entry   Name of the entry to look for
@return   integer 1 if entry exists, 0 otherwise

Finds out if a given entry exists in the dictionary. Since sections
are stored as keys with NULL associated values, this is the only way
of querying for the presence of sections in a dictionary.
*/
/*--------------------------------------------------------------------------*/
int iniparser_find_entry(const dictionary * ini, const char * entry)
{
	int found = 0;
	if (iniparser_getstring(ini, entry, INI_INVALID_KEY) != INI_INVALID_KEY) {
		found = 1;
	}
	return found;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Save a dictionary section to a loadable ini file
@param    d   Dictionary to dump
@param    s   Section name of dictionary to dump
@param    f   Opened file pointer to dump to
@return   void

This function dumps a given section of a given dictionary into a loadable ini
file.  It is Ok to specify @c stderr or @c stdout as output files.
*/
/*--------------------------------------------------------------------------*/
void iniparser_dumpsection_ini(const dictionary * d, const char * s, FILE * f)
{
	int     j;
	char    keym[ASCIILINESZ + 1];
	int     seclen;

	if (d == NULL || f == NULL) return;
	if (!iniparser_find_entry(d, s)) return;

	seclen = (int)strlen(s);
	fprintf(f, "\n[%s]\n", s);
	sprintf(keym, "%s:", s);
	for (j = 0; j<d->size; j++) {
		if (d->key[j] == NULL)
			continue;
		if (!strncmp(d->key[j], keym, seclen + 1)) {
			fprintf(f,
				"%-30s = %s\n",
				d->key[j] + seclen + 1,
				d->val[j] ? d->val[j] : "");
		}
	}
	fprintf(f, "\n");
	return;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Save a dictionary to a loadable ini file
@param    d   Dictionary to dump
@param    f   Opened file pointer to dump to
@return   void

This function dumps a given dictionary into a loadable ini file.
It is Ok to specify @c stderr or @c stdout as output files.
*/
/*--------------------------------------------------------------------------*/
void iniparser_dump_ini(const dictionary * d, FILE * f)
{
	int          i;
	int          nsec;
	const char * secname;

	if (d == NULL || f == NULL) return;

	nsec = iniparser_getnsec(d);
	if (nsec<1) {
		/* No section in file: dump all keys as they are */
		for (i = 0; i<d->size; i++) {
			if (d->key[i] == NULL)
				continue;
			fprintf(f, "%s = %s\n", d->key[i], d->val[i]);
		}
		return;
	}
	for (i = 0; i<nsec; i++) {
		secname = iniparser_getsecname(d, i);
		iniparser_dumpsection_ini(d, secname, f);
	}
	fprintf(f, "\n");
	return;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Get the number of keys in a section of a dictionary.
@param    d   Dictionary to examine
@param    s   Section name of dictionary to examine
@return   Number of keys in section
*/
/*--------------------------------------------------------------------------*/
int iniparser_getsecnkeys(const dictionary * d, const char * s)
{
	int     seclen, nkeys;
	char    keym[ASCIILINESZ + 1];
	int j;

	nkeys = 0;

	if (d == NULL) return nkeys;
	if (!iniparser_find_entry(d, s)) return nkeys;

	seclen = (int)strlen(s);
	strlwc(s, keym, sizeof(keym));
	keym[seclen] = ':';

	for (j = 0; j<d->size; j++) {
		if (d->key[j] == NULL)
			continue;
		if (!strncmp(d->key[j], keym, seclen + 1))
			nkeys++;
	}

	return nkeys;

}

/*-------------------------------------------------------------------------*/
/**
@brief    Get the number of keys in a section of a dictionary.
@param    d    Dictionary to examine
@param    s    Section name of dictionary to examine
@param    keys Already allocated array to store the keys in
@return   The pointer passed as `keys` argument or NULL in case of error

This function queries a dictionary and finds all keys in a given section.
The keys argument should be an array of pointers which size has been
determined by calling `iniparser_getsecnkeys` function prior to this one.

Each pointer in the returned char pointer-to-pointer is pointing to
a string allocated in the dictionary; do not free or modify them.
*/
/*--------------------------------------------------------------------------*/
const char ** iniparser_getseckeys(const dictionary * d, const char * s, const char ** keys)
{
	int i, j, seclen;
	char keym[ASCIILINESZ + 1];

	if (d == NULL || keys == NULL) return NULL;
	if (!iniparser_find_entry(d, s)) return NULL;

	seclen = (int)strlen(s);
	strlwc(s, keym, sizeof(keym));
	keym[seclen] = ':';

	i = 0;

	for (j = 0; j<d->size; j++) {
		if (d->key[j] == NULL)
			continue;
		if (!strncmp(d->key[j], keym, seclen + 1)) {
			keys[i] = d->key[j];
			i++;
		}
	}

	return keys;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Get the string associated to a key, convert to an long int
@param    d Dictionary to search
@param    key Key string to look for
@param    notfound Value to return in case of error
@return   long integer

This function queries a dictionary for a key. A key as read from an
ini file is given as "section:key". If the key cannot be found,
the notfound value is returned.

Supported values for integers include the usual C notation
so decimal, octal (starting with 0) and hexadecimal (starting with 0x)
are supported. Examples:

"42"      ->  42
"042"     ->  34 (octal -> decimal)
"0x42"    ->  66 (hexa  -> decimal)

Warning: the conversion may overflow in various ways. Conversion is
totally outsourced to strtol(), see the associated man page for overflow
handling.

Credits: Thanks to A. Becker for suggesting strtol()
*/
/*--------------------------------------------------------------------------*/
long int iniparser_getlongint(const dictionary * d, const char * key, long int notfound)
{
	const char * str;

	str = iniparser_getstring(d, key, INI_INVALID_KEY);
	if (str == INI_INVALID_KEY) return notfound;
	return strtol(str, NULL, 0);
}


/*-------------------------------------------------------------------------*/
/**
@brief    Get the string associated to a key, convert to an int
@param    d Dictionary to search
@param    key Key string to look for
@param    notfound Value to return in case of error
@return   integer

This function queries a dictionary for a key. A key as read from an
ini file is given as "section:key". If the key cannot be found,
the notfound value is returned.

Supported values for integers include the usual C notation
so decimal, octal (starting with 0) and hexadecimal (starting with 0x)
are supported. Examples:

"42"      ->  42
"042"     ->  34 (octal -> decimal)
"0x42"    ->  66 (hexa  -> decimal)

Warning: the conversion may overflow in various ways. Conversion is
totally outsourced to strtol(), see the associated man page for overflow
handling.

Credits: Thanks to A. Becker for suggesting strtol()
*/
/*--------------------------------------------------------------------------*/
int iniparser_getint(const dictionary * d, const char * key, int notfound)
{
	return (int)iniparser_getlongint(d, key, notfound);
}

/*-------------------------------------------------------------------------*/
/**
@brief    Get the string associated to a key, convert to a double
@param    d Dictionary to search
@param    key Key string to look for
@param    notfound Value to return in case of error
@return   double

This function queries a dictionary for a key. A key as read from an
ini file is given as "section:key". If the key cannot be found,
the notfound value is returned.
*/
/*--------------------------------------------------------------------------*/
double iniparser_getdouble(const dictionary * d, const char * key, double notfound)
{
	const char * str;

	str = iniparser_getstring(d, key, INI_INVALID_KEY);
	if (str == INI_INVALID_KEY) return notfound;
	return atof(str);
}

/*-------------------------------------------------------------------------*/
/**
@brief    Get the string associated to a key, convert to a boolean
@param    d Dictionary to search
@param    key Key string to look for
@param    notfound Value to return in case of error
@return   integer

This function queries a dictionary for a key. A key as read from an
ini file is given as "section:key". If the key cannot be found,
the notfound value is returned.

A true boolean is found if one of the following is matched:

- A string starting with 'y'
- A string starting with 'Y'
- A string starting with 't'
- A string starting with 'T'
- A string starting with '1'

A false boolean is found if one of the following is matched:

- A string starting with 'n'
- A string starting with 'N'
- A string starting with 'f'
- A string starting with 'F'
- A string starting with '0'

The notfound value returned if no boolean is identified, does not
necessarily have to be 0 or 1.
*/
/*--------------------------------------------------------------------------*/
int iniparser_getboolean(const dictionary * d, const char * key, int notfound)
{
	int          ret;
	const char * c;

	c = iniparser_getstring(d, key, INI_INVALID_KEY);
	if (c == INI_INVALID_KEY) return notfound;
	if (c[0] == 'y' || c[0] == 'Y' || c[0] == '1' || c[0] == 't' || c[0] == 'T') {
		ret = 1;
	}
	else if (c[0] == 'n' || c[0] == 'N' || c[0] == '0' || c[0] == 'f' || c[0] == 'F') {
		ret = 0;
	}
	else {
		ret = notfound;
	}
	return ret;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Set an entry in a dictionary.
@param    ini     Dictionary to modify.
@param    entry   Entry to modify (entry name)
@param    val     New value to associate to the entry.
@return   int 0 if Ok, -1 otherwise.

If the given entry can be found in the dictionary, it is modified to
contain the provided value. If it cannot be found, the entry is created.
It is Ok to set val to NULL.
*/
/*--------------------------------------------------------------------------*/
int iniparser_set(dictionary * ini, const char * entry, const char * val)
{
	char tmp_str[ASCIILINESZ + 1];
	return dictionary_set(ini, strlwc(entry, tmp_str, sizeof(tmp_str)), val);
}

/*-------------------------------------------------------------------------*/
/**
@brief    Delete an entry in a dictionary
@param    ini     Dictionary to modify
@param    entry   Entry to delete (entry name)
@return   void

If the given entry can be found, it is deleted from the dictionary.
*/
/*--------------------------------------------------------------------------*/
void iniparser_unset(dictionary * ini, const char * entry)
{
	char tmp_str[ASCIILINESZ + 1];
	dictionary_unset(ini, strlwc(entry, tmp_str, sizeof(tmp_str)));
}

/*-------------------------------------------------------------------------*/
/**
@brief    Load a single line from an INI file
@param    input_line  Input line, may be concatenated multi-line input
@param    section     Output space to store section
@param    key         Output space to store key
@param    value       Output space to store value
@return   line_status value
*/
/*--------------------------------------------------------------------------*/
static line_status iniparser_line(
	const char * input_line,
	char * section,
	char * key,
	char * value)
{
	line_status sta;
	char * line = NULL;
	DWORD      len;

	line = xstrdup(input_line);
	len = strstrip(line);

	sta = LINE_UNPROCESSED;
	if (len<1) {
		/* Empty line */
		sta = LINE_EMPTY;
	}
	else if (line[0] == '#' || line[0] == ';') {
		/* Comment line */
		sta = LINE_COMMENT;
	}
	else if (line[0] == '[' && line[len - 1] == ']') {
		/* Section name */
		sscanf(line, "[%[^]]", section);
		strstrip(section);
		strlwc(section, section, len);
		sta = LINE_SECTION;
	}
	else if (sscanf(line, "%[^=] = \"%[^\"]\"", key, value) == 2
		|| sscanf(line, "%[^=] = '%[^\']'", key, value) == 2) {
		/* Usual key=value with quotes, with or without comments */
		strstrip(key);
		strlwc(key, key, len);
		/* Don't strip spaces from values surrounded with quotes */
		sta = LINE_VALUE;
	}
	else if (sscanf(line, "%[^=] = %[^;#]", key, value) == 2) {
		/* Usual key=value without quotes, with or without comments */
		strstrip(key);
		strlwc(key, key, len);
		strstrip(value);
		/*
		* sscanf cannot handle '' or "" as empty values
		* this is done here
		*/
		if (!strcmp(value, "\"\"") || (!strcmp(value, "''"))) {
			value[0] = 0;
		}
		sta = LINE_VALUE;
	}
	else if (sscanf(line, "%[^=] = %[;#]", key, value) == 2
		|| sscanf(line, "%[^=] %[=]", key, value) == 2) {
		/*
		* Special cases:
		* key=
		* key=;
		* key=#
		*/
		strstrip(key);
		strlwc(key, key, len);
		value[0] = 0;
		sta = LINE_VALUE;
	}
	else {
		/* Generate syntax error */
		sta = LINE_ERROR;
	}

	free(line);
	return sta;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Parse an ini file and return an allocated dictionary object
@param    ininame Name of the ini file to read.
@return   Pointer to newly allocated dictionary

This is the parser for ini files. This function is called, providing
the name of the file to be read. It returns a dictionary object that
should not be accessed directly, but through accessor functions
instead.

The returned dictionary must be freed using iniparser_freedict().
*/
/*--------------------------------------------------------------------------*/
dictionary * iniparser_load(const char * ininame)
{
	FILE * in;

	char line[ASCIILINESZ + 1];
	char section[ASCIILINESZ + 1];
	char key[ASCIILINESZ + 1];
	char tmp[(ASCIILINESZ * 2) + 1];
	char val[ASCIILINESZ + 1];

	int  last = 0;
	int  len;
	int  lineno = 0;
	int  errs = 0;
	int  mem_err = 0;

	dictionary * dict;

	if ((in = fopen(ininame, "r")) == NULL) {
		iniparser_error_callback("iniparser: cannot open %s\n", ininame);
		return NULL;
	}

	dict = dictionary_new(0);
	if (!dict) {
		fclose(in);
		return NULL;
	}

	memset(line, 0, ASCIILINESZ);
	memset(section, 0, ASCIILINESZ);
	memset(key, 0, ASCIILINESZ);
	memset(val, 0, ASCIILINESZ);
	last = 0;

	while (fgets(line + last, ASCIILINESZ - last, in) != NULL) {
		lineno++;
		len = (int)strlen(line) - 1;
		if (len <= 0)
			continue;
		/* Safety check against buffer overflows */
		if (line[len] != '\n' && !feof(in)) {
			iniparser_error_callback(
				"iniparser: input line too long in %s (%d)\n",
				ininame,
				lineno);
			dictionary_del(dict);
			fclose(in);
			return NULL;
		}
		/* Get rid of \n and spaces at end of line */
		while ((len >= 0) &&
			((line[len] == '\n') || (isspace(line[len])))) {
			line[len] = 0;
			len--;
		}
		if (len < 0) { /* Line was entirely \n and/or spaces */
			len = 0;
		}
		/* Detect multi-line */
		if (line[len] == '\\') {
			/* Multi-line value */
			last = len;
			continue;
		}
		else {
			last = 0;
		}
		switch (iniparser_line(line, section, key, val)) {
		case LINE_EMPTY:
		case LINE_COMMENT:
			break;

		case LINE_SECTION:
			mem_err = dictionary_set(dict, section, NULL);
			break;

		case LINE_VALUE:
			sprintf(tmp, "%s:%s", section, key);
			mem_err = dictionary_set(dict, tmp, val);
			break;

		case LINE_ERROR:
			iniparser_error_callback(
				"iniparser: syntax error in %s (%d):\n-> %s\n",
				ininame,
				lineno,
				line);
			errs++;
			break;

		default:
			break;
		}
		memset(line, 0, ASCIILINESZ);
		last = 0;
		if (mem_err<0) {
			iniparser_error_callback("iniparser: memory allocation failure\n");
			break;
		}
	}
	if (errs) {
		dictionary_del(dict);
		dict = NULL;
	}
	fclose(in);
	return dict;
}

/*-------------------------------------------------------------------------*/
/**
@brief    Free all memory associated to an ini dictionary
@param    d Dictionary to free
@return   void

Free all memory associated to an ini dictionary.
It is mandatory to call this function before the dictionary object
gets out of the current context.
*/
/*--------------------------------------------------------------------------*/
void iniparser_freedict(dictionary * d)
{
	dictionary_del(d);
}
#endif