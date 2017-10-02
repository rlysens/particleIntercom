"""LCM type definitions
This file automatically generated by lcm.
DO NOT MODIFY BY HAND!!!!
"""

try:
    import cStringIO.StringIO as BytesIO
except ImportError:
    from io import BytesIO
import struct

class echo_request_t(object):
    __slots__ = ["source_id", "destination_id"]

    MSG_ID = 8

    def __init__(self):
        self.source_id = 0
        self.destination_id = 0

    def encode(self):
        buf = BytesIO()
        buf.write(echo_request_t._get_packed_fingerprint())
        self._encode_one(buf)
        return buf.getvalue()

    def _encode_one(self, buf):
        buf.write(struct.pack(">hh", self.source_id, self.destination_id))

    def decode(data):
        if hasattr(data, 'read'):
            buf = data
        else:
            buf = BytesIO(data)
        if buf.read(8) != echo_request_t._get_packed_fingerprint():
            raise ValueError("Decode error")
        return echo_request_t._decode_one(buf)
    decode = staticmethod(decode)

    def _decode_one(buf):
        self = echo_request_t()
        self.source_id, self.destination_id = struct.unpack(">hh", buf.read(4))
        return self
    _decode_one = staticmethod(_decode_one)

    _hash = None
    def _get_hash_recursive(parents):
        if echo_request_t in parents: return 0
        tmphash = (0x18f1ed839291afec) & 0xffffffffffffffff
        tmphash  = (((tmphash<<1)&0xffffffffffffffff)  + (tmphash>>63)) & 0xffffffffffffffff
        return tmphash
    _get_hash_recursive = staticmethod(_get_hash_recursive)
    _packed_fingerprint = None

    def _get_packed_fingerprint():
        if echo_request_t._packed_fingerprint is None:
            echo_request_t._packed_fingerprint = struct.pack(">Q", echo_request_t._get_hash_recursive([]))
        return echo_request_t._packed_fingerprint
    _get_packed_fingerprint = staticmethod(_get_packed_fingerprint)
