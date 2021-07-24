from ctypes import *
libC = CDLL("./log.so")

path ="test_file"
libC.constant_char(path)
path = path.encode()
libC.constant_char(path)
path_buf = create_string_buffer(path)
str_wr = create_string_buffer(b"test1\n")
plp = libC.create_event_pool(path_buf)
plp_ptr = c_void_p(plp)
print("BEFORE PRINT NUM BYTES")
#libC.num_bytes(byref(plp_ptr),path_buf)
libC.append_str(byref(plp_ptr),str_wr,path_buf)
#print("AFTER APPEND")
libC.print_log(plp,path_buf)
print("AFTER PRINT LOG")
libC.close_pool(byref(plp_ptr),path_buf)
