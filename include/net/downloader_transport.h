/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file downloader_transport.h
 *
 * @ingroup downloader
 * @{
 * @brief Downloader transport definition.
 */

#ifndef DOWNLOADER_TRANSPORT_H
#define DOWNLOADER_TRANSPORT_H

#include <net/downloader.h>

/**
 * @brief Transport data event callback.
 *
 * This function is called by the transport to notify the downloader of downloaded data.
 *
 * @param dl Downloader instance.
 * @param data Downloaded data.
 * @param len Length of downloaded data.
 *
 * @retval Zero if the fragment was accepted and the download can continue.
 * @return Negative errno if the fragment was refused by the application and the download
 *         should be aborted.
 */
int dl_transport_evt_data(struct downloader *dl, void *data, size_t len);

/**
 * Downloader transport API
 */
struct dl_transport {
	/**
	 * Parse protocol
	 *
	 * @param dl Downloader instance.
	 * @param uri URI
	 *
	 * @retval true if protocol is supported by the transport
	 * @retval false if protocol is not supported by the transport
	 */
	bool (*proto_supported)(struct downloader *dl, const char *uri);
	/**
	 * Initialize DL transport
	 *
	 * @param dl Downloader instance.
	 * @param host_conf Host configuration.
	 * @param uri URI
	 *
	 * @returns 0 on success, negative error on failure.
	 */
	int (*init)(struct downloader *dl, struct downloader_host_cfg *dl_host_cfg,
		    const char *uri);
	/**
	 * Deinitialize DL transport
	 *
	 * @param dl Downloader instance.
	 *
	 * @returns 0 on success, negative error on failure.
	 */
	int (*deinit)(struct downloader *dl);
	/**
	 * Connect DL transport.
	 *
	 * Connection result is given by callback to @c dl_transport_event_connected.
	 *
	 * @param dl Downloader instance.
	 *
	 * @returns 0 on success, negative error on failure.
	 */
	int (*connect)(struct downloader *dl);
	/**
	 * Close DL transport
	 *
	 * @param dl Downloader instance.
	 *
	 * @returns 0 on success, negative error on failure.
	 */
	int (*close)(struct downloader *dl);
	/**
	 * Download data with DL transport
	 *
	 * @param dl Downloader instance.
	 *
	 * @returns 0 on success, negative error on failure.
	 * Return -ECONNRESET if the downloader can reconnect to resume the download.
	 */
	int (*download)(struct downloader *dl);
};

/**
 * @brief Define a DL transport and place it in a dedicated DL transport iterable section in ROM.
 *
 * @param _name transport name.
 */
#define DL_TRANSPORT(_name)                                                                        \
	const STRUCT_SECTION_ITERABLE(dl_transport, CONCAT(dl_transport_, _name))

#endif /* DOWNLOADER_TRANSPORT_H */

/**@} */
