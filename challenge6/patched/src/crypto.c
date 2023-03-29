// Taken from https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <string.h>

void handleErrors(void);


void digest_message(const unsigned char *message, size_t message_len, unsigned char **digest, unsigned int *digest_len);


int main(){
    const unsigned char *plain = (unsigned char *)"The quick brown fox jumps over the lazy dog";
    unsigned char *digest;
    unsigned int d_len;
    printf("Message: \"%s\"\nLength: %lu\n", plain, strlen((const char*)plain));
    digest_message(plain, sizeof(plain), &digest, &d_len);
    printf("Digest Length (bytes): %u (bits): %u\n", d_len, d_len*8);
    BIO_dump_fp(stdout, (const char *)digest, d_len);
    return 1;
}


void digest_message(const unsigned char *message, size_t message_len, unsigned char **digest, unsigned int *digest_len)
{
	EVP_MD_CTX *mdctx;

	if((mdctx = EVP_MD_CTX_new()) == NULL)
		handleErrors();

	if(1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
		handleErrors();

	if(1 != EVP_DigestUpdate(mdctx, message, message_len))
		handleErrors();

	if((*digest = (unsigned char *)OPENSSL_malloc(EVP_MD_size(EVP_sha256()))) == NULL)
		handleErrors();

	if(1 != EVP_DigestFinal_ex(mdctx, *digest, digest_len))
		handleErrors();

	EVP_MD_CTX_free(mdctx);
}


void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}
