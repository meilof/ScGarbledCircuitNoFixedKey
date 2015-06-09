/********************************************************************/
/* Copyright(c) 2014, Intel Corp.                                   */
/* Developers and authors: Shay Gueron (1) (2)                      */
/* (1) University of Haifa, Israel                                  */
/* (2) Intel, Israel                                                */
/* IPG, Architecture, Israel Development Center, Haifa, Israel      */
/********************************************************************/
#include "ecb.h"
#include <wmmintrin.h>

#if defined(__INTEL_COMPILER)
# include <ia32intrin.h> 
#elif defined(__GNUC__)
# include <emmintrin.h>
# include <smmintrin.h>
#endif
/*****************************************************************************/
void AES_ECB_encrypt(const unsigned char *in, 
                     unsigned char *out,
                     unsigned long length,
                     const unsigned char *key,
                     int nr)
    {
    __m128i tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7,tmp8;
    __m128i *Key_Schedule = (__m128i*)key;
    int i,j;

    if (length%16)
        length = length/16 + 1;
    else length/=16;

    for(i=0; i < length/8; i++){
        tmp1 = _mm_loadu_si128 (&((__m128i*)in)[i*8+0]);
        tmp2 = _mm_loadu_si128 (&((__m128i*)in)[i*8+1]);
        tmp3 = _mm_loadu_si128 (&((__m128i*)in)[i*8+2]);
        tmp4 = _mm_loadu_si128 (&((__m128i*)in)[i*8+3]);
        tmp5 = _mm_loadu_si128 (&((__m128i*)in)[i*8+4]);
        tmp6 = _mm_loadu_si128 (&((__m128i*)in)[i*8+5]);
        tmp7 = _mm_loadu_si128 (&((__m128i*)in)[i*8+6]);
        tmp8 = _mm_loadu_si128 (&((__m128i*)in)[i*8+7]);

        tmp1 = _mm_xor_si128 (tmp1,Key_Schedule[0]);
        tmp2 = _mm_xor_si128 (tmp2,Key_Schedule[0]);
        tmp3 = _mm_xor_si128 (tmp3,Key_Schedule[0]);
        tmp4 = _mm_xor_si128 (tmp4,Key_Schedule[0]);
        tmp5 = _mm_xor_si128 (tmp5,Key_Schedule[0]);
        tmp6 = _mm_xor_si128 (tmp6,Key_Schedule[0]);
        tmp7 = _mm_xor_si128 (tmp7,Key_Schedule[0]);
        tmp8 = _mm_xor_si128 (tmp8,Key_Schedule[0]);
   
        for(j=1; j < nr; j++){
            tmp1 = _mm_aesenc_si128 (tmp1,Key_Schedule[j]);
            tmp2 = _mm_aesenc_si128 (tmp2,Key_Schedule[j]);
            tmp3 = _mm_aesenc_si128 (tmp3,Key_Schedule[j]);
            tmp4 = _mm_aesenc_si128 (tmp4,Key_Schedule[j]);
            tmp5 = _mm_aesenc_si128 (tmp5,Key_Schedule[j]);
            tmp6 = _mm_aesenc_si128 (tmp6,Key_Schedule[j]);
            tmp7 = _mm_aesenc_si128 (tmp7,Key_Schedule[j]);
            tmp8 = _mm_aesenc_si128 (tmp8,Key_Schedule[j]);
        }

        tmp1 = _mm_aesenclast_si128 (tmp1,Key_Schedule[j]);
        tmp2 = _mm_aesenclast_si128 (tmp2,Key_Schedule[j]);
        tmp3 = _mm_aesenclast_si128 (tmp3,Key_Schedule[j]);
        tmp4 = _mm_aesenclast_si128 (tmp4,Key_Schedule[j]);
        tmp5 = _mm_aesenclast_si128 (tmp5,Key_Schedule[j]);
        tmp6 = _mm_aesenclast_si128 (tmp6,Key_Schedule[j]);
        tmp7 = _mm_aesenclast_si128 (tmp7,Key_Schedule[j]);
        tmp8 = _mm_aesenclast_si128 (tmp8,Key_Schedule[j]);

        _mm_storeu_si128 (&((__m128i*)out)[i*8+0],tmp1);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+1],tmp2);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+2],tmp3);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+3],tmp4);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+4],tmp5);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+5],tmp6);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+6],tmp7);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+7],tmp8);
        }

    for(j=i*8;j<length;j++){
        tmp1 = _mm_loadu_si128 (&((__m128i*)in)[j]);
        tmp1 = _mm_xor_si128 (tmp1,Key_Schedule[0]);
        for(i=1; i < nr; i++)
            tmp1 = _mm_aesenc_si128 (tmp1,Key_Schedule[i]);
        tmp1 = _mm_aesenclast_si128 (tmp1,Key_Schedule[i]);
        _mm_storeu_si128 (&((__m128i*)out)[j],tmp1);
        }
    }

