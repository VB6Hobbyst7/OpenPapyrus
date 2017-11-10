/* Copyright (C) 2009, 2010 Simon Josefsson
 * Copyright (C) 2006, 2007 The Written Word, Inc.  All rights reserved.
 * Copyright (C) 2010 Daniel Stenberg
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 *   Redistributions of source code must retain the above
 *   copyright notice, this list of conditions and the
 *   following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials
 *   provided with the distribution.
 *
 *   Neither the name of the copyright holder nor the names
 *   of any other contributors may be used to endorse or
 *   promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#ifndef LIBSSH2_CRYPTO_H
#define LIBSSH2_CRYPTO_H

#ifdef LIBSSH2_OPENSSL
	//
	//#include "openssl.h"
	//
	#include <openssl/opensslconf.h>
	#include <openssl/sha.h>
	#include <openssl/rsa.h>
	#include <openssl/engine.h>
	#ifndef OPENSSL_NO_DSA
		#include <openssl/dsa.h>
	#endif
	#ifndef OPENSSL_NO_MD5
		#include <openssl/md5.h>
	#endif
	#include <openssl/evp.h>
	#include <openssl/hmac.h>
	#include <openssl/bn.h>
	#include <openssl/pem.h>
	#include <openssl/rand.h>

	#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
		#define HAVE_OPAQUE_STRUCTS 1
	#endif
	#ifdef OPENSSL_NO_RSA
		#define LIBSSH2_RSA 0
	#else
		#define LIBSSH2_RSA 1
	#endif
	#ifdef OPENSSL_NO_DSA
		#define LIBSSH2_DSA 0
	#else
		#define LIBSSH2_DSA 1
	#endif
	#ifdef OPENSSL_NO_MD5
		#define LIBSSH2_MD5 0
	#else
		#define LIBSSH2_MD5 1
	#endif
	#ifdef OPENSSL_NO_RIPEMD
		#define LIBSSH2_HMAC_RIPEMD 0
	#else
		#define LIBSSH2_HMAC_RIPEMD 1
	#endif
	#define LIBSSH2_HMAC_SHA256 1
	#define LIBSSH2_HMAC_SHA512 1
	#if OPENSSL_VERSION_NUMBER >= 0x00907000L && !defined(OPENSSL_NO_AES)
		#define LIBSSH2_AES_CTR 1
		#define LIBSSH2_AES 1
	#else
		#define LIBSSH2_AES_CTR 0
		#define LIBSSH2_AES 0
	#endif
	#ifdef OPENSSL_NO_BF
		#define LIBSSH2_BLOWFISH 0
	#else
		#define LIBSSH2_BLOWFISH 1
	#endif
	#ifdef OPENSSL_NO_RC4
		#define LIBSSH2_RC4 0
	#else
		#define LIBSSH2_RC4 1
	#endif
	#ifdef OPENSSL_NO_CAST
		#define LIBSSH2_CAST 0
	#else
		#define LIBSSH2_CAST 1
	#endif
	#ifdef OPENSSL_NO_DES
		#define LIBSSH2_3DES 0
	#else
		#define LIBSSH2_3DES 1
	#endif
	#define _libssh2_random(buf, len) RAND_bytes ((buf), (len))
	#define libssh2_prepare_iovec(vec, len)  /* Empty. */
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_sha1_ctx EVP_MD_CTX *
	#else
		#define libssh2_sha1_ctx EVP_MD_CTX
	#endif
	/* returns 0 in case of failure */
	int _libssh2_sha1_init(libssh2_sha1_ctx *ctx);
	#define libssh2_sha1_init(x) _libssh2_sha1_init(x)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_sha1_update(ctx, data, len) EVP_DigestUpdate(ctx, data, len)
		#define libssh2_sha1_final(ctx, out) do { EVP_DigestFinal(ctx, out, NULL); EVP_MD_CTX_free(ctx); } while(0)
	#else
		#define libssh2_sha1_update(ctx, data, len) EVP_DigestUpdate(&(ctx), data, len)
		#define libssh2_sha1_final(ctx, out) EVP_DigestFinal(&(ctx), out, NULL)
	#endif
	int _libssh2_sha1(const uchar *message, ulong len, uchar *out);
	#define libssh2_sha1(x,y,z) _libssh2_sha1(x,y,z)

	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_sha256_ctx EVP_MD_CTX *
	#else
		#define libssh2_sha256_ctx EVP_MD_CTX
	#endif

	/* returns 0 in case of failure */
	int _libssh2_sha256_init(libssh2_sha256_ctx *ctx);
	#define libssh2_sha256_init(x) _libssh2_sha256_init(x)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_sha256_update(ctx, data, len) EVP_DigestUpdate(ctx, data, len)
		#define libssh2_sha256_final(ctx, out) do { EVP_DigestFinal(ctx, out, NULL); EVP_MD_CTX_free(ctx); } while(0)
	#else
		#define libssh2_sha256_update(ctx, data, len) EVP_DigestUpdate(&(ctx), data, len)
		#define libssh2_sha256_final(ctx, out) EVP_DigestFinal(&(ctx), out, NULL)
	#endif
	int _libssh2_sha256(const uchar *message, ulong len, uchar *out);
	#define libssh2_sha256(x,y,z) _libssh2_sha256(x,y,z)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_md5_ctx EVP_MD_CTX *
	#else
		#define libssh2_md5_ctx EVP_MD_CTX
	#endif
	/* returns 0 in case of failure */
	int _libssh2_md5_init(libssh2_md5_ctx *ctx);
	#define libssh2_md5_init(x) _libssh2_md5_init(x)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_md5_update(ctx, data, len) EVP_DigestUpdate(ctx, data, len)
		#define libssh2_md5_final(ctx, out) do { EVP_DigestFinal(ctx, out, NULL); EVP_MD_CTX_free(ctx); } while(0)
	#else
		#define libssh2_md5_update(ctx, data, len) EVP_DigestUpdate(&(ctx), data, len)
		#define libssh2_md5_final(ctx, out) EVP_DigestFinal(&(ctx), out, NULL)
	#endif
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_hmac_ctx HMAC_CTX *
		#define libssh2_hmac_ctx_init(ctx) ctx = HMAC_CTX_new()
		#define libssh2_hmac_sha1_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_sha1(), NULL)
		#define libssh2_hmac_md5_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_md5(), NULL)
		#define libssh2_hmac_ripemd160_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_ripemd160(), NULL)
		#define libssh2_hmac_sha256_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_sha256(), NULL)
		#define libssh2_hmac_sha512_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_sha512(), NULL)
		#define libssh2_hmac_update(ctx, data, datalen) HMAC_Update(ctx, data, datalen)
		#define libssh2_hmac_final(ctx, data) HMAC_Final(ctx, data, NULL)
		#define libssh2_hmac_cleanup(ctx) HMAC_CTX_free(*(ctx))
	#else
		#define libssh2_hmac_ctx HMAC_CTX
		#define libssh2_hmac_ctx_init(ctx) HMAC_CTX_init(&ctx)
		#define libssh2_hmac_sha1_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_sha1(), NULL)
		#define libssh2_hmac_md5_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_md5(), NULL)
		#define libssh2_hmac_ripemd160_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_ripemd160(), NULL)
		#define libssh2_hmac_sha256_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_sha256(), NULL)
		#define libssh2_hmac_sha512_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_sha512(), NULL)
		#define libssh2_hmac_update(ctx, data, datalen) HMAC_Update(&(ctx), data, datalen)
		#define libssh2_hmac_final(ctx, data) HMAC_Final(&(ctx), data, NULL)
		#define libssh2_hmac_cleanup(ctx) HMAC_cleanup(ctx)
	#endif
	#define libssh2_crypto_init() OpenSSL_add_all_algorithms(); ENGINE_load_builtin_engines(); ENGINE_register_all_complete()
	#define libssh2_crypto_exit()
	#define libssh2_rsa_ctx RSA
	#define _libssh2_rsa_free(rsactx) RSA_free(rsactx)
	#define libssh2_dsa_ctx DSA
	#define _libssh2_dsa_free(dsactx) DSA_free(dsactx)
	#define _libssh2_cipher_type(name) const EVP_CIPHER *(*name)(void)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define _libssh2_cipher_ctx EVP_CIPHER_CTX *
	#else
		#define _libssh2_cipher_ctx EVP_CIPHER_CTX
	#endif
	#define _libssh2_cipher_aes256 EVP_aes_256_cbc
	#define _libssh2_cipher_aes192 EVP_aes_192_cbc
	#define _libssh2_cipher_aes128 EVP_aes_128_cbc
	#ifdef HAVE_EVP_AES_128_CTR
		#define _libssh2_cipher_aes128ctr EVP_aes_128_ctr
		#define _libssh2_cipher_aes192ctr EVP_aes_192_ctr
		#define _libssh2_cipher_aes256ctr EVP_aes_256_ctr
	#else
		#define _libssh2_cipher_aes128ctr _libssh2_EVP_aes_128_ctr
		#define _libssh2_cipher_aes192ctr _libssh2_EVP_aes_192_ctr
		#define _libssh2_cipher_aes256ctr _libssh2_EVP_aes_256_ctr
	#endif
	#define _libssh2_cipher_blowfish EVP_bf_cbc
	#define _libssh2_cipher_arcfour EVP_rc4
	#define _libssh2_cipher_cast5 EVP_cast5_cbc
	#define _libssh2_cipher_3des EVP_des_ede3_cbc
	#ifdef HAVE_OPAQUE_STRUCTS
		#define _libssh2_cipher_dtor(ctx) EVP_CIPHER_CTX_reset(*(ctx))
	#else
		#define _libssh2_cipher_dtor(ctx) EVP_CIPHER_CTX_cleanup(ctx)
	#endif
	#define _libssh2_bn BIGNUM
	#define _libssh2_bn_ctx BN_CTX
	#define _libssh2_bn_ctx_new() BN_CTX_new()
	#define _libssh2_bn_ctx_free(bnctx) BN_CTX_free(bnctx)
	#define _libssh2_bn_init() BN_new()
	#define _libssh2_bn_init_from_bin() _libssh2_bn_init()
	#define _libssh2_bn_rand(bn, bits, top, bottom) BN_rand(bn, bits, top, bottom)
	#define _libssh2_bn_mod_exp(r, a, p, m, ctx) BN_mod_exp(r, a, p, m, ctx)
	#define _libssh2_bn_set_word(bn, val) BN_set_word(bn, val)
	#define _libssh2_bn_from_bin(bn, len, val) BN_bin2bn(val, len, bn)
	#define _libssh2_bn_to_bin(bn, val) BN_bn2bin(bn, val)
	#define _libssh2_bn_bytes(bn) BN_num_bytes(bn)
	#define _libssh2_bn_bits(bn) BN_num_bits(bn)
	#define _libssh2_bn_free(bn) BN_clear_free(bn)

	const EVP_CIPHER *_libssh2_EVP_aes_128_ctr(void);
	const EVP_CIPHER *_libssh2_EVP_aes_192_ctr(void);
	const EVP_CIPHER *_libssh2_EVP_aes_256_ctr(void);
	//
