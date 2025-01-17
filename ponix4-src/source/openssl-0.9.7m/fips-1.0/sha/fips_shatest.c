/* fips_shatest.c */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 2005.
 */
/* ====================================================================
 * Copyright (c) 2005 The OpenSSL Project.  All rights reserved.
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
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
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
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#ifndef OPENSSL_FIPS

int main(int argc, char *argv[])
{
    printf("No FIPS SHAXXX support\n");
    return(0);
}

#else

static int dgst_test(BIO *err, BIO *out, BIO *in);
static int print_dgst(BIO *err, const EVP_MD *md, BIO *out,
		unsigned char *Msg, int Msglen);
static int print_monte(BIO *err, const EVP_MD *md, BIO *out,
		unsigned char *Seed, int SeedLen);

int main(int argc, char **argv)
	{
	BIO *in = NULL, *out = NULL, *err = NULL;

	int ret = 1;

	err = BIO_new_fp(stderr, BIO_NOCLOSE);

	if (!err)
		{
		fprintf(stderr, "FATAL stderr initialization error\n");
		goto end;
		}

	if(!FIPS_mode_set(1))
		{
		ERR_print_errors(err);
		goto end;
		}

	if (argc == 1)
		in = BIO_new_fp(stdin, BIO_NOCLOSE);
	else
		in = BIO_new_file(argv[1], "r");

	if (argc < 2)
		out = BIO_new_fp(stdout, BIO_NOCLOSE);
	else
		out = BIO_new_file(argv[2], "w");

	if (!in)
		{
		BIO_printf(err, "FATAL input initialization error\n");
		goto end;
		}

	if (!out)
		{
		fprintf(stderr, "FATAL output initialization error\n");
		goto end;
		}

	if (!dgst_test(err, out, in))
		{
		fprintf(stderr, "FATAL digest file processing error\n");
		goto end;
		}
	else
		ret = 0;

	end:

	if (ret && err)
		ERR_print_errors(err);

	if (in)
		BIO_free(in);
	if (out)
		BIO_free(out);
	if (err)
		BIO_free(err);

	return ret;

	}

#define SHA_TEST_MAX_BITS	102400
#define SHA_TEST_MAXLINELEN	(((SHA_TEST_MAX_BITS >> 3) * 2) + 100)

int dgst_test(BIO *err, BIO *out, BIO *in)
	{
	const EVP_MD *md = NULL;
	char *linebuf, *olinebuf, *p, *q;
	char *keyword, *value;
	unsigned char *Msg = NULL, *Seed = NULL;
	long MsgLen = -1, Len = -1, SeedLen = -1;
	int ret = 0;
	int lnum = 0;

	olinebuf = OPENSSL_malloc(SHA_TEST_MAXLINELEN);
	linebuf = OPENSSL_malloc(SHA_TEST_MAXLINELEN);

	if (!linebuf || !olinebuf)
		goto error;


	while (BIO_gets(in, olinebuf, SHA_TEST_MAXLINELEN) > 0)
		{
		lnum++;
		strcpy(linebuf, olinebuf);
		keyword = linebuf;
		/* Skip leading space */
		while (isspace((unsigned char)*keyword))
			keyword++;

		/* Look for = sign */
		p = strchr(linebuf, '=');

		/* If no = or starts with [ (for [L=20] line) just copy */
		if (!p)
			{
			if (!BIO_puts(out, olinebuf))
				goto error;
			continue;
			}

		q = p - 1;

		/* Remove trailing space */
		while (isspace((unsigned char)*q))
			*q-- = 0;

		*p = 0;
		value = p + 1;

		/* Remove leading space from value */
		while (isspace((unsigned char)*value))
			value++;

		/* Remove trailing space from value */
		p = value + strlen(value) - 1;
		while (*p == '\n' || isspace((unsigned char)*p))
			*p-- = 0;

		if (!strcmp(keyword,"[L") && *p==']')
			{
			switch (atoi(value))
				{
				case 20: md=EVP_sha1();   break;
				case 28: md=EVP_sha224(); break;
				case 32: md=EVP_sha256(); break;
				case 48: md=EVP_sha384(); break;
				case 64: md=EVP_sha512(); break;
				default: goto parse_error;
				}
			}
		else if (!strcmp(keyword, "Len"))
			{
			if (Len != -1)
				goto parse_error;
			Len = atoi(value);
			if (Len < 0)
				goto parse_error;
			/* Only handle multiples of 8 bits */
			if (Len & 0x7)
				goto parse_error;
			if (Len > SHA_TEST_MAX_BITS)
				goto parse_error;
			MsgLen = Len >> 3;
			}

		else if (!strcmp(keyword, "Msg"))
			{
			long tmplen;
			if (strlen(value) & 1)
				*(--value) = '0';
			if (Msg)
				goto parse_error;
			Msg = string_to_hex(value, &tmplen);
			if (!Msg)
				goto parse_error;
			}
		else if (!strcmp(keyword, "Seed"))
			{
			if (strlen(value) & 1)
				*(--value) = '0';
			if (Seed)
				goto parse_error;
			Seed = string_to_hex(value, &SeedLen);
			if (!Seed)
				goto parse_error;
			}
		else if (!strcmp(keyword, "MD"))
			continue;
		else
			goto parse_error;

		BIO_puts(out, olinebuf);

		if (md && Msg && (MsgLen >= 0))
			{
			if (!print_dgst(err, md, out, Msg, MsgLen))
				goto error;
			OPENSSL_free(Msg);
			Msg = NULL;
			MsgLen = -1;
			Len = -1;
			}
		else if (md && Seed && (SeedLen > 0))
			{
			if (!print_monte(err, md, out, Seed, SeedLen))
				goto error;
			OPENSSL_free(Seed);
			Seed = NULL;
			SeedLen = -1;
			}
	

		}


	ret = 1;


	error:

	if (olinebuf)
		OPENSSL_free(olinebuf);
	if (linebuf)
		OPENSSL_free(linebuf);
	if (Msg)
		OPENSSL_free(Msg);
	if (Seed)
		OPENSSL_free(Seed);

	return ret;

	parse_error:

	BIO_printf(err, "FATAL parse error processing line %d\n", lnum);

	goto error;

	}

