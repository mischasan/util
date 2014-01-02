// msutil: convenience functions.
// MEM: manipulating byte-blocks and slices thereaof.
//  "MEMBUF" is a <len,ptr> descriptor of a malloc'd block of memory.
//          The owner of the MEMBUF variable also "owns" the memory.
//  "MEMREF" is a <len,ptr> descriptor of memory allocated elsewhere.
//          A MEMREF can describe a substring of a MEMBUF value.
//	membuf(size)                    Create membuf; membuf(0) => NILBUF
//	buffree(buf)
//	memref(char const*mem, int len) Create memref from (ptr,len)
//	nilbuf(buf)                     Test if buf is a NILBUF
//	nilref(ref)                     Test if ref is a NILREF
//	strbuf(str)                     Create MEMBUF from a C string; NULL => NILBUF
//	strref(str)                     Create MEMREF from a C string; NULL => NILREF
//	bufref(buf)                     Create MEMREF from a MEMBUF; NILBUF => NILREF
//      subref(ref,pos,len)             Create MEMREF based on another MEMREF.
//                                      (pos < 0) are offsets from end of ref.
//      bufcat(&buf,ref)                Append ref to buf, growing buf as needed.
//	refcmp(refa, refb)              "memcmp" for memrefs, including NILREF.
//	refcpy(mem, ref)                "memcpy" from a ref to a block of memory.
//	refdup(ref)                     Create (strdup) a C string from a MEMREF; NILREF => NULL.
//	reffind(tgt, pat)               "strstr" for memrefs
//	refpcmp(&refa, &refb)           refcmp for qsort
//      fileref(filename)               Create MEMREF to mmap'd readonly file
//      defileref(ref)                  Release MEMREF'd mmap'd file.
//
// VEC: dynamic vectors, akin to STL vectors.
//      vec_new(int width, DTOR *dtor, void *context)
//            vec_free/vec_resize may call dtor(&elem, context) per element.
//      vec_free(VEC*)                Destroy vec
//      vec_width(VEC*)               element size
//      vec_count(VEC*)               element count
//      vel(VEC*,int i)               pointer to (i)th element, or NULL if i >= count.
//      vec_data(VEC*)                pointer to data part of VEC
//      vec_allow(VEC*,limit)         Ensure Vec can hold at least (limit) elements.
//      vec_resize(VEC*,limit)        Change count to (limit).
//      vec_push(VEC*, void*)         Append an element, resizing if needed.
//      vec_pop(VEC*)                 Return last element, decrementing count.
//
// STR: dynamic strings, akin to STL strings. Based on VEC.
//      STR{new,free,len,ptr}       - STR functions
//      STR{cat,chr,cmp,cpy,dup,str}- (STR,C-string) functions
//      STR{CAT,CMP,CPY,DUP,STR}    - (STR,STR) functions.
//
// MAP: dynamic hash table with opaque records. Type "/@map" for details.
//          Caller owns all keys and values.
//
// KVS: (key,val) string-pair map, based on MAP. 
//          Object owns all keys and values.
//      kvs_create()
//      kvs_destroy(ms)
//      kvs_load(filename,sep)  Create KVS from key<sep>val pairs in a text
//      kvs_count(ms)           Key count
//      kvs_set(ms, key, val)   Add/replace key:val
//      kvs_get(ms, key)
//      kvs_del(ms, key)
//      kvs_start(ms)           Initialize a "next" loop.
//      kvs_next(ms, char**kp, char**vp)
//      kvs_resize(ms, int n)
// CACHE: multiprocess/multithread fixed-size cache.
//      KEY is a (char const*), usually <= 16 bytes.
//      VAL is a uint32_t (I know that is tight).
//      cache_open(path, maxbytes)
//      cache_close(cache)
//      cache_get(cache, key)
//      cache_set(cache, key, val)
//
// THREAD: pthread wrapper. 
//      thread_start(func, arg)         Create and start a thread.
//      thread_cancel(thread)
//      thread_wait(thread)             Wait for a thread to exit.
//
// FLAG: pthread_cond wrapper. Behaves like signals.
//      flag_create()
//      flag_destroy(flag)
//      flag_watch(flag, waitsecs)      Wait for a signal.
//      flag_any(flag)                  Signal any one watching thread.
//      flag_all(flag)                  Signal all watching threads.
//
// SOCK: simple socket interface using only std "C" types
//       int: skt, nowait, port, size, val     char*: host, ip, buf, name
//          Int values are in HOST byte order.
//	    IP values are string-format IPv4/IPv6 addresses;
//              "" or NULL => INADDR_ANY, "::" => IN6ADDR_ANY.
//          All but (sock_getopt,sock_recv*) return fd>= 0 or 0=okay or -1=error.
//          ALL sockets are created with CLOEXEC.
//
//      sock_addr(skt, IPSTR, &port, name, namesize)
//      sock_bind(ipstr,port)       Create Inet socket (server)
//      sock_create(path)           Create Unix socket (server)
//      sock_accept(skt)
//      sock_connect(host|ip, port, nowait)
//      sock_open(path)             Connect to unix socket.
//      sock_getopt(skt, SOCK_OPT)
//      sock_setopt(skt, SOCK_OPT, val)
//      sock_recv(skt, buf, size)
//      sock_send(skt, buf, size)
//  Unix sockets only:
//      sock_recvfd(skt, &fd, buf, size)
//      sock_sendfd(skt,  fd, buf, size)
//          NOTE: if recvfd has a smaller buffer than sendfd, or if send is
//          used instead of sendfd, data and message boundaries are lost.
//          Never mix send/recv and sendfd/recvfd on the same connection.
//  For Inet sockets; any ptr may be 0:
//      sock_peer(skt, &ip, &port, &host, size)
//  For Inet sockets on Linux: retrieve original destination:
//      sock_dest(skt, &ip, &port)
//  UDP sockets:
//      udp_open(ipstr, port)
//      udp_send(skt, buf, len, ip, port)
//      udp_recv(skt, buf, len, ip, &port)
//
// ROLLHASH: rolling hash. See rollhash_t.c for usage.
//
// MISCELLANEOUS
//	Some of these are useful; some are just experimental leftovers.
//
//  acstr(buf,len)          - format buffer as a CSTRING (malloc'd result).
//                              including "...", \n, \xHH formats
//  addr_part(memref)       - a memref to the useful email address part of the input
//  belch(fname,ref)        - Save memref to file. e.g. belch("/tmp/foo", strref(s))
//  bit_count               - count bits
//  bitmat_trans            - transpose a bit matrix
//  bsrl(long)              - execute the bsrl op on a NON-ZERO VALUE, returning 0..31
//  bitwid(x)               - min # of bits required to represent (x); ceil(log2(N-1))
//  bndmem(t,tlen,p,plen)   - BNDM search for patt in text.
//  CacheLineWidth          - in bytes, for use with _mm_prefetch.
//  cessu32                 - high-quality FAST 32-byte hash (Jenkins using SSE2).
//  chomp(buf)              - Remove trailing >>WHITESPACE<< from a MEMBUF.
//                              chomp(NILBUF) = NILBUF.
//  cmpxm(src,dst,count)    - memcmp implemented with SSE2.
//  cpuinfo                 - Invoke machine CPUID op, returning ECX.
//  day2ymd(epoch)          - the {int y,m,d;} for days since 1900/03/01 for (y,m,d) < (2100,03,01).
//  die(fmt, ...)           - Print message to stderr and exit(1).
//                              If fmt begins with ":", die prepends program name.
//                              If fmt ends in ":", die appends strerror(errno).
//
//  eainame[neainames]      - (string) names for (negative) getaddrinfo return codes.
//  errname[nerrnames]      - (string) names for errno values. More succinct than strerror(). 
//  findbit_0(vec,nbytes)   - find position of first '0' bit in a bit vector.
//  findbit_1(vec,nbytes)   - find position of first '1' bit in bit vector.
//  fnv{04,08,16}           - reasonable, fast Fowler-Noll-Voh scalar hashes.
//  fls(x)                  - find highest bit set (similar to "ffs"), for Linux.
//                              fls(1)=1, fls(10)=4, fls(-1)=32, fls(0)=0
//  fopenf(mode,fmt...)     - Open a temp file using an fopen mode and a printf-fmt filename.
//  getprogname()           - Returns pointer to the program name (basename).
//  lohuff                  - Generate a LSBit-first Huffman-code table,
//                              from a table of NONZERO symbol frequencies.
//                              Codes are stored as (CODE << 8)+LENG.
//	                        Ensures that code[len-1] is a string of zero bits.
//  maccess(buf,len,mode)   - Test whether buf[0..len-1] is readable/writable memory.
//                              mode is 0 for readable, 1 for readwrite.
//  maccess_init()          - Optional initialization routine, called explicitly 
//                              when maccess is used in a threaded environment.
//  memfind(A,m,B,n)        - "strstr" for (ptr,len) pairs
//  mmhash3_x{86,64}_{04,08,16} - MurmurHash3 for 32 and 64-bit cpus.
//  ordhuff                 - Generate ORDERED huffman code table, in LINEAR time.
//  phkeys(file,&cnt)       - Retrieve key strings of a perl hash whose values are all (1),
//                              saved with Storable.
//  popenf(type,fmt,...)    - popen((fmt,...), type) using printf-fmt and args for a command.
//  refsplit(s,sep,&cnt)    - split text. Replaces every (sep) with \0 in (text).
//                              Returns malloc'd vector of [cnt] MEMREF's.
//  revbit32(u32)           - reverse bit order in 32bit word
//  scan{brk,chr,cmp,len,str,str2,strx} - faster versions of str{*}
//                              On some platforms, scanstr is 6* faster than strstr,
//                              scanchr is 15* faster than strchr.
//                              scanstrx is scanstr using scanstr2 (per blog post)
//  signame[nsignames]      - (string) names for signals (e.g. "SIGHUP").
//  slurp(fname)            - create membuf from a file. fname "-" or NULL reads stdin.
//  strim(s)                - "Trim" leading/trailing whitespace. (s) is modified.
//  systemf(fmt,...)        - system() using printf-style fmt and args.
//  tick()                  - High-precision timer returns secs as a (double).
//  tolog(FILE**)           - Connect a writable (FILE*) to syslog.
//  under(x)                - mask of bits below the lowest bit (set) in x; e.g. under 0x0320 = 0x001F
//  uri_parts(s,uriref)     - Break a request string into 7 MEMREF fields (URIREF).
//                              [scheme:][//[auth@]host[:port]][path][?query][#fragment]
//                              where "path" may begin with a slash and "auth"
//                              is "user[:password]".
// ymd2day(y,m,d)           - the days since 1900/03/01 for (y,m,d) < (2100,03,01).
//--------------|---------------------------------------------

