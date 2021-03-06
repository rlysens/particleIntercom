"""LCM type definitions
This file automatically generated by lcm.
DO NOT MODIFY BY HAND!!!!
"""

try:
    import cStringIO.StringIO as BytesIO
except ImportError:
    from io import BytesIO
import struct

class voice_data_t(object):
    __slots__ = ["source_id", "destination_id", "seq_number", "data_size", "data"]

    MSG_ID = 1

    def __init__(self):
        self.source_id = 0
        self.destination_id = 0
        self.seq_number = 0
        self.data_size = 0
        self.data = [ 0 for dim0 in range(482) ]

    def encode(self):
        buf = BytesIO()
        buf.write(voice_data_t._get_packed_fingerprint())
        self._encode_one(buf)
        return buf.getvalue()

    def _encode_one(self, buf):
        buf.write(struct.pack(">iiih", self.source_id, self.destination_id, self.seq_number, self.data_size))
        buf.write(struct.pack('>482b', *self.data[:482]))

    def decode(data):
        if hasattr(data, 'read'):
            buf = data
        else:
            buf = BytesIO(data)
        if buf.read(8) != voice_data_t._get_packed_fingerprint():
            raise ValueError("Decode error")
        return voice_data_t._decode_one(buf)
    decode = staticmethod(decode)

    def _decode_one(buf):
        self = voice_data_t()
        self.source_id, self.destination_id, self.seq_number, self.data_size = struct.unpack(">iiih", buf.read(14))
        self.data = struct.unpack('>482b', buf.read(482))
        return self
    _decode_one = staticmethod(_decode_one)

    _hash = None
    def _get_hash_recursive(parents):
        if voice_data_t in parents: return 0
        tmphash = (0x5b0470be243fe4d0) & 0xffffffffffffffff
        tmphash  = (((tmphash<<1)&0xffffffffffffffff)  + (tmphash>>63)) & 0xffffffffffffffff
        return tmphash
    _get_hash_recursive = staticmethod(_get_hash_recursive)
    _packed_fingerprint = None

    def _get_packed_fingerprint():
        if voice_data_t._packed_fingerprint is None:
            voice_data_t._packed_fingerprint = struct.pack(">Q", voice_data_t._get_hash_recursive([]))
        return voice_data_t._packed_fingerprint
    _get_packed_fingerprint = staticmethod(_get_packed_fingerprint)

