from ctypes import *
libC = CDLL("./tensor.so")
path = b"test_pool_file"
path_buf = create_string_buffer(path)
buf = b"Hi there"*8000
buf2 = b"Testing LUL"
#buf_buf = create_string_buffer(buf)
#buf_buf2 = create_string_buffer(buf2)
#libC.persist_tensor(path_buf,buf_buf2,len(buf2))
libC.persist_tensor(path,buf2,len(buf2))
libC.persist_tensor(path,buf,len(buf))
#libC.persist_tensor(path_buf,buf_buf,len(buf))