#ifndef MSUTIL_H
#define MSUTIL_H
#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

// Convenient format strings constants e.g. printf("%"FSIZE"X", sizeof thing);
//  F64:int64_t  FPTR:intptr_t  FSIZE:size_t 
//  The strings do NOT include the "%", so that the user can specify
//  width/justification/signedness/radix etc.
//XXX see how inttypes.h tries to do this.

#ifdef __x86_64__
#   define FPTR     "l"
#elif __SIZEOF_POINTER__ == __SIZEOF_INT__
#   define FPTR
#elif __SIZEOF_POINTER__ == __SIZEOF_LONG__ 
#   define FPTR     "l"
#else
#   error need to define FPTR 
#endif

// uintptr_t is not unsigned int on stinky
// This does not work on gcc 4.1 (64)
#if __LONG_LONG_MAX__ == __LONG_MAX__
#   define FSIZE    "l"
#elif __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#   define FSIZE
#elif __SIZEOF_SIZE_T__ == __SIZEOF_LONG__
#   define FSIZE    "l"
#else
#   error need to define FSIZE 
#endif

#if __LONG_MAX__ == 9223372036854775807L
#   define F64      "l"
#elif __LONG_MAX__ == 2147483647L
#   define F64      "ll"
#else
#   error need to defined F64
#endif

