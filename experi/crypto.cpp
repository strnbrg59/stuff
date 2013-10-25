/**
  AES encryption/decryption demo program using OpenSSL EVP apis
  gcc -Wall openssl_aes.c -lcrypto

  this is public domain code. 

  Saju Pillai (saju.pillai@gmail.com)

  Minor modifications by tsternberg
  Compile with "g++ -g -Wall -o crypto crypto.cpp -lssl".
**/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <boost/format.hpp>

int aes_init(unsigned char *enc_key, int enc_key_len, unsigned char *salt,
             EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx);
int
aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len,
            unsigned char **ciphertext);
int
aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len,
            unsigned char **plaintext);

/**
 * Create an 256 bit key and IV using the supplied enc_key. salt can be added
 * for taste.
 * Fills in the encryption and decryption ctx objects and returns 0 on success
 **/
int aes_init(unsigned char *enc_key, int enc_key_len, unsigned char *salt,
             EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx)
{
  int i, nrounds = 1;
  unsigned char key[32], iv[32];
  
  /*
   * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the
   * supplied key material.
   * nrounds is the number of times the we hash the material. More rounds are
   * more secure but slower.
   */
  i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_md5(), salt, enc_key,
                     enc_key_len, nrounds, key, iv);
  if (i != 32) {
    printf("Key size is %d bits - should be 256 bits\n", i);
    return -1;
  }

  EVP_CIPHER_CTX_init(e_ctx);
  EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
  EVP_CIPHER_CTX_init(d_ctx);
  EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, key, iv);

  return 0;
}

/*
 * Encrypt *len bytes of data
 * All data going in & out is considered binary (unsigned char[])
 *
 * Client must free *ciphertext (but we malloc it in this function).
 */
int
aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len,
            unsigned char **ciphertext)
{
  /* max ciphertext len for a n bytes of plaintext is
   * n + AES_BLOCK_SIZE -1 bytes
   */
  int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
  *ciphertext = (unsigned char*)malloc(c_len);

  /* allows reusing of 'e' for multiple encryption cycles */
  EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

  /* update ciphertext, c_len is filled with the length of ciphertext generated,
    *len is the size of plaintext in bytes */
  EVP_EncryptUpdate(e, *ciphertext, &c_len, plaintext, *len);

  /* update ciphertext with the final remaining bytes */
  EVP_EncryptFinal_ex(e, *ciphertext+c_len, &f_len);

  *len = c_len + f_len;

  return 0;
}

/*
 * Decrypt *len bytes of ciphertext
 *
 * Client must free plaintext (but we malloc it here).
 */
int
aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len,
            unsigned char **plaintext)
{
  /* plaintext will always be equal to or lesser than length of ciphertext*/
  int p_len = *len, f_len = 0;
  *plaintext = (unsigned char*)malloc(p_len);
  
  EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
  EVP_DecryptUpdate(e, *plaintext, &p_len, ciphertext, *len);
  EVP_DecryptFinal_ex(e, *plaintext+p_len, &f_len);

  *len = p_len + f_len;
  
  return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <encryption-key>\n", argv[0]);
        exit(1);
    }

  /* "opaque" encryption, decryption ctx structures that libcrypto uses to
     record status of enc/dec operations */
  EVP_CIPHER_CTX en, de;

  /* 8 bytes to salt the enc_key during key generation. This is an example of
     compiled in salt. We just read the bit pattern created by these two 4 byte 
     integers on the stack as 64 bits of contigous salt material - 
     ofcourse this only works if sizeof(int) >= 4 */
//  unsigned int salt[] = {12345, 54321};
  unsigned char *enc_key;
  int enc_key_len;

  /* the enc_key is read from the argument list */
  enc_key = (unsigned char *)argv[1];
  enc_key_len = strlen(argv[1]);
  
  /* gen key and iv. init the cipher ctx object */
  if (aes_init(enc_key, enc_key_len, NULL /*salt*/, &en, &de)) {
    printf("Couldn't initialize AES cipher\n");
    return -1;
  }

  /* encrypt and decrypt each input string and compare with the original */
  const int plaintext_len = 1000;
  char plaintext[plaintext_len+1];
  for (int i=0;i<plaintext_len;++i) plaintext[i] = 'a' + i%26;
  plaintext[plaintext_len] = 0x0;
  unsigned char *ciphertext;
  unsigned char *reconstituted_plaintext;

  int olen, len;
    
  /* The enc/dec functions deal with binary data and not C strings. strlen()
     will return length of the string without counting the '\0' string marker.
     We always pass in the marker byte to the encrypt/decrypt functions so
     that after decryption we end up with a legal C string */
  olen = len = strlen(plaintext)+1;
    
  aes_encrypt(&en, (unsigned char *)plaintext, &len, &ciphertext);
  aes_decrypt(&de, ciphertext, &len, &reconstituted_plaintext);

  if (strncmp((const char*)reconstituted_plaintext, plaintext, olen)) {
      printf("FAIL: enc/dec failed for \"%s\"\n", plaintext);
      exit(1);
  }

  /*
   * Compare ciphertext to output of /usr/bin/openssl.
   */
  int fd = open("/tmp/crypto.programmatic", O_RDWR|O_CREAT);
  write(fd, ciphertext, len);
  close(fd);
  boost::format fmt =
      boost::format("echo -n %s | "
         "/usr/bin/openssl enc -nosalt -aes-256-cbc -k %s -e -out %s")
        % plaintext % enc_key % "/tmp/crypto.shell";
  system(fmt.str().c_str());
    
  free(ciphertext);
  free(reconstituted_plaintext);

  EVP_CIPHER_CTX_cleanup(&en);
  EVP_CIPHER_CTX_cleanup(&de);

  return 0;
}