static int print_dgst(BIO *err, const EVP_MD *emd, BIO *out,
		unsigned char *Msg, int Msglen)
	{
	int i, mdlen;
	unsigned char md[EVP_MAX_MD_SIZE];
	if (!EVP_Digest(Msg, Msglen, md, (unsigned int *)&mdlen, emd, NULL))
		{
		BIO_puts(err, "Error calculating HASH\n");
		return 0;
		}
	BIO_puts(out, "MD = ");
	for (i = 0; i < mdlen; i++)
		BIO_printf(out, "%02x", md[i]);
	BIO_puts(out, "\n");
	return 1;
	}

static int print_monte(BIO *err, const EVP_MD *md, BIO *out,
		unsigned char *Seed, int SeedLen)
	{
	unsigned int i, j, k;
	int ret = 0;
	EVP_MD_CTX ctx;
	unsigned char *m1, *m2, *m3, *p;
	unsigned int mlen, m1len, m2len, m3len;

	EVP_MD_CTX_init(&ctx);

	if (SeedLen > EVP_MAX_MD_SIZE)
		mlen = SeedLen;
	else
		mlen = EVP_MAX_MD_SIZE;

	m1 = OPENSSL_malloc(mlen);
	m2 = OPENSSL_malloc(mlen);
	m3 = OPENSSL_malloc(mlen);

	if (!m1 || !m2 || !m3)
		goto mc_error;

	m1len = m2len = m3len = SeedLen;
	memcpy(m1, Seed, SeedLen);
	memcpy(m2, Seed, SeedLen);
	memcpy(m3, Seed, SeedLen);

	BIO_puts(out, "\n");

	for (j = 0; j < 100; j++)
		{
		for (i = 0; i < 1000; i++)
			{
			EVP_DigestInit_ex(&ctx, md, NULL);
			EVP_DigestUpdate(&ctx, m1, m1len);
			EVP_DigestUpdate(&ctx, m2, m2len);
			EVP_DigestUpdate(&ctx, m3, m3len);
			p = m1;
			m1 = m2;
			m1len = m2len;
			m2 = m3;
			m2len = m3len;
			m3 = p;
			EVP_DigestFinal_ex(&ctx, m3, &m3len);
			}
		BIO_printf(out, "COUNT = %d\n", j);
		BIO_puts(out, "MD = ");
		for (k = 0; k < m3len; k++)
			BIO_printf(out, "%02x", m3[k]);
		BIO_puts(out, "\n\n");
		memcpy(m1, m3, m3len);
		memcpy(m2, m3, m3len);
		m1len = m2len = m3len;
		}

	ret = 1;

	mc_error:
	if (m1)
		OPENSSL_free(m1);
	if (m2)
		OPENSSL_free(m2);
	if (m3)
		OPENSSL_free(m3);

	EVP_MD_CTX_cleanup(&ctx);

	return ret;
	}

#endif