#define FOFF FPTR

//XXX todo: add FTIME for time_t.

#if defined(__FreeBSD__)
#   include <sys/endian.h>  // byteswap
#   define  bswap_16(x) bswap16(x)
#   define  bswap_32(x) bswap32(x)
#   define regargs
#elif defined(__linux__)
#   include <byteswap.h>
#   include <endian.h>      // __BYTE_ORDER
#   define regargs __attribute__((fastcall))
#else
#   error "need byteswap intrinsics"
#endif

// This is the gcc way 
#define UNALIGNED __attribute__((align(1))

// Defeat gcc 4.4 cdefs.h defining __wur = __attribute((unused-result))
//	on system calls where you just don't care (vasprintf...)
// This requires <stdint.h> to be included first.
#include <stdint.h>
#undef	__wur
#define __wur

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>         // _exit etc

typedef int (*qsort_cmp)(const void *, const void *);
typedef int (*qsort_r_cmp)(const void*, const void*, void*);

// MAX_MINMAX(int) creates int_min(int,int) and imt_max(int,int):
#define MAKE_MINMAX(TYPE) \
static inline TYPE TYPE##_min(TYPE a, TYPE b) {return a < b ? a : b; }\
static inline TYPE TYPE##_max(TYPE a, TYPE b) {return a > b ? a : b; }

