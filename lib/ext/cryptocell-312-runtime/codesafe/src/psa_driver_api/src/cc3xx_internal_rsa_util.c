/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cc3xx_internal_rsa_util.h"
#include "cc3xx_internal_asn1_util.h"

#include "cc_common.h"
#include "cc_rnd_error.h"
#include "cc_rsa_build.h"
#include "cc_rsa_error.h"
#include "cc_rsa_types.h"
#include "psa/crypto.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf printf
#define mbedtls_calloc calloc
#define mbedtls_free free
#endif

CCError_t cc3xx_rsa_psa_pub_to_cc_pub(const uint8_t *psa_pub_key_buffer,
                                      size_t psa_pub_key_buffer_size,
                                      CCRsaUserPubKey_t *UserPubKey_ptr)

{
    uint8_t *pub_key_buffer_start_pnt = (uint8_t *)psa_pub_key_buffer;
    uint8_t **pub_key_buffer_start = &pub_key_buffer_start_pnt;
    uint8_t *pub_key_buffer_end =
        (uint8_t *)psa_pub_key_buffer + psa_pub_key_buffer_size;
    uint8_t *n_ptr;
    size_t n_len;
    uint8_t *e_ptr;
    size_t e_len;
    size_t len;
    int ret;

    /* Move the pointer after the sequence */
    ret = cc3xx_asn1_get_tag(pub_key_buffer_start, pub_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_CONSTRUCTED |
                                 CC3XX_TAG_ASN1_SEQUENCE);
    if (ret < 0) {
        return CC_FAIL;
    }

    /* Get the modulus n */
    ret = cc3xx_asn1_get_tag(pub_key_buffer_start, pub_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    n_ptr = *pub_key_buffer_start;
    n_len = len;

    *pub_key_buffer_start += len;

    /* Get the exponent e */
    ret = cc3xx_asn1_get_tag(pub_key_buffer_start, pub_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    e_ptr = *pub_key_buffer_start;
    e_len = len;
    *pub_key_buffer_start += len;

    return CC_RsaPubKeyBuild(UserPubKey_ptr, e_ptr, e_len, n_ptr, n_len);
}

CCError_t cc3xx_rsa_psa_priv_to_cc_pub(const uint8_t *psa_pub_key_buffer,
                                       size_t psa_pub_key_buffer_size,
                                       CCRsaUserPubKey_t *UserPubKey_ptr)

{
    uint8_t *pub_key_buffer_start_pnt = (uint8_t *)psa_pub_key_buffer;
    uint8_t **pub_key_buffer_start = &pub_key_buffer_start_pnt;
    uint8_t *pub_key_buffer_end =
        (uint8_t *)psa_pub_key_buffer + psa_pub_key_buffer_size;
    uint8_t *n_ptr;
    size_t n_len;
    uint8_t *e_ptr;
    size_t e_len;
    size_t len;
    int dummy;
    int ret;

    /* Move the pointer after the sequence */
    ret = cc3xx_asn1_get_tag(pub_key_buffer_start, pub_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_CONSTRUCTED |
                                 CC3XX_TAG_ASN1_SEQUENCE);
    if (ret < 0) {
        return CC_FAIL;
    }

    /* Move the pointer after the version */
    ret = cc3xx_asn1_get_int(pub_key_buffer_start, pub_key_buffer_end, &dummy);
    if (ret < 0) {
        return CC_FAIL;
    }

    /* Get the modulus n */
    ret = cc3xx_asn1_get_tag(pub_key_buffer_start, pub_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    n_ptr = *pub_key_buffer_start;
    n_len = len;
    *pub_key_buffer_start += len;

    /* Get the exponent e */
    ret = cc3xx_asn1_get_tag(pub_key_buffer_start, pub_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    e_ptr = *pub_key_buffer_start;
    e_len = len;
    *pub_key_buffer_start += len;

    return CC_RsaPubKeyBuild(UserPubKey_ptr, e_ptr, e_len, n_ptr, n_len);
}

CCError_t cc3xx_rsa_psa_priv_to_cc_priv(const uint8_t *psa_priv_key_buffer,
                                        size_t psa_priv_key_buffer_size,
                                        CCRsaUserPrivKey_t *UserPrivKey_ptr)

{
    uint8_t *p_ptr;
    size_t p_len;
    uint8_t *q_ptr;
    size_t q_len;
    uint8_t *dP_ptr;
    size_t dP_len;
    uint8_t *dQ_ptr;
    size_t dQ_len;
    uint8_t *qInv_ptr;
    size_t qInv_len;
    int ret;
    size_t len;
    int dummy;
    CCError_t cc_err = CC_SUCCESS;

    uint8_t *priv_key_buffer_start_pnt = (uint8_t *)psa_priv_key_buffer;
    uint8_t **priv_key_buffer_start = &priv_key_buffer_start_pnt;
    uint8_t *priv_key_buffer_end =
        (uint8_t *)psa_priv_key_buffer + psa_priv_key_buffer_size;

    /* Move the pointer after the sequence */
    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_CONSTRUCTED |
                                 CC3XX_TAG_ASN1_SEQUENCE);
    if (ret < 0) {
        return CC_FAIL;
    }

    /* Move the pointer after the version */
    ret =
        cc3xx_asn1_get_int(priv_key_buffer_start, priv_key_buffer_end, &dummy);
    if (ret < 0) {
        return CC_FAIL;
    }

    /* Move the pointer after the modulus n */
    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }
    *priv_key_buffer_start += len;

    /* Move the pointer after the publicExponent e*/
    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }
    *priv_key_buffer_start += len;

    /* Move the pointer after the privateExponent d */
    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }
    *priv_key_buffer_start += len;

    /* Get the P */
    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    p_ptr = *priv_key_buffer_start;
    p_len = len;

    *priv_key_buffer_start += len;

    /* Get the Q */
    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    q_ptr = *priv_key_buffer_start;
    q_len = len;

    *priv_key_buffer_start += len;

    /* Get the dP */
    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    dP_ptr = *priv_key_buffer_start;
    dP_len = len;

    *priv_key_buffer_start += len;

    /* Get the dQ */
    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    dQ_ptr = *priv_key_buffer_start;
    dQ_len = len;

    *priv_key_buffer_start += len;

    /* Get the dInv */
    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    qInv_ptr = *priv_key_buffer_start;
    qInv_len = len;

    *priv_key_buffer_start += len;

    cc_err = CC_RsaPrivKeyCrtBuild(UserPrivKey_ptr, p_ptr, p_len, q_ptr, q_len,
                                   dP_ptr, dP_len, dQ_ptr, dQ_len, qInv_ptr,
                                   qInv_len);
    return cc_err;
}

