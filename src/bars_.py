from .helpers.read_bars_header import parse, FileOffsetSet

def read_bars(file_name: str):
    with open(file_name, "rb") as b:
        file_sets: list[FileOffsetSet] = parse(b)
    print(file_sets)
