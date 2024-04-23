#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_sources(psa_crypto_service.c)

generate_and_add_cbor_files(psa_crypto_service.cddl
  INSTALL_DIR zcbor_generated
  ENTRY_TYPES
    psa_crypto_req
    psa_crypto_rsp
)