CCError_t cc3xx_rsa_psa_priv_to_psa_publ(uint8_t *priv_key_buffer,
                                         size_t priv_key_buffer_size,
                                         uint8_t *publ_key_buffer,
                                         size_t publ_key_buffer_size)
{

    uint8_t *priv_key_buffer_start_pnt = priv_key_buffer;
    uint8_t **priv_key_buffer_start = &priv_key_buffer_start_pnt;
    uint8_t *priv_key_buffer_end = priv_key_buffer + priv_key_buffer_size;
    uint8_t *n_pnt;
    size_t n_len;
    uint8_t *e_pnt;
    size_t e_len;

    uint8_t *pub_key_buffer_end_pnt = publ_key_buffer + publ_key_buffer_size;
    uint8_t **pub_key_buffer_end = &pub_key_buffer_end_pnt;

    CCError_t cc_err = CC_OK;
    int bytes_written;
    size_t buffer_used;
    size_t len;
    int ret;
    int dummy;

    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_CONSTRUCTED |
                                 CC3XX_TAG_ASN1_SEQUENCE);
    if (ret < 0) {
        return CC_FAIL;
    }

    ret =
        cc3xx_asn1_get_int(priv_key_buffer_start, priv_key_buffer_end, &dummy);
    if (ret < 0) {
        return CC_FAIL;
    }

    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    n_pnt = *priv_key_buffer_start;
    n_len = len;

    *priv_key_buffer_start += len;

    ret = cc3xx_asn1_get_tag(priv_key_buffer_start, priv_key_buffer_end, &len,
                             CC3XX_TAG_ASN1_INTEGER);
    if (ret < 0) {
        return CC_FAIL;
    }

    e_pnt = *priv_key_buffer_start;
    e_len = len;

    bytes_written = cc3xx_asn1_write_big_integer(pub_key_buffer_end,
                                                 publ_key_buffer, e_pnt, e_len);
    buffer_used = bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    bytes_written = cc3xx_asn1_write_big_integer(pub_key_buffer_end,
                                                 publ_key_buffer, n_pnt, n_len);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    bytes_written =
        cc3xx_asn1_write_len(pub_key_buffer_end, publ_key_buffer, buffer_used);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    bytes_written = cc3xx_asn1_write_tag(pub_key_buffer_end, publ_key_buffer,
                                         CC3XX_TAG_ASN1_SEQUENCE |
                                             CC3XX_TAG_ASN1_CONSTRUCTED);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    /* The asn1 functions write to the end of the buffer.
     * Move the data to the beginning and erase remaining data
     * at the original location. */

    if (2 * buffer_used <= publ_key_buffer_size) {
        CC_PalMemMove(publ_key_buffer,
                      publ_key_buffer + publ_key_buffer_size - buffer_used,
                      buffer_used);
        CC_PalMemSetZero(publ_key_buffer + publ_key_buffer_size - buffer_used,
                         buffer_used);
    } else if ((size_t)buffer_used < publ_key_buffer_size) {
        CC_PalMemMove(publ_key_buffer,
                      publ_key_buffer + publ_key_buffer_size - buffer_used,
                      buffer_used);
        CC_PalMemSetZero(publ_key_buffer + buffer_used,
                         publ_key_buffer_size - buffer_used);
    }

