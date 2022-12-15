from io import BytesIO
from struct import unpack

class FileOffsetSet():
    def __init__(self, data: BytesIO, bom: chr):
        self.amta_offset: int = unpack(bom+'I', data.read(4))[0]
        self.file_offset: int = unpack(bom+'I', data.read(4))[0]

def parse(data: BytesIO) -> list[FileOffsetSet]:
    view = data.getbuffer()

    bom: chr
    if (view[0x8:0xA] == 0xFFFE):
        bom = '<'
    else:
        bom = '>'

    signature: str     = unpack(bom+"4s", data.read(4))[0]
    assert signature == "BARS"

    size: int          = unpack(bom+'I', data.read(4))[0]
    byteOrderMark: int = unpack(bom+'H', data.read(2))[0]
    assert byteOrderMark == 0xFEFF

    version: int       = unpack(bom+'H', data.read(2))[0]
    file_count: int    = unpack(bom+'I', data.read(4))[0]

    crc32table: list[bytes]
    for i in range(file_count):
        crc32table.append(unpack(bom+'I', data.read(4))[0])

    file_offsets: list[FileOffsetSet]
    for i in range(file_count):
        file_offsets.append(FileOffsetSet(data, bom))

    return file_offsets