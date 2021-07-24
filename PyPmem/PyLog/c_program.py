from ctypes import *
libC = CDLL("./log.so")

path = b"test_file"
path_buf = create_string_buffer(path)
_str = b"\x0B\x02" * 50 + b"\n"
str2 = str(_str).encode()
if(_str == str2):
    print("EQUAL")
else:
    print("NOT EQUAL")
print("TYPE",_str)
str_wr = create_string_buffer(_str)
print("TYPE IS",type(_str))
plp = libC.create_event_pool(path_buf)
#plp_ptr = c_void_p(plp)
libC.num_bytes(path_buf)
libC.append_str(str_wr,path_buf,len(_str))
libC.print_log(path_buf)
