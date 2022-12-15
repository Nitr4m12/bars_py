from pathlib import Path

def write_bars(path: Path):
    buffer: bytearray

    buffer = b"BARS\x00\x00\x00\x00\xFE\xFF\x01\x01"

    path = Path(path)
    path.write_bytes(buffer)