#endif
#ifdef LIBSSH2_LIBGCRYPT
	#include "libgcrypt.h"
#endif
#ifdef LIBSSH2_WINCNG
	#include "wincng.h"
#endif
#ifdef LIBSSH2_OS400QC3
	#include "os400qc3.h"
#endif
#ifdef LIBSSH2_MBEDTLS
	#include "mbedtls.h"
#endif

int _libssh2_rsa_new(libssh2_rsa_ctx ** rsa, const uchar * edata, ulong elen,
    const uchar * ndata, ulong nlen, const uchar * ddata, ulong dlen,
    const uchar * pdata, ulong plen, const uchar * qdata, ulong qlen,
    const uchar * e1data, ulong e1len, const uchar * e2data, ulong e2len, const uchar * coeffdata, ulong coefflen);
int _libssh2_rsa_new_private(libssh2_rsa_ctx ** rsa, LIBSSH2_SESSION * session, const char * filename, const uchar * passphrase);
int _libssh2_rsa_sha1_verify(libssh2_rsa_ctx * rsa, const uchar * sig, ulong sig_len, const uchar * m, ulong m_len);
int _libssh2_rsa_sha1_sign(LIBSSH2_SESSION * session, libssh2_rsa_ctx * rsactx, const uchar * hash, size_t hash_len, uchar ** signature, size_t * signature_len);
int _libssh2_rsa_new_private_frommemory(libssh2_rsa_ctx ** rsa, LIBSSH2_SESSION * session, const char * filedata, size_t filedata_len, const uchar * passphrase);
#if LIBSSH2_DSA
	int _libssh2_dsa_new(libssh2_dsa_ctx ** dsa, const uchar * pdata, ulong plen, const uchar * qdata, ulong qlen,
		const uchar * gdata, ulong glen, const uchar * ydata, ulong ylen, const uchar * x, ulong x_len);
	int _libssh2_dsa_new_private(libssh2_dsa_ctx ** dsa, LIBSSH2_SESSION * session, const char * filename, const uchar * passphrase);
	int _libssh2_dsa_sha1_verify(libssh2_dsa_ctx * dsactx, const uchar * sig, const uchar * m, ulong m_len);
	int _libssh2_dsa_sha1_sign(libssh2_dsa_ctx * dsactx, const uchar * hash, ulong hash_len, uchar * sig);
	int _libssh2_dsa_new_private_frommemory(libssh2_dsa_ctx ** dsa, LIBSSH2_SESSION * session, const char * filedata, size_t filedata_len, const uchar * passphrase);
#endif
int _libssh2_cipher_init(_libssh2_cipher_ctx * h, _libssh2_cipher_type(algo), uchar *iv, uchar *secret, int encrypt);
int _libssh2_cipher_crypt(_libssh2_cipher_ctx * ctx, _libssh2_cipher_type(algo), int encrypt, uchar *block, size_t blocksize);
int _libssh2_pub_priv_keyfile(LIBSSH2_SESSION * session, uchar ** method, size_t * method_len,
    uchar ** pubkeydata, size_t * pubkeydata_len, const char * privatekey, const char * passphrase);
int _libssh2_pub_priv_keyfilememory(LIBSSH2_SESSION * session, uchar ** method, size_t * method_len,
    uchar ** pubkeydata, size_t * pubkeydata_len, const char * privatekeydata, size_t privatekeydata_len, const char * passphrase);
void _libssh2_init_aes_ctr(void);

#endif