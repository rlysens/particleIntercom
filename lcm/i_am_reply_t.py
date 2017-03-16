"""LCM type definitions
This file automatically generated by lcm.
DO NOT MODIFY BY HAND!!!!
"""

try:
    import cStringIO.StringIO as BytesIO
except ImportError:
    from io import BytesIO
import struct

class i_am_reply_t(object):
    __slots__ = ["id", "name"]

    MSG_ID = 3

    def __init__(self):
        self.id = 0
        self.name = [ 0 for dim0 in range(32) ]

    def encode(self):
        buf = BytesIO()
        buf.write(i_am_reply_t._get_packed_fingerprint())
        self._encode_one(buf)
        return buf.getvalue()

    def _encode_one(self, buf):
        buf.write(struct.pack(">i", self.id))
        buf.write(struct.pack('>32b', *self.name[:32]))

    def decode(data):
        if hasattr(data, 'read'):
            buf = data
        else:
            buf = BytesIO(data)
        if buf.read(8) != i_am_reply_t._get_packed_fingerprint():
            raise ValueError("Decode error")
        return i_am_reply_t._decode_one(buf)
    decode = staticmethod(decode)

    def _decode_one(buf):
        self = i_am_reply_t()
        self.id = struct.unpack(">i", buf.read(4))[0]
        self.name = struct.unpack('>32b', buf.read(32))
        return self
    _decode_one = staticmethod(_decode_one)

    _hash = None
    def _get_hash_recursive(parents):
        if i_am_reply_t in parents: return 0
        tmphash = (0x4a63db5ad5c6199a) & 0xffffffffffffffff
        tmphash  = (((tmphash<<1)&0xffffffffffffffff)  + (tmphash>>63)) & 0xffffffffffffffff
        return tmphash
    _get_hash_recursive = staticmethod(_get_hash_recursive)
    _packed_fingerprint = None

    def _get_packed_fingerprint():
        if i_am_reply_t._packed_fingerprint is None:
            i_am_reply_t._packed_fingerprint = struct.pack(">Q", i_am_reply_t._get_hash_recursive([]))
        return i_am_reply_t._packed_fingerprint
    _get_packed_fingerprint = staticmethod(_get_packed_fingerprint)

