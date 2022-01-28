/* nrf_modem trace mediums */
. = ALIGN(4);
_trace_medium_entry_list_start = .;
KEEP(*(SORT_BY_NAME("._trace_medium_entry.*")));
_trace_medium_entry_list_end = .;
