/**
 * \file psa_crypto_storage.h
 *
 * \brief PSA cryptography module: Mbed TLS key storage
 */
/*
 *  Copyright (C) 2018, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#ifndef PSA_CRYPTO_STORAGE_H
#define PSA_CRYPTO_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include the Mbed TLS configuration file, the way Mbed TLS does it
 * in each of its header files. */
#if defined(MBEDTLS_CONFIG_FILE)
#include MBEDTLS_CONFIG_FILE
#else
#include "mbedtls/config.h"
#endif

#include "psa/crypto.h"
#include <stdint.h>
#include <string.h>

/* Limit the maximum key size to 30kB (just in case someone tries to
 * inadvertently store an obscene amount of data) */
#define PSA_CRYPTO_MAX_STORAGE_SIZE ( 30 * 1024 )

/** The maximum permitted persistent slot number.
 *
 * In Mbed Crypto 0.1.0b:
 * - Using the file backend, all key ids are ok except 0.
 * - Using the ITS backend, all key ids are ok except 0xFFFFFF52
 *   (#PSA_CRYPTO_ITS_RANDOM_SEED_UID) for which the file contains the
 *   device's random seed (if this feature is enabled).
 * - Only key ids from 1 to #PSA_KEY_SLOT_COUNT are actually used.
 *
 * Since we need to preserve the random seed, avoid using that key slot.
 * Reserve a whole range of key slots just in case something else comes up.
 *
 * This limitation will probably become moot when we implement client
 * separation for key storage.
 */
#define PSA_MAX_PERSISTENT_KEY_IDENTIFIER PSA_KEY_ID_VENDOR_MAX

/**
 * \brief Checks if persistent data is stored for the given key slot number
 *
 * This function checks if any key data or metadata exists for the key slot in
 * the persistent storage.
 *
 * \param key           Persistent identifier to check.
 *
 * \retval 0
 *         No persistent data present for slot number
 * \retval 1
 *         Persistent data present for slot number
 */
int psa_is_key_present_in_storage( const psa_key_file_id_t key );

/**
 * \brief Format key data and metadata and save to a location for given key
 *        slot.
 *
 * This function formats the key data and metadata and saves it to a
 * persistent storage backend. The storage location corresponding to the
 * key slot must be empty, otherwise this function will fail. This function
 * should be called after psa_import_key_into_slot() to ensure the
 * persistent key is not saved into a storage location corresponding to an
 * already occupied non-persistent key, as well as validating the key data.
 *
 *
 * \param key           Persistent identifier of the key to be stored. This
 *                      should be an unoccupied storage location.
 * \param type          Key type (a \c PSA_KEY_TYPE_XXX value).
 * \param[in] policy    The key policy to save.
 * \param[in] data      Buffer containing the key data.
 * \param data_length   The number of bytes that make up the key data.
 *
 * \retval PSA_SUCCESS
 * \retval PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval PSA_ERROR_INSUFFICIENT_STORAGE
 * \retval PSA_ERROR_STORAGE_FAILURE
 * \retval PSA_ERROR_ALREADY_EXISTS
 */
psa_status_t psa_save_persistent_key( const psa_key_file_id_t key,
                                      const psa_key_type_t type,
                                      const psa_key_policy_t *policy,
                                      const uint8_t *data,
                                      const size_t data_length );

/**
 * \brief Parses key data and metadata and load persistent key for given
 * key slot number.
 *
 * This function reads from a storage backend, parses the key data and
 * metadata and writes them to the appropriate output parameters.
 *
 * Note: This function allocates a buffer and returns a pointer to it through
 * the data parameter. psa_free_persistent_key_data() must be called after
 * this function to zeroize and free this buffer, regardless of whether this
 * function succeeds or fails.
 *
 * \param key               Persistent identifier of the key to be loaded. This
 *                          should be an occupied storage location.
 * \param[out] type         On success, the key type (a \c PSA_KEY_TYPE_XXX
 *                          value).
 * \param[out] policy       On success, the key's policy.
 * \param[out] data         Pointer to an allocated key data buffer on return.
 * \param[out] data_length  The number of bytes that make up the key data.
 *
 * \retval PSA_SUCCESS
 * \retval PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval PSA_ERROR_STORAGE_FAILURE
 * \retval PSA_ERROR_DOES_NOT_EXIST
 */
