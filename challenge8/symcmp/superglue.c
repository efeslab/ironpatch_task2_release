#include <stdio.h>
#include <openssl/ossl_typ.h>
#include <openssl/evp.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <string.h>
#include <byteswap.h>

#include <assert.h>
#include "klee.h"
#include "../glue.h"

extern unsigned char vuln_output[OUTPUT_SIZE];
extern unsigned long vuln_output_idx;
extern unsigned char patched_output[OUTPUT_SIZE];
extern unsigned long patched_output_idx;

int symbolic_ready[NR_ITERATIONS] = {1, 1, 1};
struct can_frame symboic_can_frame[NR_ITERATIONS] = {0};
extern int patched_symbolic_ready[NR_ITERATIONS];
extern int vuln_symbolic_ready[NR_ITERATIONS];
extern struct can_frame patched_symboic_can_frame[NR_ITERATIONS];
extern struct can_frame vuln_symboic_can_frame[NR_ITERATIONS];

int patched_main(int argc, char * argv[]);
int vuln_main(int argc, char * argv[]);

/*
 * input: file - the input file
 * function returns pointer to continue reading from
 * size, buf - outputs. Buf is the buffer in this section and size if the size of the section
 */
unsigned char* consume_file(unsigned char* file, int* size, unsigned char** buf)
{
	int i = 0;
	*size = *((int*)file);
	*buf = file + sizeof(int);
	return file + *size + sizeof(int);
}

int main()
{
#if 1
	klee_make_symbolic(symbolic_ready, sizeof(symbolic_ready), "symbolic_ready");
	klee_make_symbolic(symboic_can_frame, sizeof(symboic_can_frame), "symboic_can_frame");
#endif

	((char*)(symboic_can_frame+0))[0] = 0x00; // can_id
	((char*)(symboic_can_frame+0))[1] = 0x7f; // can_id
	((char*)(symboic_can_frame+0))[9] = 0x00; // affects size of malloc
	((char*)(symboic_can_frame+0))[10] = 0x01; // affects size of malloc

	uint16_t size = SIZE;
	size = bswap_16(size);
	memcpy(&symboic_can_frame[0].data[1], &size, 2);


	((char*)(symboic_can_frame+1))[0] = 0x00; // can_id
	((char*)(symboic_can_frame+1))[1] = 0x7f; // can_id
	((char*)(symboic_can_frame+1))[2] = 0xeb; // type of packet (if I let it be symbolic, than the size has to be concrete)


	memcpy(&patched_symbolic_ready, &symbolic_ready, sizeof(symbolic_ready));
	memcpy(&vuln_symbolic_ready, &symbolic_ready, sizeof(symbolic_ready));

	memcpy(&patched_symboic_can_frame, &symboic_can_frame, sizeof(symboic_can_frame));
	memcpy(&vuln_symboic_can_frame, &symboic_can_frame, sizeof(symboic_can_frame));

#if 0
	printf("======================== calling vuln_main\n");
	vuln_main(0, NULL);
#else
	printf("======================== calling patched_main\n");
	patched_main(0, NULL);
#endif
}
