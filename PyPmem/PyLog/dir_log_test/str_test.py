
from ctypes import *
libC = CDLL("./copy_str.so")
c_char_p = POINTER(c_char)
_str = libC.return_str()
_strn = cast(_str,c_char_p)
print(_strn.contents)

print(_str)