// Generate a function that maps a (ptr to a link field in a struct),
//   to a (ptr to the container struct). S-O-O-O C++ templating.
//  For a given typedef struct foo FOO, with field LinkType Link,
//  MAKE_LINK_FUNC(FOO,Link) creates a function
//      FOO *Link_FOO(LinkType *lp) {...}

#ifdef __GNUC__ /* required for "__typeof" */
#define MAKE_LINK_FUNC(TYPE,FIELD) \
static inline TYPE*FIELD##_##TYPE(__typeof(((TYPE*)0)->FIELD) *lp) \
{ return (TYPE*)((uint8_t*)lp - (uint8_t*)&((TYPE*)0)->FIELD); }
#endif
//--------------|-------|-------|-------------------------------
// MEMBUF's created from C strings allocate the trailing \0 
//  but do not count it in .len. Maybe that's wrong.

typedef struct { char *ptr; size_t len; }	MEMBUF;
typedef struct { char const *ptr; size_t len; } MEMREF;

#define	NILBUF		(MEMBUF){NULL,0}
#define	NILREF		(MEMREF){NULL,0}

static inline MEMBUF membuf(int size)
{ return size ? (MEMBUF){calloc(size+1, 1), size} : NILBUF; }

static inline void buffree(MEMBUF buf)
{ free(buf.ptr); }

static inline int nilbuf(MEMBUF buf)
{ return !buf.ptr; }

static inline int nilref(MEMREF const ref)
{ return !ref.len && !ref.ptr; }

static inline MEMREF memref(char const *mem, int len)
{ return mem && len ? (MEMREF){mem,len} : NILREF; }

static inline MEMREF bufref(MEMBUF const buf)
{ return (MEMREF) {buf.ptr, buf.len}; }

void    bufcat(MEMBUF*, MEMREF);

static inline MEMBUF strbuf(char const *str)
{ return str ? (MEMBUF){(char *)strdup(str), strlen(str)} : NILBUF; }

static inline MEMREF strref(char const *str)
{ return str ? (MEMREF){str, strlen(str)} : NILREF; }

int     refcmp(MEMREF const a, MEMREF const b);
int     refpcmp(MEMREF const *ap, MEMREF const *bp);

static inline void *refcpy(char *str, MEMREF r)
{ str[r.len] = 0; return memcpy(str, r.ptr, r.len); }

char*   refdup(MEMREF r);
MEMREF  subref(MEMREF r, int pos, unsigned len);

char const *memfind(char const *tgt, int tgtlen, char const *pat, int patlen);
static inline MEMREF reffind(MEMREF tgt, MEMREF pat)
{ return nilref(tgt) || nilref(pat) ? NILREF : (MEMREF){ memfind(tgt.ptr, tgt.len, pat.ptr, pat.len), pat.len }; }

MEMREF  fileref(char const *filename);
void    defileref(MEMREF mr);
//--------------|-----------------------------------------------
typedef struct bitmat_s BITMAT;

BITMAT *bitmat(int nrows, int ncols);
void    bitmat_destroy(BITMAT*);
void    bitmat_clear(BITMAT*);
BITMAT *bitmat_copy(BITMAT const*);
int     bitmat_rows(BITMAT const*) regargs;
int     bitmat_cols(BITMAT const*) regargs;
int     bitmat_get(BITMAT const*, int row, int col) regargs;
void    bitmat_set(BITMAT*, int row, int col, int val) regargs;
MEMREF  bitmat_ref(BITMAT const*);
BITMAT *bitmat_trans(BITMAT const*);
//--------------|-----------------------------------------------
// HASH FUNCTIONS
void    cessu32(uint8_t const *inp, int len, uint8_t out[32]);