psa_status_t psa_load_persistent_key( psa_key_file_id_t key,
                                      psa_key_type_t *type,
                                      psa_key_policy_t *policy,
                                      uint8_t **data,
                                      size_t *data_length );

/**
 * \brief Remove persistent data for the given key slot number.
 *
 * \param key           Persistent identifier of the key to remove
 *                      from persistent storage.
 *
 * \retval PSA_SUCCESS
 *         The key was successfully removed,
 *         or the key did not exist.
 * \retval PSA_ERROR_STORAGE_FAILURE
 */
psa_status_t psa_destroy_persistent_key( const psa_key_file_id_t key );

/**
 * \brief Free the temporary buffer allocated by psa_load_persistent_key().
 *
 * This function must be called at some point after psa_load_persistent_key()
 * to zeroize and free the memory allocated to the buffer in that function.
 *
 * \param key_data        Buffer for the key data.
 * \param key_data_length Size of the key data buffer.
 *
 */
void psa_free_persistent_key_data( uint8_t *key_data, size_t key_data_length );

/**
 * \brief Formats key data and metadata for persistent storage
 *
 * \param[in] data          Buffer for the key data.
 * \param data_length       Length of the key data buffer.
 * \param type              Key type (a \c PSA_KEY_TYPE_XXX value).
 * \param policy            The key policy.
 * \param[out] storage_data Output buffer for the formatted data.
 *
 */
void psa_format_key_data_for_storage( const uint8_t *data,
                                      const size_t data_length,
                                      const psa_key_type_t type,
                                      const psa_key_policy_t *policy,
                                      uint8_t *storage_data );

/**
 * \brief Parses persistent storage data into key data and metadata
 *
 * \param[in] storage_data     Buffer for the storage data.
 * \param storage_data_length  Length of the storage data buffer
 * \param[out] key_data        On output, pointer to a newly allocated buffer
 *                             containing the key data. This must be freed
 *                             using psa_free_persistent_key_data()
 * \param[out] key_data_length Length of the key data buffer
 * \param[out] type            Key type (a \c PSA_KEY_TYPE_XXX value).
 * \param[out] policy          The key policy.
 *
 * \retval PSA_SUCCESS
 * \retval PSA_ERROR_INSUFFICIENT_STORAGE
 * \retval PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval PSA_ERROR_STORAGE_FAILURE
 */
psa_status_t psa_parse_key_data_from_storage( const uint8_t *storage_data,
                                              size_t storage_data_length,
                                              uint8_t **key_data,
                                              size_t *key_data_length,
                                              psa_key_type_t *type,
                                              psa_key_policy_t *policy );

#if defined(MBEDTLS_PSA_CRYPTO_SE_C)
/** This symbol is defined if transaction support is required. */
#define PSA_CRYPTO_STORAGE_HAS_TRANSACTIONS
#endif

#if defined(PSA_CRYPTO_STORAGE_HAS_TRANSACTIONS)

/** The type of transaction that is in progress.
 */
/* This is an integer type rather than an enum for two reasons: to support
 * unknown values when loading a transaction file, and to ensure that the
 * type has a known size.
 */
typedef uint16_t psa_crypto_transaction_type_t;

/** No transaction is in progress.
 */
#define PSA_CRYPTO_TRANSACTION_NONE             ( (psa_crypto_transaction_type_t) 0x0000 )

/** Transaction data.
 *
 * This type is designed to be serialized by writing the memory representation
 * and reading it back on the same device.
 *
 * \note The transaction mechanism is designed for a single active transaction
 *       at a time. The transaction object is #psa_crypto_transaction.
 *
 * \note If an API call starts a transaction, it must complete this transaction
 *       before returning to the application.
 *
 * The lifetime of a transaction is the following (note that only one
 * transaction may be active at a time):
 *
 * -# Call psa_crypto_prepare_transaction() to initialize the transaction
 *    object in memory and declare the type of transaction that is starting.
 * -# Fill in the type-specific fields of #psa_crypto_transaction.
 * -# Call psa_crypto_save_transaction() to start the transaction. This
 *    saves the transaction data to internal storage.
 * -# If there are intermediate stages in the transaction, update
 *    the fields of #psa_crypto_transaction and call
 *    psa_crypto_save_transaction() again when each stage is reached.
 * -# When the transaction is over, whether it has been committed or aborted,
 *    call psa_crypto_stop_transaction() to remove the transaction data in
 *    storage and in memory.
 *
 * If the system crashes while a transaction is in progress, psa_crypto_init()
 * calls psa_crypto_load_transaction() and takes care of completing or
 * rewinding the transaction.
 */
