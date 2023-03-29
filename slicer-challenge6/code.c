#include <assert.h>
#include <string.h>

int vuln_g_break_status = 0;
int vuln_g_turn_status = 0;
unsigned int vuln_g_log[2] = {0};

int patched_g_break_status = 0;
int patched_g_turn_status = 0;
unsigned int patched_g_log[2] = {0};


////////// //////////////////// fake crypto begin /////////////////////
void fake_aes_encrypt(
		const unsigned char* plaintext, unsigned long plaintext_len,
		const unsigned char* key, unsigned long key_len,
		unsigned char* ciphertext) {
	unsigned long key_idx = 0;

	for(int i = 0 ; i < plaintext_len ; i ++) {
		unsigned int p = plaintext[i];
		// rotare the byte one bit to the left

		p <<= 1;
		p |= ((p >> 8) & 1);
		p &= (1 << 8) - 1;

		//xor with key byte
		p ^= key[key_idx];
		key_idx ++;
		key_idx %= key_len;

		ciphertext[i] = p;
	}
}

void fake_aes_decrypt(
		const unsigned char* ciphertext, unsigned long ciphertext_len,
		const unsigned char* key, unsigned long key_len,
		unsigned char* plaintext) {
	unsigned long key_idx = 0;

	for(int i = 0 ; i < ciphertext_len ; i ++) {
		unsigned int p = ciphertext[i];

		//xor with key byte
		p ^= key[key_idx];
		key_idx ++;
		key_idx %= key_len;

		// rotate the byte one bit to the right
		int lsb = p & 1;
		p >>= 1;
		p |= (lsb << 7);

		plaintext[i] = p;
	}
}
////////////////////////////// fake crypto end /////////////////////

////////////////////////////// Turn funcs begin /////////////////////
void vuln_toggle_turn(int x)
{
	vuln_g_turn_status = 1 - vuln_g_turn_status;
}

void patched_toggle_turn(int x)
{
	patched_g_turn_status = 1 - patched_g_turn_status;
}
////////////////////////////// Turn funcs end /////////////////////

////////////////////////////// Break funcs begin /////////////////////
void vuln_toggle_break(int x)
{
	if (x > 0)
		vuln_g_break_status = 1 - vuln_g_break_status;
}

void patched_toggle_break(int x)
{
	if (x != 0)
		patched_g_break_status = 1 - patched_g_break_status;
}
////////////////////////////// Break funcs end /////////////////////

////////////////////////////// Log funcs begin /////////////////////
void vuln_log()
{
	unsigned int plaintext[2];
	plaintext[0] = vuln_g_break_status;
	plaintext[1] = vuln_g_turn_status;
	unsigned char key[] = {1,2,3,4};
	
	fake_aes_encrypt((unsigned char*)plaintext, sizeof(plaintext), key, 4, (unsigned char*)vuln_g_log);
}

void patched_log()
{
	unsigned int plaintext[2];
	plaintext[0] = patched_g_break_status;
	plaintext[1] = patched_g_turn_status;
	unsigned char key[] = {1,2,3,4};
	
	fake_aes_encrypt((unsigned char*)plaintext, sizeof(plaintext), key, 4, (unsigned char*)patched_g_log);
}
////////////////////////////// Log funcs end /////////////////////
int main()
{
	char input[8] = { 5, 5, 5, 7, 5, 5, 5, 5 }; // TODO: KLEE make symbolic

	vuln_toggle_break(input[3]);
	vuln_toggle_turn(input[2]);
	vuln_log();

	patched_toggle_break(input[3]);
	patched_toggle_turn(input[2]);
	patched_log();

	unsigned int vuln_plaintext[2] = {0};
	unsigned int patched_plaintext[2] = {0};
	unsigned char key[] = {1,2,3,4};
	fake_aes_decrypt((unsigned char*)vuln_g_log, sizeof(vuln_g_log), key, 4, (unsigned char*)vuln_plaintext);
	fake_aes_decrypt((unsigned char*)patched_g_log, sizeof(patched_g_log), key, 4, (unsigned char*)patched_plaintext);
	//assert(vuln_g_break_status == patched_g_break_status); // Slicing criteria
	assert(memcmp(vuln_plaintext, patched_plaintext, sizeof(vuln_plaintext)) == 0); // Slicing criteria
}