uint32_t fnv04(char const *buf, int len);
uint64_t fnv08(char const *buf, int len);
// Not recommended: haven't worked out final mix step yet
void     fnv16(char const *buf, int len, char out[16]);
static inline uint32_t fnvstr(char const *buf)
    { return fnv04(buf, strlen(buf)); }

void mmhash3_x86_04(uint8_t const *key, int len, uint8_t out[ 4], uint32_t seed);
void mmhash3_x86_08(uint8_t const *key, int len, uint8_t out[ 8], uint32_t seed);
void mmhash3_x86_16(uint8_t const *key, int len, uint8_t out[16], uint32_t seed);
void mmhash3_x64_04(uint8_t const *inp, int len, uint8_t out[ 4], uint32_t seed);
void mmhash3_x64_08(uint8_t const *inp, int len, uint8_t out[ 8], uint32_t seed);
void mmhash3_x64_16(uint8_t const *key, int len, uint8_t out[16], uint32_t seed);
//--------------|-----------------------------------------------
// EZ socket library interface: nothing but "C" types and SOCK_OPT.
typedef enum {
      KEEPALIVE // TCP keepalive
    , NODELAY   // Nonbuffered TCP, for tty-like responsiveness
    , NOWAIT    // nonblocking I/O
    , RCVSIZE   // (size) buffer size
    , SNDSIZE   // (size) buffer size
    , CONERR    // connection state (0 == connected).
    , LINGER    // (secs) bg delay to flush output after close
    , SOCK_OPTS
} SOCK_OPT;

typedef char IPSTR[46]; // strlen(IPv4) < 16;  strlen(IPv6) < 46
// 123456789.123456789.123456789.123456789.123456789.
// xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx\0        - IPv6 max
// 0000:0000:0000:0000:0000:FFFF:ddd.ddd.ddd.ddd\0  - IPv4 over IPv6

int sock_accept(int skt);
int sock_addr(int skt, IPSTR ip, int *pport, char *name, int size);
int sock_bind(IPSTR const ip, int port);
int sock_connect(char const *host, int port, int nowait);
int sock_create(char const *path);
#ifdef linux
int sock_dest(int skt, IPSTR ip, int *pport);
#endif
int sock_getopt(int skt, SOCK_OPT);
int sock_open(char const *path);
int sock_peer(int skt, IPSTR ip, int *pport, char *name, int size);
int sock_recv(int skt, char *buf, int size);
int sock_recvfd(int skt, int *pfd, char *buf, int size);
int sock_send(int skt, char const*buf, int size);
int sock_sendfd(int skt, int fd, char const*buf, int size);
int sock_setopt(int skt, SOCK_OPT, int val);

int udp_open(IPSTR const ip, int port);
int udp_recv(int fd, char *buf, int size, IPSTR ip, int *port);
int udp_send(int fd, char const *buf, int size, IPSTR const ip, int port);
//--------------|-----------------------------------------------
//@map - variable-sized in-memory hash tables using open hashing.
//	"map" is a base implementation of (key,val) lookup.
//
// MAP map_create(diff,hash,context)
//	Create an empty hash table.
//      Hash table resizes itself whenever it reaches 75% full.
//      Caller is responsible for (key,val) allocation.
//	"diff"		- caller-supplied key-comparison function
//	"hash"		- caller-supplied key-hash function.
//	"context" 	- (void*) passed to "diff" and "hash".
//
// int map_count(m)
//	Returns the number of (key,val) pairs in hash table.
//
// void *map_set(m,key,val)
//	Set a (key,val) pair in hash table. (val) may be NULL.
//	Returns the previous (val) for the given (key) if any; else NULL.
//
// void *map_get(m,key)
//	Returns (val) for the given (key); or NULL.
//
// void *map_del(m,key)
//	Deletes (key,val) from hash table.
//	Returns the old (val) pointer, or NULL.
//
// void map_start(m)
// int map_next(m, &key, &val)
//	Iterate through hash table. Returns 0 when done.
//      "map_set" or "map_del" calls between "map_next" calls
//      has undefined effect on the scan.
//	EXAMPLE OF USE:
//		for (map_start(x); map_next(x,&key,&val);)
//			printf("%s %s\n", key, val);
//
// void *map_key(m,key)
//	Returns the (key) pointers stored in hash table that matches (key).
//      This can be used as an EXISTS test
//	This is only useful to a wrapper layer that manages
//	space allocation in the hash table.
//
// void map_resize(m,count)
//	Resize underlying space allocation to the minimum necessary to hold
//	"count" elements. Will never resize to less than map_count(m),
//	so "map_resize(m,0)" can be used to "pack" a hash table.
//	This reorganizes the table in two passes (qv Knuth) to ensure
//      minimum median access time (probes) for a successful query.
//
// The optional final arg passed to every diff() or hash() call
//  is the context supplied to map_create.

