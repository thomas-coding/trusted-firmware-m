/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cc3xx_psa_key_generation.h"
#include "cc3xx_internal_asn1_util.h"
#include "cc3xx_internal_drbg_util.h"
#include "cc3xx_internal_ecc_util.h"
#include "cc3xx_internal_rsa_util.h"
#include "cc3xx_psa_hash.h"
#include "psa/crypto.h"

#include "cc_common.h"
#include "cc_ecc_internal.h"
#include "cc_ecpki_build.h"
#include "cc_ecpki_domain.h"
#include "cc_ecpki_ecdsa.h"
#include "cc_ecpki_error.h"
#include "cc_ecpki_kg.h"
#include "cc_ecpki_local.h"
#include "cc_pal_abort.h"
#include "cc_pal_mem.h"
#include "cc_pal_types.h"
#include "cc_rsa_error.h"
#include "cc_rsa_kg.h"
#include "cc_rsa_types.h"
#include "pka_ec_wrst.h"
#include "pki.h"
#include "rsa.h"
#include "rsa_private.h"
#include "rsa_public.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf printf
#define mbedtls_calloc calloc
#define mbedtls_free free
#endif

/* Based on ecp_wrst_gen_keypair_base */
static psa_status_t
cc3xx_internal_gen_ecc_wstr_keypair(const psa_key_attributes_t *attributes,
                                    uint8_t *key_buffer, size_t key_buffer_size)
{
    psa_key_type_t key_type = psa_get_key_type(attributes);
    psa_key_type_t key_bits = psa_get_key_bits(attributes);
    psa_ecc_family_t curve = PSA_KEY_TYPE_ECC_GET_FAMILY(key_type);
    psa_status_t ret;
    CCError_t rc;
    const CCEcpkiDomain_t *pDomain;
    CCEcpkiDomainID_t domainId;
    CCEcpkiUserPrivKey_t *pUserPrivKey;
    CCEcpkiUserPublKey_t *pUserPublKey;
    CCEcpkiPrivKey_t *pPrivKey;
    CCEcpkiKgTempData_t *pTempBuff = NULL;
    CCRndContext_t *pRndContext;
    uint32_t pTempBuffsize =
        sizeof(CCEcpkiKgTempData_t) + sizeof(CCRndContext_t) +
        sizeof(CCEcpkiUserPrivKey_t) + sizeof(CCEcpkiUserPublKey_t);

    ret = cc3xx_ecc_psa_domain_to_cc_domain(curve, key_bits, &domainId);
    if (ret != PSA_SUCCESS) {
        CC_PAL_LOG_ERR("Error - curve is not supported\n");
        return ret;
    }

    pDomain = CC_EcpkiGetEcDomain(domainId);
    if (NULL == pDomain) {
        CC_PAL_LOG_ERR("Error - domain id %d is not supported\n", domainId);
        return PSA_ERROR_NOT_SUPPORTED;
    }

    pTempBuff = mbedtls_calloc(1, pTempBuffsize);
    if (NULL == pTempBuff) {
        CC_PAL_LOG_ERR(
            "Error - failed to allocate memory for temporary buffer\n");
        return PSA_ERROR_INSUFFICIENT_MEMORY;
    }

    pRndContext = (CCRndContext_t *)(pTempBuff + 1);
    pUserPrivKey = (CCEcpkiUserPrivKey_t *)(pRndContext + 1);
    pUserPublKey = (CCEcpkiUserPublKey_t *)(pUserPrivKey + 1);

    ret = cc3xx_ctr_drbg_get_ctx(pRndContext);
    if (ret != PSA_SUCCESS) {
        goto end;
    }

    pPrivKey = (CCEcpkiPrivKey_t *)&pUserPrivKey->PrivKeyDbBuff;

    rc = CC_EcpkiKeyPairGenerate(pRndContext, pDomain, pUserPrivKey,
                                 pUserPublKey, pTempBuff, NULL);

    if (rc != CC_SUCCESS) {
        CC_PAL_LOG_ERR("Error - Key generation ended with result: %d\n", rc);
        ret = cc3xx_ecc_cc_error_to_psa_error(rc);
        goto end;
    }

    CC_CommonReverseMemcpy(key_buffer, (uint8_t *)pPrivKey->PrivKey,
                           PSA_BITS_TO_BYTES(key_bits));

    ret = PSA_SUCCESS;

end:
    CC_PalMemSetZero(pTempBuff, pTempBuffsize);
    mbedtls_free(pTempBuff);
    return ret;
}

