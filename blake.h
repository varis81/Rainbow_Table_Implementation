#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>       /* time */
#include <inttypes.h>
#include <string.h>

#define U8TO32_BIG(p)					      \
  (((uint32_t)((p)[0]) << 24) | ((uint32_t)((p)[1]) << 16) |  \
   ((uint32_t)((p)[2]) <<  8) | ((uint32_t)((p)[3])      ))

#define U32TO8_BIG(p, v)				        \
  (p)[0] = (uint8_t)((v) >> 24); (p)[1] = (uint8_t)((v) >> 16); \
  (p)[2] = (uint8_t)((v) >>  8); (p)[3] = (uint8_t)((v)      );

#define U8TO64_BIG(p) \
  (((uint64_t)U8TO32_BIG(p) << 32) | (uint64_t)U8TO32_BIG((p) + 4))

#define U64TO8_BIG(p, v)		      \
  U32TO8_BIG((p),     (uint32_t)((v) >> 32)); \
  U32TO8_BIG((p) + 4, (uint32_t)((v)      ));

typedef struct
{
  uint32_t h[8], s[4], t[2];
  int buflen, nullt;
  uint8_t  buf[64];
} state256;

typedef state256 state224;

typedef struct
{
  uint64_t h[8], s[4], t[2];
  int buflen, nullt;
  uint8_t buf[128];
} state512;

typedef state512 state384;

void blake256_compress(state256*,const uint8_t*);
void blake256_init( state256*);
void blake256_update( state256*,const uint8_t*, uint64_t);
void blake256_final( state256*, uint8_t* );
void blake256_hash( uint8_t *, const uint8_t *, uint64_t );
void blake256_test();

unsigned long generateHash(const char *, int);
void Reduce(char* ,int ,char* );