typedef struct map *MAP;
typedef const void *MKEY;
typedef unsigned long mhash_t;
typedef mhash_t       (map_hash)(MKEY, ...);
typedef int           (map_diff)(MKEY, MKEY, ...);

MAP     map_create(map_diff*diff, map_hash*map, void*context);
void    map_destroy(MAP);
int     map_count(MAP);
void *  map_get(const MAP, MKEY);
void *  map_set(MAP, MKEY, void *val);
void *  map_del(MAP, MKEY);
void    map_start(MAP);
int     map_next(MAP, MKEY*, void **pval);
void	map_resize(MAP, int limit);
MKEY    map_key(const MAP, MKEY);
//--------------|-----------------------------------------------
//@kvs - string:string hash table based on map.
//       A kvs 'owns' its key and value strings; they are
//       always COPIES of the kvs_set args.
typedef MAP KVS;

static inline KVS kvs_create(void)
{ return map_create((map_diff*)strcmp, (map_hash*)fnvstr, 0); }
void    kvs_destroy(KVS sh);
KVS     kvs_load(char const *file, char sep);
void	kvs_set(KVS sh, char const *key, char const *val);
void	kvs_del(KVS sh, char const *key);

static inline int kvs_count(KVS sh)  { return map_count(sh); }
static inline void kvs_start(KVS sh) { map_start(sh); }

static inline int kvs_next(KVS sh, char **kp, char **vp)
{ return map_next(sh, (void const**)kp, (void **)vp); }

static inline char const *kvs_get(KVS sh, char const *k)
{ return map_get(sh, k); }

static inline void kvs_resize(KVS sh, int n)
{ map_resize(sh, n); }

//--------------|-----------------------------------------------
//@cache: 
typedef char const*  CACHEKEY;
typedef uint32_t     CACHEVAL;
typedef struct cache CACHE;

CACHE*   cache_open(char const *path, int maxbytes);
void     cache_close(CACHE*);
CACHEVAL cache_get(CACHE*, CACHEKEY);
void     cache_set(CACHE*, CACHEKEY, CACHEVAL);

// To dump cache to a file and reload with a different maxbytes:
void     cache_save(CACHE const*, FILE*);
void     cache_load(CACHE*, FILE *);

//--------------|-----------------------------------------------
typedef struct thread_s THREAD;
typedef struct flag_s   FLAG;
typedef void *          THREAD_FN(void*);

THREAD* thread_start(THREAD_FN, void*arg);
int     thread_cancel(THREAD *pth);
void*   thread_wait(THREAD*);

FLAG*   flag_create(void);
void    flag_destroy(FLAG*);
int     flag_watch(FLAG*, double waitsecs); // wait for signal.
void    flag_any(FLAG*);   // signal any one watching thread.
void    flag_all(FLAG*);   // signal all watching threads.
//--------------|-----------------------------------------------
// VEC: dynamic vectors, akin to STL "vector<>"
typedef void DTOR(void *elem, /* void *context */...);
typedef struct {
    int		limit;
    int		count;
    char    	*ptr;
    int		width;
    DTOR	*dtor;
    void	*context;
} VEC;

static inline void*vec_data(VEC* v)  { return v ? v->ptr : 0; }
static inline int  vec_count(VEC* v) { return v ? v->count : 0; }
static inline int  vec_width(VEC* v) { return v ? v->width : 0; }

VEC* vec_new(int width, DTOR *dtor, void *context);
void vec_free(VEC*);
void vec_allow(VEC*, int limit);
void vec_resize(VEC*, int limit);
void*vec_push(VEC*, void *elem);

