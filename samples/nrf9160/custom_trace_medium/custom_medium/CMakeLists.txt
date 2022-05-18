#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(CONFIG_NRF_MODEM_LIB_TRACE_ENABLED)

zephyr_library()

# Only add 'custom' medium to compilation when selected.
zephyr_library_sources_ifdef(
  CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_CUSTOM
  custom_medium.c
)

endif()