static psa_status_t
cc3xx_internal_gen_rsa_keypair(const psa_key_attributes_t *attributes,
                               uint8_t *key_buffer, size_t key_buffer_size)
{
    CCError_t cc_err = CC_FAIL;
    psa_status_t status;
    psa_key_type_t key_bits = psa_get_key_bits(attributes);
    uint32_t pubExpSizeBits, mask;
    CCRndContext_t rndContext;
    CCRsaPubKey_t *pCcPubKey = NULL;
    CCRsaPrivKey_t *pCcPrivKey = NULL;
    CCRsaKgData_t *pKeyGenData = NULL;
    uint32_t *d_buff = NULL;

    /* PSA apis states that the public exponent is 65537 */
    uint32_t pubExp = CC_RSA_KG_PUB_EXP_ALLOW_VAL_3;

    uint32_t keySizeBytes = CALC_FULL_BYTES(key_bits);

    /*  check that the key size allowed by CRYS requirements  */
    if ((key_bits < CC3XX_RSA_MIN_VALID_KEYGEN_SIZE_VALUE_IN_BITS) ||
        (key_bits > CC3XX_RSA_MAX_VALID_KEYGEN_SIZE_VALUE_IN_BITS) ||
        (key_bits % CC_RSA_VALID_KEY_SIZE_MULTIPLE_VALUE_IN_BITS)) {
        return PSA_ERROR_DATA_INVALID;
    }

    pCcPubKey = (CCRsaPubKey_t *)mbedtls_calloc(1, sizeof(CCRsaPubKey_t));
    if (pCcPubKey == NULL) {
        return PSA_ERROR_INSUFFICIENT_MEMORY;
    }

    pCcPrivKey = (CCRsaPrivKey_t *)mbedtls_calloc(1, sizeof(CCRsaPrivKey_t));
    if (pCcPubKey == NULL) {
        mbedtls_free(pCcPubKey);
        return PSA_ERROR_INSUFFICIENT_MEMORY;
    }

    pKeyGenData = (CCRsaKgData_t *)mbedtls_calloc(1, sizeof(CCRsaKgData_t));
    if (pCcPubKey == NULL) {
        mbedtls_free(pCcPubKey);
        mbedtls_free(pCcPrivKey);
        return PSA_ERROR_INSUFFICIENT_MEMORY;
    }

    d_buff = (uint32_t *)mbedtls_calloc(1, PSA_BITS_TO_BYTES(key_bits));
    if (pCcPubKey == NULL) {
        mbedtls_free(pKeyGenData);
        mbedtls_free(pCcPubKey);
        mbedtls_free(pCcPrivKey);
        return PSA_ERROR_INSUFFICIENT_MEMORY;
    }

    status = cc3xx_ctr_drbg_get_ctx(&rndContext);
    if (status != PSA_SUCCESS) {
        cc_err = CC_FAIL;
        goto end;
    }

    /* get pub.exp size in bits */
    pubExpSizeBits = 32;
    mask = 1UL << 31;
    while ((pubExp & mask) == 0) {
        pubExpSizeBits--;
        mask >>= 1;
    }

    /* init sizes */
    pCcPubKey->nSizeInBits = key_bits;
    pCcPubKey->eSizeInBits = pubExpSizeBits;
    pCcPrivKey->nSizeInBits = key_bits;
    pCcPubKey->e[0] = pubExp;

    /* set params for non CRT */
    pCcPrivKey->OperationMode = CC_RSA_NoCrt; /* default mode */
    pCcPrivKey->PriveKeyDb.NonCrt.eSizeInBits = pCcPubKey->eSizeInBits;
    pCcPrivKey->PriveKeyDb.NonCrt.e[0] = pubExp;

    /* .....   calculate primes (P, Q) and nonCRT key (N, D) ..... */
    do {
        cc_err = RsaGenPandQ(&rndContext, key_bits, pubExpSizeBits,
                             (uint32_t *)&pubExp, pKeyGenData);
        if (cc_err != CC_OK) {
            goto end;
        }

        /* calculate modulus n and private nonCRT exponent d */
        cc_err =
            RsaCalculateNandD(pCcPubKey, pCcPrivKey, pKeyGenData, key_bits / 2);

        if (cc_err != CC_OK) {
            goto end;
        }

        /* repeat the loop if D is too low */
    } while (cc_err == CC_RSA_GENERATED_PRIV_KEY_IS_TOO_LOW);

    /* calculate Barr. tag for modulus N */
    cc_err = PkiCalcNp(((RsaPubKeyDb_t *)(pCcPubKey->ccRSAIntBuff))->NP, /*out*/
                       pCcPubKey->n, key_bits);                          /*in*/

    if (cc_err != CC_OK) {
        goto end;
    }

    /* The NonCrt.d will be discarded when we caclulate the Crt parameterers so
     * we need to keep it */
    CC_PalMemCopy(d_buff, pCcPrivKey->PriveKeyDb.NonCrt.d, keySizeBytes);

    /* calculate Barrett tags for P,Q and set into context */
    cc_err = PkiCalcNp(
        ((RsaPrivKeyDb_t *)(pCcPrivKey->ccRSAPrivKeyIntBuff))->Crt.PP, /*out*/
        pKeyGenData->KGData.p, key_bits / 2);                          /*in*/
    if (cc_err != CC_OK) {
        goto end;
    }
    cc_err = PkiCalcNp(
        ((RsaPrivKeyDb_t *)(pCcPrivKey->ccRSAPrivKeyIntBuff))->Crt.QP, /*out*/
        pKeyGenData->KGData.q, key_bits / 2);                          /*in*/
    if (cc_err != CC_OK) {
        goto end;
    }

    /* calculate CRT parameters */
    pCcPrivKey->OperationMode = CC_RSA_Crt;
    cc_err = RsaCalculateCrtParams(
        (uint32_t *)&pubExp, pubExpSizeBits, key_bits, pKeyGenData->KGData.p,
        pKeyGenData->KGData.q, pCcPrivKey->PriveKeyDb.Crt.dP,
        pCcPrivKey->PriveKeyDb.Crt.dQ, pCcPrivKey->PriveKeyDb.Crt.qInv);

    if (cc_err != CC_OK) {
        goto end;
    }

    CC_PalMemSetZero(key_buffer, key_buffer_size);
    cc_err = cc3xx_rsa_save_der_priv_key(
        key_buffer, key_buffer_size, pCcPubKey->n, pCcPubKey->e, d_buff,
        pKeyGenData->KGData.p, pKeyGenData->KGData.q,
        pCcPrivKey->PriveKeyDb.Crt.dP, pCcPrivKey->PriveKeyDb.Crt.dQ,
        pCcPrivKey->PriveKeyDb.Crt.qInv, keySizeBytes);

    if (cc_err != CC_OK) {
        CC_PalMemSetZero(key_buffer, key_buffer_size);
    }

end:
    /* zeroing temp buffers  */
    CC_PalMemSetZero(d_buff, sizeof(PSA_BITS_TO_BYTES(key_bits)));
    CC_PalMemSetZero(pCcPrivKey, sizeof(CCRsaPrivKey_t));
    CC_PalMemSetZero(pCcPubKey, sizeof(CCRsaPubKey_t));
    CC_PalMemSetZero(pKeyGenData, sizeof(CCRsaKgData_t));

    mbedtls_free(d_buff);
    mbedtls_free(pKeyGenData);
    mbedtls_free(pCcPubKey);
    mbedtls_free(pCcPrivKey);

    return cc3xx_rsa_cc_error_to_psa_error(cc_err);
}

