/********************************************************************/
/* Copyright(c) 2014, Intel Corp.                                   */
/* Developers and authors: Shay Gueron (1) (2)                      */
/* (1) University of Haifa, Israel                                  */
/* (2) Intel, Israel                                                */
/* IPG, Architecture, Israel Development Center, Haifa, Israel      */
/********************************************************************/
#ifndef AES_ECB_H
#define AES_ECB_H
/******************************************************************************
Function parameters:
unsigned char *in  - pointer to the data that will be encrypted/decrypted
unsigned char *out - pointer to the buffer where encrypted/decrypted data will be stored
unsigned long length - length of the data in bytes
int nbits - length of the key in bits  128/192/256
AES_KEY *key - the pre expanded key schedule, expanded using any of the key expansion functions
unsigned char *KS - the KEY part of AES_KEY
int nr -the nr part of AES_KEY
******************************************************************************/
/*These functions receive a pointer to the Key Schedule*/

#ifdef __cplusplus
extern "C" {
#endif

void AES_ECB_encrypt(const unsigned char *in,
                     unsigned char *out,
                     unsigned long length,     
                     const unsigned char *KS,
                     int nr);

void AES_ECB_encrypt_in_fours(const unsigned char *in,
	unsigned char *out,
	unsigned long length,
	const unsigned char *KS,
	int nr);


void AES_ECB_encrypt_in_three(const unsigned char *in,
	unsigned char *out,
	const unsigned char *key,
	int nr);


void AES_ECB_encrypt_in_two(const unsigned char *in,
	unsigned char *out,
	const unsigned char *key,
	int nr);

void AES_ECB_encrypt_in_one(const unsigned char *in,
	unsigned char *out,
	const unsigned char *key,
	int nr);

void AES_ECB_decrypt(const unsigned char *in, 
                     unsigned char *out,
                     unsigned long length,        
                     const unsigned char *KS,
                     int nr);

#ifdef __cplusplus
};
#endif
#endif