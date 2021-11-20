# -*- coding: utf-8 -*-
#!/usr/bin/python3

import struct

class Header(struct.Struct):
	# Bars Header
	def __init__(self, bom):
		super().__init__(bom + "4sI2HI")

	def data(self, data, pos):
		(self.magic,
		 self.size_,
		 self.endian,
		 self.reserved,
		 self.count) = self.unpack_from(data, pos)

class TRKInfo(struct.Struct):
	# Unknown bytes, but I'm guessing track info
	def __init__(self, bom, count):
		super().__init__(f'{bom}{count}I')

	def data(self, data, pos):
		self.unknown = self.unpack_from(data, pos)

class AMTAHeader(struct.Struct):
	# Amta Header
	def __init__(self, bom):
		super().__init__(bom + "4s2H5I")

	def data(self, data, pos):
		(self.magic,
		 self.endian,
		 self.reserved,
		 self.length,
		 self.data_offset,
		 self.mark_offset,
		 self.ext_offset,
		 self.strg_offset) = self.unpack_from(data, pos)

class BLKHeader(struct.Struct):
	# Header for DATA, MARK, EXT_ and STRG sections
	def __init__(self, bom):
		super().__init__(bom + "4sI")

	def data(self, data, pos):
		(self.magic,
		 self.length) = self.unpack_from(data, pos)

class FWAVHeader(struct.Struct):
	def __init__(self, bom):
		super().__init__(bom + "4s8xI8x2I32x")

	def data(self, data, pos):
		(self.magic,
		 self.size_,
		 self.info_offset,
		 self.data_offset) = self.unpack_from(data, pos)

class TRKStruct(struct.Struct):
	def __init__(self, bom, count):
		super().__init__(f"{bom}{count*2}I")
	
	def data(self, data, pos):
		(self.offsets) = self.unpack_from(data, pos)

# Magic numbers, commonly known as "headers"

MAGICS = [b"DATA", b"MARK", b"EXT_", b"STRG"]
BARS_HEADER = b"BARS"
AMTA_HEADER = b"AMTA"
FWAV_HEADERS = [b"FWAV", b"FSTP"]