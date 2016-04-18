#define _GNU_SOURCE

#define GELF_HEADER_SIZE 12
#define GELF_MAX_PACKETS 128
#define GELF_MAX_PACKET_SIZE 1024

#include "vrt.h"
#include "cache/cache.h"
#include "vcc_if.h"

#include <arpa/inet.h>
#include <math.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <zlib.h>

typedef struct cache {
  int sockfd;
} cache_t;


static void free_mc_vcl_cache(void *data)
{
    cache_t *cache = (cache_t*)data;

    if (cache->sockfd != -1) {
        close(cache->sockfd);
    }

    free(cache);
}

int init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
    cache_t *cache;

    cache = calloc(1, sizeof(cache_t));

    AN(cache);

    cache->sockfd = -1;

    priv->priv = cache;
    priv->free = free_mc_vcl_cache;

    return 0;
}

VCL_VOID vmod_send(const struct vrt_ctx *ctx, struct vmod_priv *priv, VCL_STRING value, VCL_STRING host, VCL_INT port)
{
    struct sockaddr_in destaddr;
    struct timeval tv;
    char compressed_value[GELF_MAX_PACKETS * GELF_MAX_PACKET_SIZE];
    char udp_packet[GELF_MAX_PACKET_SIZE];
    char gelf_header[GELF_HEADER_SIZE];
    int compressed_value_size;
    int gelf_chunk_size = GELF_MAX_PACKET_SIZE - GELF_HEADER_SIZE;
    double left_overs;
    uint8_t chunk_index = 0;
    uint8_t total_chunks = 1;
    z_stream defstream;
    cache_t *cache = (cache_t*)priv->priv;

    if (cache->sockfd == -1) {
        cache->sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;

    defstream.avail_in = strlen(value); // size of input, string - terminator
    defstream.next_in = (Bytef *)value; // input char array
    defstream.avail_out = sizeof(compressed_value); // size of output
    defstream.next_out = (Bytef *)compressed_value; // output char array

    // the actual compression work.
    deflateInit(&defstream, Z_BEST_SPEED);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    compressed_value_size = (char*)defstream.next_out - compressed_value;

    total_chunks = ceil((float) compressed_value_size / gelf_chunk_size);

    // This byte here should be is a 9c when using gzcompress but here it's not.
    // logstash checks for this magic byte sequence to detect compression ( 78,9c ), 78 is good :
    compressed_value[1] = 0x9c;

    gettimeofday(&tv,NULL);

    bzero(&destaddr, sizeof(destaddr));

    destaddr.sin_family = AF_INET;
    destaddr.sin_addr.s_addr = inet_addr(host);
    destaddr.sin_port = htons(port);

    while(chunk_index +1 <= total_chunks) {

        //by default we get max chunk size
        left_overs = gelf_chunk_size;

        //if it's the only chunk, left_overs is compressed_value_size
        if(total_chunks == 1)
            left_overs =  compressed_value_size;

        // else if we're at the last chunk
        else if(chunk_index + 1 == total_chunks)
        //remaining chars = strlen(line) % total_chunks
            left_overs =  fmod((float) compressed_value_size, (float) gelf_chunk_size);

        // build our gelf header (12 bytes) :

        gelf_header[0] = 0x1e;
        gelf_header[1] = 0x0f;

        // id is from tv_usec.tv_sec
        memcpy(gelf_header+2, &tv.tv_usec, 4);
        memcpy(gelf_header+6, &tv.tv_usec, 4);

        // Add chunk index and total chunk
        memcpy(gelf_header+10, &chunk_index ,1);
        memcpy(gelf_header+11, &total_chunks, 1);

        // Forge our UDP Packet :
        memcpy(udp_packet, gelf_header, GELF_HEADER_SIZE);
        memcpy(udp_packet + GELF_HEADER_SIZE, &compressed_value[chunk_index * gelf_chunk_size], left_overs+1);

        sendto(cache->sockfd, udp_packet, ((int) left_overs + GELF_HEADER_SIZE), 0, (struct sockaddr *) &destaddr, sizeof(destaddr));
        // Increase our chunk index
        chunk_index++;
    }

    return;
}
