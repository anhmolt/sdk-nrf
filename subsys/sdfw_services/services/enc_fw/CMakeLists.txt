#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

generate_and_add_cbor_files(enc_fw_service.cddl
  INSTALL_DIR zcbor_generated
  ENTRY_TYPES
    enc_fw_req
    enc_fw_rsp
)