typedef union
{
    /* Each element of this union must have the following properties
     * to facilitate serialization and deserialization:
     *
     * - The element is a struct.
     * - The first field of the struct is `psa_crypto_transaction_type_t type`.
     * - Elements of the struct are arranged such a way that there is
     *   no padding.
     */
    struct psa_crypto_transaction_unknown_s
    {
        psa_crypto_transaction_type_t type;
    } unknown;
} psa_crypto_transaction_t;

/** The single active transaction.
 */
extern psa_crypto_transaction_t psa_crypto_transaction;

/** Prepare for a transaction.
 *
 * There must not be an ongoing transaction.
 *
 * \param type          The type of transaction to start.
 */
static inline void psa_crypto_prepare_transaction(
    psa_crypto_transaction_type_t type )
{
    psa_crypto_transaction.unknown.type = type;
}

/** Save the transaction data to storage.
 *
 * You may call this function multiple times during a transaction to
 * atomically update the transaction state.
 *
 * \retval #PSA_SUCCESS
 * \retval #PSA_ERROR_INSUFFICIENT_STORAGE
 * \retval #PSA_ERROR_STORAGE_FAILURE
 */
psa_status_t psa_crypto_save_transaction( void );

/** Load the transaction data from storage, if any.
 *
 * This function is meant to be called from psa_crypto_init() to recover
 * in case a transaction was interrupted by a system crash.
 *
 * \retval #PSA_SUCCESS
 *         The data about the ongoing transaction has been loaded to
 *         #psa_crypto_transaction.
 * \retval #PSA_ERROR_DOES_NOT_EXIST
 *         There is no ongoing transaction.
 * \retval #PSA_ERROR_STORAGE_FAILURE
 */
psa_status_t psa_crypto_load_transaction( void );

/** Indicate that the current transaction is finished.
 *
 * Call this function at the very end of transaction processing, whether
 * the transaction has been committed or aborted.
 *
 * This function erases the transaction data in storage (if any) and
 * resets the transaction data in memory.
 *
 * \retval #PSA_SUCCESS
 *         There was transaction data in storage.
 * \retval #PSA_ERROR_DOES_NOT_EXIST
 *         There was no transaction data in storage.
 * \retval #PSA_ERROR_STORAGE_FAILURE
 *         It was impossible to determine whether there was transaction data
 *         in storage, or the transaction data could not be erased.
 */
psa_status_t psa_crypto_stop_transaction( void );

/** The ITS file identifier for the transaction data.
 *
 * 0xffffffNN = special file; 0x74 = 't' for transaction.
 */
#define PSA_CRYPTO_ITS_TRANSACTION_UID ( (psa_key_id_t) 0xffffff74 )

#endif /* PSA_CRYPTO_STORAGE_HAS_TRANSACTIONS */

#if defined(MBEDTLS_PSA_INJECT_ENTROPY)
/** Backend side of mbedtls_psa_inject_entropy().
 *
 * This function stores the supplied data into the entropy seed file.
 *
 * \retval #PSA_SUCCESS
 *         Success
 * \retval #PSA_ERROR_STORAGE_FAILURE
 * \retval #PSA_ERROR_INSUFFICIENT_STORAGE
 * \retval #PSA_ERROR_NOT_PERMITTED
 *         The entropy seed file already exists.
 */
psa_status_t mbedtls_psa_storage_inject_entropy( const unsigned char *seed,
                                                 size_t seed_size );
#endif /* MBEDTLS_PSA_INJECT_ENTROPY */

#ifdef __cplusplus
}
#endif

#endif /* PSA_CRYPTO_STORAGE_H */