static inline void* _vel(VEC* v, int i)
{ return v->ptr + v->width * i; }

static inline void* vel(VEC* v, int i)
{ return v && v->ptr && i < v->count ? _vel(v,i) : NULL; }

static inline void*vec_pop(VEC* v)
{ return v && v->count ? _vel(v,--v->count) : 0; }
//--------------|-------|-------|-------------------------------
typedef VEC *STR;

static inline STR STRnew(void)
{
    STR     str = vec_new(sizeof(char), 0, 0);
    *str->ptr = 0;
    return  str;
}

static inline int   STRlen(STR str)  { return vec_count(str); }
static inline char* STRptr(STR str)  { return vec_data(str); }
static inline void  STRfree(STR str) { vec_free(str); }

static inline char* STRchr(STR str, char c)
{ return strchr(STRptr(str), c); }

STR STRcat(STR str, char const *s);
STR STRcpy(STR str, char const *s);

#define	STRcmp(str,s)	strcmp(STRptr(str), s)
#define	STRdup(s)	STRcpy(STRnew(), s)
#define	STRstr(str,s)	strstr(STRptr(str), s)

#define	STRCAT(a,b)	STRcat(a, STRptr(b))
#define	STRCMP(a,b)	strcmp(STRptr(a), STRptr(b))
#define	STRCPY(a,b)	STRcpy(a, STRptr(b))
#define	STRDUP(a)	STRdup(STRptr(a))
#define	STRSTR(a,b)	STRstr(a, STRptr(s))

static inline MEMREF STRREF(STR s)
{ return (MEMREF){STRptr(s), STRlen(s)}; }
//--------------|-----------------------------------------------

#define ROLLHASH_MOD 8355967

// rollhash_arg returns (256 ^ (leng - 1) mod ROLLHASH_MOD).
//  Most efficient to compute this once then pass it to rollhash_step.
//  Calling   rollhash_step(1, hash, data[i]*arg, data[i+leng])
//  amounts to the same thing. 

static inline uint32_t rollhash_arg(uint32_t leng)
{
    uint32_t arg = 1;
    while (--leng) arg = arg * 256 % ROLLHASH_MOD;
    return arg;
}

static inline uint32_t rollhash_init(uint8_t const*data, uint32_t leng)
{
    uint32_t hash = 0;
    while (leng--) hash = (hash * 256 + *data++) % ROLLHASH_MOD;
    return hash;
}

static inline uint32_t rollhash_step(uint32_t arg, uint32_t hash, uint8_t old, uint8_t new)
{
    return ((hash + 256*ROLLHASH_MOD - old * arg) % ROLLHASH_MOD * 256 + new) % ROLLHASH_MOD;
}
//--------------|-----------------------------------------------
static inline int bsrl(int x) 
{ asm("bsrl %0,%0":"=r"(x):"r"(x)); return x; }

static inline int bsrq(long long x) 
{ asm("bsrq %0,%0":"=r"(x):"r"(x)); return x; }

// CPUID(eax=0): (ebx,edx,ecx) = ("Genu","ineI","ntel").
// CPUID(eax=0): (ebx,edx,ecx) = ("htuA","itne","DMAc")
//      --- "AuthenticAMD". "DMAc" == 0x444D4163.
#define AMD_SIG     0x444D4163
#define Intel_SIG   0x6E74656c

static inline int cpuinfo(unsigned func)
{
    int     ret;
    asm("cpuid" : "=c" (ret) : "a" (func) : "ebx","edx");
    return  ret;
}

static inline int CacheLineWidth(void)  //in bytes
{
    return 255 & cpuinfo(cpuinfo(0) == AMD_SIG ? 0x80000005 : 0x80000006);
}

