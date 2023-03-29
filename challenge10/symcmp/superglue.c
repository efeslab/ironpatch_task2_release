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

	// concrete can_id 
	// the entire can_id needs to be concretized to allow this challenge to proceed past the pgn array
	((unsigned char*)(symboic_can_frame+0))[0] = 0x00;
	((unsigned char*)(symboic_can_frame+0))[1] = 0x7f;
	((unsigned char*)(symboic_can_frame+0))[2] = 0xec;
	((unsigned char*)(symboic_can_frame+0))[3] = 0x98;

	// can_id of second can frame
	((unsigned char*)(symboic_can_frame+1))[0] = 0x00;
	((unsigned char*)(symboic_can_frame+1))[1] = 0x7f;
	((unsigned char*)(symboic_can_frame+1))[2] = 0xec;
	((unsigned char*)(symboic_can_frame+1))[3] = 0x98;

	memcpy(&patched_symbolic_ready, &symbolic_ready, sizeof(symbolic_ready));
	memcpy(&vuln_symbolic_ready, &symbolic_ready, sizeof(symbolic_ready));

	memcpy(&patched_symboic_can_frame, &symboic_can_frame, sizeof(symboic_can_frame));
	memcpy(&vuln_symboic_can_frame, &symboic_can_frame, sizeof(symboic_can_frame));

	printf("======================== calling vuln_main\n");
	vuln_main(0, NULL);
	printf("======================== calling patched_main\n");
	patched_main(0, NULL);
	
	int vuln_size = 0, patched_size = 0;
	unsigned char *patched_p = patched_output, *vuln_p = vuln_output;
	unsigned char *patched_key = NULL, *vuln_key = NULL;
	unsigned char *patched_buf = NULL, *vuln_buf = NULL;

	vuln_p = consume_file(vuln_p, &vuln_size, &vuln_key);
	patched_p = consume_file(patched_p, &patched_size, &patched_key);
	klee_assert(vuln_size == patched_size);
	klee_assert(vuln_size == KEY_SIZE);
	klee_assert(memcmp(vuln_key, patched_key, KEY_SIZE) == 0);

	vuln_p = consume_file(vuln_p, &vuln_size, &vuln_buf);
	patched_p = consume_file(patched_p, &patched_size, &patched_buf);
	klee_assert(vuln_size == patched_size);
	klee_assert(vuln_size == IV_SIZE);
	klee_assert(memcmp(vuln_buf, patched_buf, IV_SIZE) == 0);

	while(1) {
		vuln_p = consume_file(vuln_p, &vuln_size, &vuln_buf);
		patched_p = consume_file(patched_p, &patched_size, &patched_buf);
		if (vuln_size == 0) {
			assert(patched_size == 0);
			break;
		}
		klee_assert(patched_size == 137);
		klee_assert(vuln_size == 137);

		unsigned char patched_plaintext[patched_size], vuln_plaintext[vuln_size];
		patched_fake_aes_decrypt(patched_buf, patched_size - 8, patched_key, KEY_SIZE, patched_plaintext);
		vuln_fake_aes_decrypt(vuln_buf, vuln_size - 8, vuln_key, KEY_SIZE, vuln_plaintext);

		klee_assert(patched_size == vuln_size);
		klee_assert(memcmp(patched_plaintext, vuln_plaintext, 97) == 0);

		// verify vuln hash:
		unsigned char* vuln_hash = malloc(32);
		unsigned int vuln_hash_len = 0;
		vuln_digest_message(vuln_plaintext, 97, &vuln_hash, &vuln_hash_len);
		klee_assert(vuln_hash_len == 32);
		klee_assert(memcmp(vuln_hash, vuln_plaintext + 97, 32) == 0);
		free(vuln_hash);

		// varify patched hash
		unsigned char* patched_hash = malloc(32);
		unsigned int patched_hash_len = 0;
		patched_digest_message(patched_plaintext, 97, &patched_hash, &patched_hash_len);
		klee_assert(patched_hash_len == 32);
		klee_assert(memcmp(patched_hash, patched_plaintext + 97, 32) == 0);
		free(patched_hash);

		vuln_p = consume_file(vuln_p, &vuln_size, &vuln_buf);
		patched_p = consume_file(patched_p, &patched_size, &patched_buf);
		klee_assert(vuln_size == patched_size);
		klee_assert(vuln_size == IV_SIZE);
		klee_assert(memcmp(vuln_buf, patched_buf, IV_SIZE) == 0);
	}
}
