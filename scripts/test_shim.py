#!/usr/bin/env python3
"""Host-only dynamic library ABI smoke test. Never proof of PS4 PRX loadability."""
import ctypes
import pathlib
import sys

class Status(ctypes.Structure):
    _fields_ = [
        ("struct_size", ctypes.c_uint32),
        ("abi_version", ctypes.c_uint32),
        ("local_session", ctypes.c_uint32),
        ("network_enabled", ctypes.c_uint32),
    ]

lib = ctypes.CDLL(str(pathlib.Path(sys.argv[1]).resolve()))
lib.plugin_load.argtypes = [ctypes.c_int32, ctypes.c_void_p]
lib.plugin_load.restype = ctypes.c_int32
lib.plugin_unload.argtypes = [ctypes.c_int32, ctypes.c_void_p]
lib.plugin_unload.restype = ctypes.c_int32
lib.psnn_query_offline_status_v1.argtypes = [ctypes.POINTER(Status)]
lib.psnn_query_offline_status_v1.restype = ctypes.c_int32
s = Status()
s.struct_size = ctypes.sizeof(s)
assert lib.psnn_query_offline_status_v1(ctypes.byref(s)) == 1
assert lib.plugin_load(0, None) == 0
assert lib.psnn_query_offline_status_v1(ctypes.byref(s)) == 0
assert s.abi_version == 1 and s.local_session == 0 and s.network_enabled == 0
assert lib.psnn_query_offline_status_v1(None) == -1
assert lib.plugin_unload(0, None) == 0
assert lib.psnn_query_offline_status_v1(ctypes.byref(s)) == 1
print("PASS plugin lifecycle/query ABI; network=0 and local_session=0")