void AES_ECB_encrypt_in_fours(const unsigned char *in,
	unsigned char *out,
	unsigned long length,
	const unsigned char *key,
	int nr)
	{

	__m128i tmp1, tmp2, tmp3, tmp4;
	__m128i *Key_Schedule = (__m128i*)key;
	int i, j;

	if (length % 16)
		length = length / 16 + 1;
	else length /= 16;

	
		tmp1 = _mm_loadu_si128(&((__m128i*)in)[0]);
		tmp2 = _mm_loadu_si128(&((__m128i*)in)[1]);
		tmp3 = _mm_loadu_si128(&((__m128i*)in)[2]);
		tmp4 = _mm_loadu_si128(&((__m128i*)in)[3]);


		tmp1 = _mm_xor_si128(tmp1, Key_Schedule[0]);
		tmp2 = _mm_xor_si128(tmp2, Key_Schedule[0]);
		tmp3 = _mm_xor_si128(tmp3, Key_Schedule[0]);
		tmp4 = _mm_xor_si128(tmp4, Key_Schedule[0]);

		for (j = 1; j < nr; j++){
			tmp1 = _mm_aesenc_si128(tmp1, Key_Schedule[j]);
			tmp2 = _mm_aesenc_si128(tmp2, Key_Schedule[j]);
			tmp3 = _mm_aesenc_si128(tmp3, Key_Schedule[j]);
			tmp4 = _mm_aesenc_si128(tmp4, Key_Schedule[j]);

		}

		tmp1 = _mm_aesenclast_si128(tmp1, Key_Schedule[j]);
		tmp2 = _mm_aesenclast_si128(tmp2, Key_Schedule[j]);
		tmp3 = _mm_aesenclast_si128(tmp3, Key_Schedule[j]);
		tmp4 = _mm_aesenclast_si128(tmp4, Key_Schedule[j]);

		_mm_storeu_si128(&((__m128i*)out)[0], tmp1);
		_mm_storeu_si128(&((__m128i*)out)[1], tmp2);
		_mm_storeu_si128(&((__m128i*)out)[2], tmp3);
		_mm_storeu_si128(&((__m128i*)out)[3], tmp4);
		

	

}

void AES_ECB_encrypt_in_two(const unsigned char *in,
	unsigned char *out,
	const unsigned char *key,
	int nr)
{

	__m128i tmp1, tmp2;
	__m128i *Key_Schedule = (__m128i*)key;
	int i, j;

	


	tmp1 = _mm_loadu_si128(&((__m128i*)in)[0]);
	tmp2 = _mm_loadu_si128(&((__m128i*)in)[1]);
	
	tmp1 = _mm_xor_si128(tmp1, Key_Schedule[0]);
	tmp2 = _mm_xor_si128(tmp2, Key_Schedule[0]);
	
	for (j = 1; j < nr; j++){
		tmp1 = _mm_aesenc_si128(tmp1, Key_Schedule[j]);
		tmp2 = _mm_aesenc_si128(tmp2, Key_Schedule[j]);
	
	}

	tmp1 = _mm_aesenclast_si128(tmp1, Key_Schedule[j]);
	tmp2 = _mm_aesenclast_si128(tmp2, Key_Schedule[j]);
	
	_mm_storeu_si128(&((__m128i*)out)[0], tmp1);
	_mm_storeu_si128(&((__m128i*)out)[1], tmp2);
	



}


void AES_ECB_encrypt_in_three(const unsigned char *in,
	unsigned char *out,
	const unsigned char *key,
	int nr)
{

	__m128i tmp1, tmp2, tmp3;
	__m128i *Key_Schedule = (__m128i*)key;
	int i, j;

	

	tmp1 = _mm_loadu_si128(&((__m128i*)in)[0]);
	tmp2 = _mm_loadu_si128(&((__m128i*)in)[1]);
	tmp3 = _mm_loadu_si128(&((__m128i*)in)[2]);
	


	tmp1 = _mm_xor_si128(tmp1, Key_Schedule[0]);
	tmp2 = _mm_xor_si128(tmp2, Key_Schedule[0]);
	tmp3 = _mm_xor_si128(tmp3, Key_Schedule[0]);
	

	for (j = 1; j < nr; j++){
		tmp1 = _mm_aesenc_si128(tmp1, Key_Schedule[j]);
		tmp2 = _mm_aesenc_si128(tmp2, Key_Schedule[j]);
		tmp3 = _mm_aesenc_si128(tmp3, Key_Schedule[j]);
	

	}

	tmp1 = _mm_aesenclast_si128(tmp1, Key_Schedule[j]);
	tmp2 = _mm_aesenclast_si128(tmp2, Key_Schedule[j]);
	tmp3 = _mm_aesenclast_si128(tmp3, Key_Schedule[j]);
	

	_mm_storeu_si128(&((__m128i*)out)[0], tmp1);
	_mm_storeu_si128(&((__m128i*)out)[1], tmp2);
	_mm_storeu_si128(&((__m128i*)out)[2], tmp3);
	




}




