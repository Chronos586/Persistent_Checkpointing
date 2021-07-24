from ctypes import *
libC = CDLL("./log.so")

path = b"tmp/events/000000000000/000000000000_worker_0.tfevents.tmp"
path_buf = create_string_buffer(path)
libC.print_log(path_buf)
