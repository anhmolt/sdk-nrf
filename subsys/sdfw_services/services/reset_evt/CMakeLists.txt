#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_sources(reset_evt_service.c)

generate_and_add_cbor_files(reset_evt_service.cddl
  INSTALL_DIR zcbor_generated
  ENTRY_TYPES
    reset_evt_sub_req
    reset_evt_sub_rsp
    reset_evt_notif
)
