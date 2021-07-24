# Standard Library
import os
from ctypes import *
import shutil

# First Party
from smdebug.core.logger import get_logger
from smdebug.core.utils import ensure_dir

# Local
from .base import TSAccessBase

SMDEBUG_TEMP_PATH_SUFFIX = ".tmp"
MAX_TENS_LEN = 20000
POOL_NAME = "./checkpoint".encode()

def get_temp_path(file_path):
    temp_path = file_path + SMDEBUG_TEMP_PATH_SUFFIX
    return temp_path


WRITE_MODES = ["w", "w+", "wb", "wb+", "a", "a+", "ab", "ab+"]


class TSAccessFile(TSAccessBase):
    def __init__(self, path, mode):
        super().__init__()
        self.path = path
        self.mode = mode

        #INSERTED ATTRIBUTES
        self.libPObj = CDLL("/home/users/jonathan/PyPmem/Obj/pool_pmem.so")
        self.is_event = False

        self.logger = get_logger()
        ensure_dir(path)
        if mode in WRITE_MODES:
            self.temp_path = get_temp_path(self.path)
            ensure_dir(self.temp_path)
            self.open(self.temp_path, mode)
        else:
            self.open(self.path, mode)

    def open(self, path, mode):
        if "event" in path:
        #if False:
             self._accessor = path.encode()
             '''
             Omit creating event pool and only create the pool when
             writing tensor for the first time to PMem
             self.libPlog.create_event_pool(self._accessor)
             '''
             self.is_event = True
        else:
             self._accessor = open(path, mode)

    def write(self, _str):
        start = 0
        length = 0
        print("LENGTH OF THE BYTE STING",len(_str))
        str_len = len(_str)
        if self.is_event:
        #if False:
            #str_wr = create_string_buffer(_str,len(_str))
            #str_wr = (c_char*len(_str))()
            #memmove(str_wr,byref(c_byte(_str)),len(_str))
            #str_wr = create_string_buffer("test".encode())
            #self.libPlog.append_str.argtypes = [c_char_p, c_char_p]
            #plp = self.libPlog.append_str(_str,self._accessor)
            partition = _str
            '''
            while(len(partition) > MAX_TENS_LEN):
              write_str = partition[:MAX_TENS_LEN]
              self.libPObj.persist_tensor(self._accessor,write_str,MAX_TENS_LEN)
              partition = partition[MAX_TENS_LEN:]
            print("LENGTH OF REMAINING PARTITION {}".format(len(partition)))
            '''
            self.libPObj.persist_tensor(POOL_NAME,partition,self._accessor,len(partition))
        else:
            start = self._accessor.tell()
            self._accessor.write(_str)
            length = len(_str)
        return [start, length]

    def flush(self):
        if self.is_event:
            return
        self._accessor.flush()

    def close(self):
        if self.is_event:
            return
        """Close the file and move it from /tmp to a permanent directory."""
        self._accessor.close()
        if self.mode in WRITE_MODES:
            if not os.path.exists(self.temp_path):
                self.logger.info(
                    f"Sagemaker-Debugger: Skipping close of file:{self.temp_path} as file doesn't exist"
                )
                return
            shutil.move(self.temp_path, self.path)
            self.logger.debug(
                f"Sagemaker-Debugger: Wrote {os.path.getsize(self.path)} bytes to file {self.path}"
            )

    def ingest_all(self):
        t0 =  time.time()
        if self.is_event:
          self._data = self.libPObj.load_tensors(POOL_NAME,self._accessor)
        else:
          self._data = self._accessor.read()
        self._datalen = len(self._data)
        self._position = 0
        t1 =  time.time()
        print("Time to Read {}".format(t1-t0))

    def read(self, n):
        assert self._position + n <= self._datalen
        res = self._data[self._position : self._position + n]
        self._position += n
        return res

    def has_data(self):
        return self._position < self._datalen