End:
    return cc_err;
}

CCError_t cc3xx_rsa_save_der_priv_key(uint8_t *key_buffer,
                                      size_t key_buffer_size, uint32_t *n,
                                      uint32_t *e, uint32_t *d, uint32_t *p,
                                      uint32_t *q, uint32_t *dP, uint32_t *dQ,
                                      uint32_t *qInv, size_t d_size_bytes)
{

    uint8_t *key_buffer_end_pnt = key_buffer + key_buffer_size;
    uint8_t **key_buffer_end = &key_buffer_end_pnt;
    CCError_t cc_err = CC_OK;
    uint8_t *temp_buff;
    int bytes_written;
    size_t buffer_used;

    temp_buff = (uint8_t *)mbedtls_calloc(1, d_size_bytes);
    if (temp_buff == NULL) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    cc_err = CC_CommonConvertLswMswWordsToMsbLsbBytes(temp_buff, d_size_bytes,
                                                      qInv, d_size_bytes / 2);
    if (cc_err != CC_OK) {
        goto End;
    }

    bytes_written = cc3xx_asn1_write_big_integer(key_buffer_end, key_buffer,
                                                 temp_buff, d_size_bytes / 2);
    buffer_used = bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    cc_err = CC_CommonConvertLswMswWordsToMsbLsbBytes(temp_buff, d_size_bytes,
                                                      dQ, d_size_bytes / 2);
    if (cc_err != CC_OK) {
        goto End;
    }

    bytes_written = cc3xx_asn1_write_big_integer(key_buffer_end, key_buffer,
                                                 temp_buff, d_size_bytes / 2);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    cc_err = CC_CommonConvertLswMswWordsToMsbLsbBytes(temp_buff, d_size_bytes,
                                                      dP, d_size_bytes / 2);
    if (cc_err != CC_OK) {
        goto End;
    }

    bytes_written = cc3xx_asn1_write_big_integer(key_buffer_end, key_buffer,
                                                 temp_buff, d_size_bytes / 2);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    cc_err = CC_CommonConvertLswMswWordsToMsbLsbBytes(temp_buff, d_size_bytes,
                                                      q, d_size_bytes / 2);
    if (cc_err != CC_OK) {
        goto End;
    }

    bytes_written = cc3xx_asn1_write_big_integer(key_buffer_end, key_buffer,
                                                 temp_buff, d_size_bytes / 2);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    cc_err = CC_CommonConvertLswMswWordsToMsbLsbBytes(temp_buff, d_size_bytes,
                                                      p, d_size_bytes / 2);
    if (cc_err != CC_OK) {
        goto End;
    }

    bytes_written = cc3xx_asn1_write_big_integer(key_buffer_end, key_buffer,
                                                 temp_buff, d_size_bytes / 2);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    cc_err = CC_CommonConvertLswMswWordsToMsbLsbBytes(temp_buff, d_size_bytes,
                                                      d, d_size_bytes);
    if (cc_err != CC_OK) {
        goto End;
    }

    bytes_written = cc3xx_asn1_write_big_integer(key_buffer_end, key_buffer,
                                                 temp_buff, d_size_bytes);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    CC_PalMemCopy(temp_buff, (uint8_t *)e, 3);

    bytes_written =
        cc3xx_asn1_write_big_integer(key_buffer_end, key_buffer, temp_buff, 3);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    cc_err = CC_CommonConvertLswMswWordsToMsbLsbBytes(temp_buff, d_size_bytes,
                                                      n, d_size_bytes);
    if (cc_err != CC_OK) {
        goto End;
    }

    bytes_written = cc3xx_asn1_write_big_integer(key_buffer_end, key_buffer,
                                                 temp_buff, d_size_bytes);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    bytes_written = cc3xx_asn1_write_int(key_buffer_end, key_buffer, 0);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    bytes_written =
        cc3xx_asn1_write_len(key_buffer_end, key_buffer, buffer_used);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    bytes_written = cc3xx_asn1_write_tag(key_buffer_end, key_buffer,
                                         CC3XX_TAG_ASN1_SEQUENCE |
                                             CC3XX_TAG_ASN1_CONSTRUCTED);
    buffer_used += bytes_written;
    if (bytes_written < 0) {
        cc_err = CC_OUT_OF_RESOURCE_ERROR;
        goto End;
    }

    /* The asn1 functions write to the end of the buffer.
     * Move the data to the beginning and erase remaining data
     * at the original location. */

    if (2 * buffer_used <= key_buffer_size) {
        CC_PalMemMove(key_buffer, key_buffer + key_buffer_size - buffer_used,
                      buffer_used);
        CC_PalMemSetZero(key_buffer + key_buffer_size - buffer_used,
                         buffer_used);
    } else if ((size_t)buffer_used < key_buffer_size) {
        CC_PalMemMove(key_buffer, key_buffer + key_buffer_size - buffer_used,
                      buffer_used);
        CC_PalMemSetZero(key_buffer + buffer_used,
                         key_buffer_size - buffer_used);
    }

End:
    /* zeroing temp buffers  */
    CC_PalMemSetZero(temp_buff, d_size_bytes);
    mbedtls_free(temp_buff);

    return cc_err;
}

