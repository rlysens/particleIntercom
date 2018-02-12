"""LCM type definitions
This file automatically generated by lcm.
DO NOT MODIFY BY HAND!!!!
"""

try:
    import cStringIO.StringIO as BytesIO
except ImportError:
    from io import BytesIO
import struct

class who_is_t(object):
    __slots__ = ["name", "padding"]

    MSG_ID = 4

    def __init__(self):
        self.name = [ 0 for dim0 in range(20) ]
        self.padding = [ 0 for dim0 in range(4) ]

    def encode(self):
        buf = BytesIO()
        buf.write(who_is_t._get_packed_fingerprint())
        self._encode_one(buf)
        return buf.getvalue()

    def _encode_one(self, buf):
        buf.write(struct.pack('>20b', *self.name[:20]))
        buf.write(struct.pack('>4b', *self.padding[:4]))

    def decode(data):
        if hasattr(data, 'read'):
            buf = data
        else:
            buf = BytesIO(data)
        if buf.read(8) != who_is_t._get_packed_fingerprint():
            raise ValueError("Decode error")
        return who_is_t._decode_one(buf)
    decode = staticmethod(decode)

    def _decode_one(buf):
        self = who_is_t()
        self.name = struct.unpack('>20b', buf.read(20))
        self.padding = struct.unpack('>4b', buf.read(4))
        return self
    _decode_one = staticmethod(_decode_one)

    _hash = None
    def _get_hash_recursive(parents):
        if who_is_t in parents: return 0
        tmphash = (0xeb0beb0b232da356) & 0xffffffffffffffff
        tmphash  = (((tmphash<<1)&0xffffffffffffffff)  + (tmphash>>63)) & 0xffffffffffffffff
        return tmphash
    _get_hash_recursive = staticmethod(_get_hash_recursive)
    _packed_fingerprint = None

    def _get_packed_fingerprint():
        if who_is_t._packed_fingerprint is None:
            who_is_t._packed_fingerprint = struct.pack(">Q", who_is_t._get_hash_recursive([]))
        return who_is_t._packed_fingerprint
    _get_packed_fingerprint = staticmethod(_get_packed_fingerprint)