psa_status_t cc3xx_generate_key(const psa_key_attributes_t *attributes,
                                uint8_t *key_buffer, size_t key_buffer_size,
                                size_t *key_buffer_length)
{
    psa_key_type_t key_type = psa_get_key_type(attributes);
    psa_key_type_t key_bits = psa_get_key_bits(attributes);
    psa_status_t err = PSA_ERROR_NOT_SUPPORTED;

    if (key_buffer_size < PSA_BITS_TO_BYTES(key_bits)) {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    if (PSA_KEY_TYPE_IS_KEY_PAIR(key_type)) {
        if (PSA_KEY_TYPE_IS_ECC(key_type)) {
            if (PSA_KEY_TYPE_ECC_GET_FAMILY(key_type) ==
                    PSA_ECC_FAMILY_SECP_K1 ||
                PSA_KEY_TYPE_ECC_GET_FAMILY(key_type) ==
                    PSA_ECC_FAMILY_SECP_R1 ||
                PSA_KEY_TYPE_ECC_GET_FAMILY(key_type) ==
                    PSA_ECC_FAMILY_SECP_R2) {
                err = cc3xx_internal_gen_ecc_wstr_keypair(
                    attributes, key_buffer, key_buffer_size);
            }
        } else if (PSA_KEY_TYPE_IS_RSA(key_type)) {
            err = cc3xx_internal_gen_rsa_keypair(attributes, key_buffer,
                                                 key_buffer_size);
        }
    } else if (PSA_KEY_TYPE_IS_UNSTRUCTURED(key_type)) {
        err = psa_generate_random(key_buffer, key_buffer_size);
    }

    return err;
}

psa_status_t cc3xx_export_public_key(const psa_key_attributes_t *attributes,
                                     const uint8_t *key_buffer,
                                     size_t key_buffer_size, uint8_t *data,
                                     size_t data_size, size_t *data_length)
{
    psa_key_type_t key_type = psa_get_key_type(attributes);
    psa_key_type_t key_bits = psa_get_key_bits(attributes);
    psa_ecc_family_t curve = PSA_KEY_TYPE_ECC_GET_FAMILY(key_type);
    CCEcpkiUserPrivKey_t pUserPrivKey;
    CCEcpkiUserPublKey_t pUserPublKey;
    psa_status_t err;
    CCEcpkiDomainID_t domainId;

    if (PSA_KEY_TYPE_IS_ECC(key_type) && PSA_KEY_TYPE_IS_PUBLIC_KEY(key_type)) {
        err = cc3xx_ecc_psa_domain_to_cc_domain(curve, key_bits, &domainId);
        if (err != PSA_SUCCESS) {
            CC_PAL_LOG_ERR("Error - curve is not supported\n");
            return err;
        }

        err = cc3xx_ecc_psa_priv_to_cc_priv(domainId, key_buffer,
                                            key_buffer_size, &pUserPrivKey);

        if (err != PSA_SUCCESS) {
            return err;
        }

        err = cc3xx_ecc_cc_priv_to_cc_publ(&pUserPrivKey, &pUserPublKey);
        if (err != PSA_SUCCESS) {
            return err;
        }

        err = cc3xx_ecc_cc_publ_to_psa_publ(&pUserPublKey, data, data_size);
        return err;
    } else if (PSA_KEY_TYPE_IS_RSA(key_type) &&
               PSA_KEY_TYPE_IS_PUBLIC_KEY(key_type)) {
        err = cc3xx_rsa_psa_priv_to_psa_publ((uint8_t *)key_buffer,
                                             key_buffer_size, data, data_size);
        return err;
    }

    return PSA_ERROR_NOT_SUPPORTED;
}