psa_status_t cc3xx_rsa_cc_error_to_psa_error(CCError_t cc_error)
{
    psa_status_t err;
    switch (cc_error) {
    case CC_SUCCESS:
        return PSA_SUCCESS;
        break;

    case CC_RSA_BASE_MGF_MASK_TOO_LONG:
    case CC_RSA_BASE_OAEP_DECODE_MESSAGE_TOO_LONG:
    case CC_RSA_BASE_OAEP_DECODE_PARAMETER_STRING_TOO_LONG:
    case CC_RSA_BASE_OAEP_ENCODE_MESSAGE_TOO_LONG:
    case CC_RSA_BASE_OAEP_ENCODE_PARAMETER_STRING_TOO_LONG:
    case CC_RSA_CONV_TO_CRT_INVALID_TEMP_BUFF_POINTER_ERROR:
    case CC_RSA_DATA_POINTER_INVALID_ERROR:
    case CC_RSA_DECRYPT_INVALID_OUTPUT_SIZE:
    case CC_RSA_DECRYPT_OUTPUT_SIZE_POINTER_ERROR:
    case CC_RSA_ENCODE_15_MSG_OUT_OF_RANGE:
    case CC_RSA_GET_DER_HASH_MODE_ILLEGAL:
    case CC_RSA_HASH_ILLEGAL_OPERATION_MODE_ERROR:
    case CC_RSA_ILLEGAL_PARAMS_ACCORDING_TO_PRIV_ERROR:
    case CC_RSA_INVALID_CRT_COEFFICIENT_PTR_ERROR:
    case CC_RSA_INVALID_CRT_COEFFICIENT_SIZE_ERROR:
    case CC_RSA_INVALID_CRT_COEFFICIENT_SIZE_PTR_ERROR:
    case CC_RSA_INVALID_CRT_COEFF_VAL:
    case CC_RSA_INVALID_CRT_FIRST_AND_SECOND_FACTOR_SIZE:
    case CC_RSA_INVALID_CRT_FIRST_FACTOR_EXPONENT_VAL:
    case CC_RSA_INVALID_CRT_FIRST_FACTOR_EXP_PTR_ERROR:
    case CC_RSA_INVALID_CRT_FIRST_FACTOR_EXP_SIZE_ERROR:
    case CC_RSA_INVALID_CRT_FIRST_FACTOR_EXP_SIZE_PTR_ERROR:
    case CC_RSA_INVALID_CRT_FIRST_FACTOR_POINTER_ERROR:
    case CC_RSA_INVALID_CRT_FIRST_FACTOR_SIZE:
    case CC_RSA_INVALID_CRT_FIRST_FACTOR_SIZE_ERROR:
    case CC_RSA_INVALID_CRT_FIRST_FACTOR_SIZE_POINTER_ERROR:
    case CC_RSA_INVALID_CRT_PARAMETR_SIZE_ERROR:
    case CC_RSA_INVALID_CRT_SECOND_FACTOR_EXPONENT_VAL:
    case CC_RSA_INVALID_CRT_SECOND_FACTOR_EXP_PTR_ERROR:
    case CC_RSA_INVALID_CRT_SECOND_FACTOR_EXP_SIZE_ERROR:
    case CC_RSA_INVALID_CRT_SECOND_FACTOR_EXP_SIZE_PTR_ERROR:
    case CC_RSA_INVALID_CRT_SECOND_FACTOR_POINTER_ERROR:
    case CC_RSA_INVALID_CRT_SECOND_FACTOR_SIZE:
    case CC_RSA_INVALID_CRT_SECOND_FACTOR_SIZE_ERROR:
    case CC_RSA_INVALID_CRT_SECOND_FACTOR_SIZE_POINTER_ERROR:
    case CC_RSA_INVALID_DECRYPRION_MODE_ERROR:
    case CC_RSA_INVALID_EXPONENT_POINTER_ERROR:
    case CC_RSA_INVALID_EXPONENT_SIZE:
    case CC_RSA_INVALID_EXPONENT_VAL:
    case CC_RSA_INVALID_EXP_BUFFER_SIZE_POINTER:
    case CC_RSA_INVALID_MESSAGE_BUFFER_SIZE:
    case CC_RSA_INVALID_MESSAGE_DATA_SIZE:
    case CC_RSA_INVALID_MODULUS_ERROR:
    case CC_RSA_INVALID_MODULUS_POINTER_ERROR:
    case CC_RSA_INVALID_MODULUS_SIZE:
    case CC_RSA_INVALID_MOD_BUFFER_SIZE_POINTER:
    case CC_RSA_INVALID_OUTPUT_POINTER_ERROR:
    case CC_RSA_INVALID_OUTPUT_SIZE_POINTER_ERROR:
    case CC_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR:
    case CC_RSA_INVALID_PTR_ERROR:
    case CC_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR:
    case CC_RSA_INVALID_SIGNATURE_BUFFER_POINTER:
    case CC_RSA_INVALID_SIGNATURE_BUFFER_SIZE:
    case CC_RSA_INVALID_USER_CONTEXT_POINTER_ERROR:
    case CC_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID:
    case CC_RSA_MGF_ILLEGAL_ARG_ERROR:
    case CC_RSA_MODULUS_EVEN_ERROR:
    case CC_RSA_PKCS1_VER_ARG_ERROR:
    case CC_RSA_PRIM_DATA_STRUCT_POINTER_INVALID:
    case CC_RSA_PRIV_KEY_VALIDATION_TAG_ERROR:
    case CC_RSA_PSS_ENCODING_MODULUS_HASH_SALT_LENGTHS_ERROR:
    case CC_RSA_PUB_KEY_VALIDATION_TAG_ERROR:
    case CC_RSA_USER_CONTEXT_VALIDATION_TAG_ERROR:
    case CC_RSA_WRONG_PRIVATE_KEY_TYPE:
    case CC_RSA_INVALID_MESSAGE_VAL:
    case CC_RSA_INVALID_MESSAGE_DATA_SIZE_IN_SSL_CASE:
    case CC_RSA_ERROR_IN_DECRYPTED_BLOCK_PARSING:
    case CC_RSA_OAEP_DECODE_ERROR:
        err = PSA_ERROR_INVALID_ARGUMENT;
        break;

    case CC_RSA_15_ERROR_IN_DECRYPTED_DATA_SIZE:
        err = PSA_ERROR_BUFFER_TOO_SMALL;
        break;

    case CC_RSA_KEY_GEN_CONDITIONAL_TEST_FAIL_ERROR:
    case CC_RSA_GENERATED_PRIV_KEY_IS_TOO_LOW:
    case CC_RSA_KEY_GENERATION_FAILURE_ERROR:
        err = PSA_ERROR_GENERIC_ERROR;
        break;

    case CC_RSA_CAN_NOT_GENERATE_RAND_IN_RANGE:
    case CC_RSA_ERROR_IN_RANDOM_OPERATION_FOR_ENCODE:
    case CC_RND_STATE_PTR_INVALID_ERROR:
    case CC_RND_GEN_VECTOR_FUNC_ERROR:
        err = PSA_ERROR_GENERIC_ERROR;
        break;

    case CC_RSA_ERROR_VER15_INCONSISTENT_VERIFY:
    case CC_RSA_ERROR_PSS_INCONSISTENT_VERIFY:
        err = PSA_ERROR_INVALID_SIGNATURE;
        break;

    /* For now, there is no better error code for malloc failure, both in CC and
     * mbedtls
     */
    case CC_OUT_OF_RESOURCE_ERROR:
        err = PSA_ERROR_INSUFFICIENT_MEMORY;
        break;

    default:
        CC_PAL_LOG_ERR("Unknown CC_ERROR= 0x%08X\r\n", cc_error, cc_error);
        err = PSA_ERROR_GENERIC_ERROR;
        break;
    }

    CC_PAL_LOG_DEBUG("Converted CC_ERROR %d (0x%08x) to PSA_ERROR %d\r\n",
                     cc_error, cc_error, err);
    return err;
}