void AES_ECB_encrypt_in_one(const unsigned char *in,
	unsigned char *out,
	const unsigned char *key,
	int nr)
{

	__m128i tmp1, tmp2;
	__m128i *Key_Schedule = (__m128i*)key;
	int i, j;




	tmp1 = _mm_loadu_si128(&((__m128i*)in)[0]);
	

	tmp1 = _mm_xor_si128(tmp1, Key_Schedule[0]);
	

	for (j = 1; j < nr; j++){
		tmp1 = _mm_aesenc_si128(tmp1, Key_Schedule[j]);
	

	}

	tmp1 = _mm_aesenclast_si128(tmp1, Key_Schedule[j]);
	

	_mm_storeu_si128(&((__m128i*)out)[0], tmp1);
	
}


/*****************************************************************************/
void AES_ECB_decrypt(const unsigned char *in, 
                     unsigned char *out,
                     unsigned long length,
                     const unsigned char *key,
                     int nr)
    {
    __m128i tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7,tmp8;
    __m128i *Key_Schedule = (__m128i*)key;
    int i,j;

    if (length%16)
        length = length/16 + 1;
    else length/=16;

    for(i=0; i < length/8; i++){
        tmp1 = _mm_loadu_si128 (&((__m128i*)in)[i*8+0]);
        tmp2 = _mm_loadu_si128 (&((__m128i*)in)[i*8+1]);
        tmp3 = _mm_loadu_si128 (&((__m128i*)in)[i*8+2]);
        tmp4 = _mm_loadu_si128 (&((__m128i*)in)[i*8+3]);
        tmp5 = _mm_loadu_si128 (&((__m128i*)in)[i*8+4]);
        tmp6 = _mm_loadu_si128 (&((__m128i*)in)[i*8+5]);
        tmp7 = _mm_loadu_si128 (&((__m128i*)in)[i*8+6]);
        tmp8 = _mm_loadu_si128 (&((__m128i*)in)[i*8+7]);

        tmp1 = _mm_xor_si128 (tmp1,Key_Schedule[0]);
        tmp2 = _mm_xor_si128 (tmp2,Key_Schedule[0]);
        tmp3 = _mm_xor_si128 (tmp3,Key_Schedule[0]);
        tmp4 = _mm_xor_si128 (tmp4,Key_Schedule[0]);
        tmp5 = _mm_xor_si128 (tmp5,Key_Schedule[0]);
        tmp6 = _mm_xor_si128 (tmp6,Key_Schedule[0]);
        tmp7 = _mm_xor_si128 (tmp7,Key_Schedule[0]);
        tmp8 = _mm_xor_si128 (tmp8,Key_Schedule[0]);
   
        for(j=1; j < nr; j++){
            tmp1 = _mm_aesdec_si128 (tmp1,Key_Schedule[j]);
            tmp2 = _mm_aesdec_si128 (tmp2,Key_Schedule[j]);
            tmp3 = _mm_aesdec_si128 (tmp3,Key_Schedule[j]);
            tmp4 = _mm_aesdec_si128 (tmp4,Key_Schedule[j]);
            tmp5 = _mm_aesdec_si128 (tmp5,Key_Schedule[j]);
            tmp6 = _mm_aesdec_si128 (tmp6,Key_Schedule[j]);
            tmp7 = _mm_aesdec_si128 (tmp7,Key_Schedule[j]);
            tmp8 = _mm_aesdec_si128 (tmp8,Key_Schedule[j]);
        }

        tmp1 = _mm_aesdeclast_si128 (tmp1,Key_Schedule[j]);
        tmp2 = _mm_aesdeclast_si128 (tmp2,Key_Schedule[j]);
        tmp3 = _mm_aesdeclast_si128 (tmp3,Key_Schedule[j]);
        tmp4 = _mm_aesdeclast_si128 (tmp4,Key_Schedule[j]);
        tmp5 = _mm_aesdeclast_si128 (tmp5,Key_Schedule[j]);
        tmp6 = _mm_aesdeclast_si128 (tmp6,Key_Schedule[j]);
        tmp7 = _mm_aesdeclast_si128 (tmp7,Key_Schedule[j]);
        tmp8 = _mm_aesdeclast_si128 (tmp8,Key_Schedule[j]);

        _mm_storeu_si128 (&((__m128i*)out)[i*8+0],tmp1);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+1],tmp2);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+2],tmp3);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+3],tmp4);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+4],tmp5);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+5],tmp6);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+6],tmp7);
        _mm_storeu_si128 (&((__m128i*)out)[i*8+7],tmp8);
        }

    for(j=i*8;j<length;j++){
        tmp1 = _mm_loadu_si128 (&((__m128i*)in)[j]);
        tmp1 = _mm_xor_si128 (tmp1,Key_Schedule[0]);
        for(i=1; i < nr; i++)
            tmp1 = _mm_aesdec_si128 (tmp1,Key_Schedule[i]);
        tmp1 = _mm_aesdeclast_si128 (tmp1,Key_Schedule[i]);
        _mm_storeu_si128 (&((__m128i*)out)[j],tmp1);
        }
    }
