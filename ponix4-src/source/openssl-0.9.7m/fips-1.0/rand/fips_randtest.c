/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
/* ====================================================================
 * Copyright (c) 2003 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <openssl/rand.h>
#include <openssl/fips_rand.h>
#include <openssl/err.h>

#include "e_os.h"

#ifndef OPENSSL_FIPS
int main(int argc, char *argv[])
{
    printf("No FIPS RAND support\n");
    return(0);
}

#else

/* some FIPS 140-1 random number test */
/* some simple tests */

static DES_cblock prng_key1={0x21,0x58,0x47,0xb7,0xc2,0x97,0x5a,0x8e};
static DES_cblock prng_key2={0x61,0x23,0x05,0x96,0x18,0x91,0x86,0xac};
static unsigned char prng_seed[8]={0x6b,0xa3,0x4f,0x07,0xe4,0x2a,0xb0,0xc};

typedef struct
    {
    DES_cblock keys[2];
    const unsigned char time[8];
    const unsigned char seed[8];
    const unsigned char block1[8];
    const unsigned char block100[8];
    } PRNGtest;

/* FIXME: these test vectors are made up! */
static PRNGtest t1=
    {
    { { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07 },
      { 0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f },
    },
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    { 0x33,0xc3,0xdf,0xfe,0x60,0x60,0x49,0x9e },
    { 0xcd,0x2b,0x41,0xaf,0x80,0x51,0x37,0xd8 }
    };
static PRNGtest t2=
    {
    { { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
      { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff } },
    { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
    { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
    { 0x65,0xf1,0xa4,0x07,0x42,0x38,0xd5,0x25 },
    { 0xbb,0x75,0x84,0x20,0x7a,0x44,0xf0,0xa0 }
    };

static void dump(const unsigned char *b,int n)
    {
    while(n-- > 0)
	{
	printf(" %02x",*b++);
	}
    }

static void compare(const unsigned char *result,const unsigned char *expected,
		    int n)
    {
    int i;

    for(i=0 ; i < n ; ++i)
	if(result[i] != expected[i])
	    {
	    puts("Random test failed, got:");
	    dump(result,8);
	    puts("\n               expected:");
	    dump(expected,8);
	    putchar('\n');
	    EXIT(1);
	    }
    }

static void run_test(const PRNGtest *t)
    {
    unsigned char buf[8];
    int n;

    FIPS_set_prng_key(t->keys[0],t->keys[1]);
    FIPS_test_mode(1,t->time);
    RAND_seed(t->seed,sizeof t->seed);

    if(RAND_bytes(buf,8) <= 0)
	{
	ERR_print_errors_fp(stderr);
	EXIT(2);
	}
    compare(buf,t->block1,8);
    for(n=0 ; n < 99 ; ++n)
	if(RAND_bytes(buf,8) <= 0)
	    {
	    ERR_print_errors_fp(stderr);
	    EXIT(2);
	    }
    compare(buf,t->block100,8);
    FIPS_test_mode(0,NULL);
    }

int main()
	{
	unsigned char buf[2500];
	int i,j,k,s,sign,nsign,err=0;
	unsigned long n1;
	unsigned long n2[16];
	unsigned long runs[2][34];
	/*double d; */
	long d;

	RAND_set_rand_method(FIPS_rand_method());

	run_test(&t1);
	run_test(&t2);

	FIPS_set_prng_key(prng_key1,prng_key2);
	RAND_seed(prng_seed,sizeof prng_seed);

	i = RAND_pseudo_bytes(buf,2500);
	if (i <= 0)
		{
		printf ("init failed, the rand method is not properly installed\n");
		err++;
		goto err;
		}

	n1=0;
	for (i=0; i<16; i++) n2[i]=0;
	for (i=0; i<34; i++) runs[0][i]=runs[1][i]=0;

	/* test 1 and 2 */
	sign=0;
	nsign=0;
	for (i=0; i<2500; i++)
		{
		j=buf[i];

		n2[j&0x0f]++;
		n2[(j>>4)&0x0f]++;

		for (k=0; k<8; k++)
			{
			s=(j&0x01);
			if (s == sign)
				nsign++;
			else
				{
				if (nsign > 34) nsign=34;
				if (nsign != 0)
					{
					runs[sign][nsign-1]++;
					if (nsign > 6)
						runs[sign][5]++;
					}
				sign=s;
				nsign=1;
				}

			if (s) n1++;
			j>>=1;
			}
		}
		if (nsign > 34) nsign=34;
		if (nsign != 0) runs[sign][nsign-1]++;

	/* test 1 */
	if (!((9654 < n1) && (n1 < 10346)))
		{
		printf("test 1 failed, X=%lu\n",n1);
		err++;
		}
	printf("test 1 done\n");

	/* test 2 */
#ifdef undef
	d=0;
	for (i=0; i<16; i++)
		d+=n2[i]*n2[i];
	d=d*16.0/5000.0-5000.0;
	if (!((1.03 < d) && (d < 57.4)))
		{
		printf("test 2 failed, X=%.2f\n",d);
		err++;
		}
#endif
	d=0;
	for (i=0; i<16; i++)
		d+=n2[i]*n2[i];
	d=(d*8)/25-500000;
	if (!((103 < d) && (d < 5740)))
		{
		printf("test 2 failed, X=%ld.%02ld\n",d/100L,d%100L);
		err++;
		}
	printf("test 2 done\n");

	/* test 3 */
	for (i=0; i<2; i++)
		{
		if (!((2267 < runs[i][0]) && (runs[i][0] < 2733)))
			{
			printf("test 3 failed, bit=%d run=%d num=%lu\n",
				i,1,runs[i][0]);
			err++;
			}
		if (!((1079 < runs[i][1]) && (runs[i][1] < 1421)))
			{
			printf("test 3 failed, bit=%d run=%d num=%lu\n",
				i,2,runs[i][1]);
			err++;
			}
		if (!(( 502 < runs[i][2]) && (runs[i][2] <  748)))
			{
			printf("test 3 failed, bit=%d run=%d num=%lu\n",
				i,3,runs[i][2]);
			err++;
			}
		if (!(( 223 < runs[i][3]) && (runs[i][3] <  402)))
			{
			printf("test 3 failed, bit=%d run=%d num=%lu\n",
				i,4,runs[i][3]);
			err++;
			}
		if (!((  90 < runs[i][4]) && (runs[i][4] <  223)))
			{
			printf("test 3 failed, bit=%d run=%d num=%lu\n",
				i,5,runs[i][4]);
			err++;
			}
		if (!((  90 < runs[i][5]) && (runs[i][5] <  223)))
			{
			printf("test 3 failed, bit=%d run=%d num=%lu\n",
				i,6,runs[i][5]);
			err++;
			}
		}
	printf("test 3 done\n");
	
	/* test 4 */
	if (runs[0][33] != 0)
		{
		printf("test 4 failed, bit=%d run=%d num=%lu\n",
			0,34,runs[0][33]);
		err++;
		}
	if (runs[1][33] != 0)
		{
		printf("test 4 failed, bit=%d run=%d num=%lu\n",
			1,34,runs[1][33]);
		err++;
		}
	printf("test 4 done\n");
 err:
	err=((err)?1:0);
	EXIT(err);
	return(err);
	}

#endif
