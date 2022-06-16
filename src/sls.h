#ifndef SLS_H
#define SLS_H

#include <stdint.h>

#ifndef _U_TYPE
#define _U_TYPE
typedef int8_t			s8; 
typedef int16_t			s16;
typedef int32_t			s32;
typedef int64_t			s64;
typedef uint8_t			u8; 
typedef uint16_t		u16;
typedef uint32_t		u32;
typedef uint64_t		u64;
#endif 

typedef struct {
	u32 emb_row;		// R, embedding-size
	u32 emb_col;		// C, feature-size
	u32 lengths;		// num-indices-per-lookup
	u32 lengths_size;	// K, tables
	u32 *ids;			// ids-list
} sls_config;


void sls_io_buf(const char *table, sls_config *config, int flag);
void sls_io_unbuf(const char *table, sls_config *config, int flag);
void sls_mmap(const char *table, sls_config *config, int flag);
void sls_ram(const char *table, sls_config *config, int flag);

#endif
