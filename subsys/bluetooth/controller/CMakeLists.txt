#
# Copyright (c) 2018 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library()

zephyr_library_sources(
  hci_driver.c
  hci_internal.c
  hci_internal_wrappers.c
)

zephyr_library_sources_ifdef(
  CONFIG_BT_CTLR_CRYPTO
  crypto.c
)

zephyr_library_sources_ifdef(
  CONFIG_BT_CTLR_SDC_CS_MULTIPLE_ANTENNA_SUPPORT
  cs_antenna_switch.c
)

zephyr_library_link_libraries(subsys__bluetooth)

zephyr_include_directories(.)

if(${CONFIG_BT_LL_SOFTDEVICE} AND ${CONFIG_BT_LL_SOFTDEVICE_BUILD_TYPE_LIB})
  set(SDC_SOC ${CONFIG_BT_LL_SOC_DIR})
  set(SDC_FLOAT_TYPE ${CONFIG_BT_LL_LIB_FLOAT_ABI_DIR})

  if(CONFIG_BT_LL_SOFTDEVICE_PERIPHERAL)
    set(SDC_VARIANT "peripheral")
  elseif(CONFIG_BT_LL_SOFTDEVICE_CENTRAL)
    set(SDC_VARIANT "central")
  elseif(CONFIG_BT_LL_SOFTDEVICE_MULTIROLE)
    set(SDC_VARIANT "multirole")
  endif()

  if(CONFIG_BT_LL_SOFTDEVICE_HEADERS_INCLUDE)
    set(SDC_HEADERS_INCLUDE ON)
  else()
    set(SDC_HEADERS_INCLUDE OFF)
  endif()
  if(CONFIG_BT_LL_SOFTDEVICE_HEADERS_INCLUDE_PATH_OVERRIDE)
    set(SDC_HEADERS_INCLUDE_PATH_OVERRIDE ON)
  else()
    set(SDC_HEADERS_INCLUDE_PATH_OVERRIDE OFF)
  endif()

  add_subdirectory(${ZEPHYR_NRFXLIB_MODULE_DIR}/softdevice_controller
    ${CMAKE_BINARY_DIR}/softdevice_controller)

  if(NOT TARGET softdevice_controller)
    message(FATAL_ERROR "SoftDevice Controller target was not created.")
  endif()

  zephyr_link_libraries(softdevice_controller)
endif()
