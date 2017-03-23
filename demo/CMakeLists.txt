add_executable (put_file put_file.c)
target_link_libraries (put_file qiniu crypto m)

add_executable (put_stream put_stream.c)
target_link_libraries (put_stream qiniu crypto m)

add_executable (cdn_refresh cdn_refresh.c)
target_link_libraries (cdn_refresh qiniu crypto m)

add_executable (cdn_prefetch cdn_prefetch.c)
target_link_libraries (cdn_prefetch qiniu crypto m)

add_executable (cdn_flux cdn_flux.c)
target_link_libraries (cdn_flux qiniu crypto m)

add_executable (cdn_bandwidth cdn_bandwidth.c)
target_link_libraries (cdn_bandwidth qiniu crypto m)

add_executable (cdn_loglist cdn_loglist.c)
target_link_libraries (cdn_loglist qiniu crypto m)

add_executable (demo demo.c)
target_link_libraries (demo qiniu crypto m)

add_executable (put_file_abortable put_file_abortable.c)
target_link_libraries (put_file_abortable qiniu crypto m)

add_executable (gen_download_url_with_deadline gen_download_url_with_deadline.c)
target_link_libraries (gen_download_url_with_deadline qiniu crypto m)