static inline int64_t rdtsc(void)
{ asm("xorl %eax,%eax; cpuid; rdtsc; ret"); return 0; }
// scan*: improved versions of str* routines, usin SSE2 registers.
char const* scanbrk(char const *tgt, char const *chrs);
int         scancmp(char const *s, char const *t);
char const* scanstr(char const *tgt, char const *pat);
char const* scanstr2(char const *tgt, char const pat[2]);
char const* scanstrx(char const *tgt, char const *pat);
//--------------|-----------------------------------------------
char*   acstr(char const*buf, int len);
MEMREF 	addr_part(MEMREF);
int     belch(char const *filename, MEMREF mem); // 0 on failure
int     bit_count(char const *vec, int len);
void    bitmat_prod(uint8_t *z, uint8_t const *a, uint8_t const *bt, int nrows, int nmids, int ncols);
int	bitwid(unsigned u);
char*   bndmem(char *tgt, int tgtlen, char *pat, int patlen);
int     cmpxm(void const*src, void const*dst, int nbytes);
MEMBUF	chomp(MEMBUF buf);  // trailing whitespace => \0's
void    die(char const *fmt, ...);
int     findbit_0(uint8_t const*vec, int nbytes);
int     findbit_1(uint8_t const*vec, int nbytes);
#ifndef __BSD_VISIBLE
static inline int fls(int x) 
{ if (!x) return 0; asm("bsrl %0,%0":"=a"(x):"a"(x)); return x+1; }
#endif
FILE*   fopenf(char const *mode, char const *fmt, ...);
#if defined(__linux__)
char const *getprogname(void);  // BSD-equivalent
#endif

// lohuff: calculate lowbit Huffman codes for a given frequency vector.
// freq[]: Symbol frequencies. No output code created where frequency is 0.
// code[]: Lowbit huffman codes, in the form (code << 8) + leng.
//	    Codes are bit-reversed: stream parsing starts at bit 0.
// (freq) and (code) may safely be the same block of memory.
//
void    lohuff(int len, unsigned freq[len], unsigned code[len]);
int     maccess(void const *mem, int len, int mode);
void    maccess_init(void);

// Return the length of the common prefix of src and dst; [0..len]
int memsame(void const*_src, void const*_dst, int len);

// ordhuff: generate ordered huffman code table, in linear time.
//  count: size of all three arrays.
//  freq:   relative frequency counts.
//  code:   codes, in low bits of each word.
//  bits:   witdth of each code, in bits.
//
void    ordhuff(int count, unsigned const freq[],
                unsigned code[], unsigned bits[]);

// Returns an array of pointers. Array and its data deleted with one free().
char**  phkeys(char const *psfile, int *nvals);

// "type" is an fopen type ("r", "w").
FILE*   popenf(char const *type, char const *fmt, ...);
#ifndef __BSD_VISIBLE

#endif

// Returns 0 if no suffix of s is also a prefix of s.
int     presuf(MEMREF);
MEMREF* refsplit(char *text, char sep, int *pnrefs);

// Reverses bits in word
static inline uint32_t revbit32(uint32_t x)
{
    uint32_t    y;
    y = x & 0x55555555;
    x = ((x - y) >> 1) + (y << 1);
    y = x & 0x33333333;
    x = ((x - y) >> 2) + (y << 2);
    y = x & 0x0F0F0F0F;
    x = ((x - y) >> 4) + (y << 4);
    return  bswap_32(x);
}

MEMBUF	    slurp(char const *filename);
char*       strim(char*);
int         systemf(char const *fmt, ...);
double      tick(void);

// tolog(&stderr) redirects stderr output to syslog.
//  fileno(stderr) is unchanged. Default priority is LOG_INFO
//  If the message begins with a std priority level
//      (EMERG ALERT CRIT ERR WARNING NOTICE INFO DEBUG)
//  it is parsed as such; e.g.:
//      fprintf(stderr, "ERR:   No such file: %s", filename)
//  invokes :
//      syslog(LOG_ERR, "No such file: ...")

void tolog(FILE**);

static inline unsigned under(unsigned x)
{ return (x - 1) & ~x; }

typedef struct {
    MEMREF scheme, auth, host, port, path, query, fragment;
} URIREF;
void uri_parts(char const *request, URIREF *uri);

static inline void usage(char const *str)
{
    fprintf(stderr, "Usage: %s %s\n", getprogname(), str);
    _exit(2);
}

typedef struct { unsigned yyyy, mm, dd; } YMD;
unsigned ymd2day(YMD);
YMD	 day2ymd(unsigned nDate);

extern char const *eainame[];   // ret = getaddrinfo(...); puts(eainame[-ret]);
extern int  const neainames;

extern char const *errname[];
extern int  const nerrnames;

extern char const *signame[];
extern int  const nsignames;

#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif//MSUTIL_H
