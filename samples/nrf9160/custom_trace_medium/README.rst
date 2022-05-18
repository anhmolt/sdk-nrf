.. _custom_trace_medium_sample:

nRF9160: Custom Trace Medium
############################

.. contents::
   :local:
   :depth: 2

The Custom Trace Medium sample demonstrates how to ---TODO---

Requirements
************

The sample supports the following development kit:

.. table-from-sample-yaml::

.. include:: /includes/spm.txt

Overview
********

The sample

Building and running
********************

.. |sample path| replace:: :file:`samples/nrf9160/custom_trace_medium`

.. include:: /includes/build_and_run_nrf9160.txt


Testing
=======

|test_sample|

#. |connect_kit|
#. |connect_terminal|
#. Power on or reset your nRF9160 DK.
#. Observe that the sample starts and connects to the LTE network.
#. Observe that the sample displays the number of received traces on the terminal.
#. Observe that the sample completes with a message on the terminal.

Sample Output
=============

The sample shows the following output:

.. code-block:: console

	TODO
	Bye

Dependencies
************

This sample uses the following |NCS| libraries:

* :ref:`_nrf_modem_lib_readme`

It uses the following `sdk-nrfxlib`_ library:

* :ref:`nrfxlib:nrf_modem`

In addition, it uses the following sample:

* :ref:`secure_partition_manager`