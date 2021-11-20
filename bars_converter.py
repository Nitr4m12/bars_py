# -*- coding: utf-8 -*-
#!/usr/bin/python3

"""bars_converter.py: converts BARS files between platforms"""

__author__ = "Nitr4m12(based on bars_convertor.py from Peter Wunder (@SamusAranX))"
__license__ = "WTFPL"

from utils import *
from typing import List
import bcf_converter as sound

def plural_s(n):
	return "s" if n != 1 else ""

def get_bars_tracks(bars: bytearray, swap_endian: bool=False):

	if bars[0x8:0x12] == b"\xFF\xFE":
		bom = '<'
	else:
		bom = '>'

	# Set our position to the start of the file
	file_size: int = len(bars)
	pos: int = 0

	# Create a structure for the header
	header: Header(bom) #= Header(bom)
	header.data(bars, pos)

	# Check for an invalid file
	if header.magic != BARS_HEADER:
		return False

	if header.size_ != file_size:
		return False

	# Offset an amount of header.size
	pos += header.size + header.count * 4

	# Create a structure for the track structure
	track_struct: TRKStruct(bom, header.count) #= TRKStruct(bom, header.count)
	track_struct.data(bars, pos)

	tracks: List[bytearray]

	for t in range(header.count):
		# Get the offset of our track
		pos = track_struct.offsets[t * 2 + 1] 

		if bars_track_offset >= header.size_: 
			# The offset the file is telling us to jump to can't exist because the file's too small
			continue

		fwav = FWAVHeader(bom)
		fwav.data(bars, pos)

		fwav.data_ = bars[pos:pos + fwav.size_]

		tracks.append(fwav.data_)

	return tracks

def convert_track(track: bytearray, dest_bom: str):
	# Read the header of our track
	dest: str = track[:0x4].decode('utf-8')

	# Run the track through the converter
	new_track: bytearray = sound.convFile(track, dest, dest_bom)
	track = new_track

	return track
		
def read_bars(bars, dest_bom):

	if bars[0x8:0x12] == b"\xFF\xFE":
		bom = '<'
	else:
		bom = '>'

	# Create an output buffer the length of our file
	output_buffer = bytearray(len(f))
	pos = 0

	# Create a structure for the header
	header = Header(bom)
	header.data(f, pos)

	# Check for an invalid file
	if header.magic != BARS_HEADER:
		raise RuntimeError(f"Not a valid BARS file.")

	file_size = len(f)
	if header.size_ != file_size:
		# raise RuntimeError(f"File size mismatch (expected {header.size_} bytes, got {file_size} bytes)")
		pass

	# Write the header data to the output buffer
	output_buffer[pos:pos + header.size] = bytes(
		Header(dest_bom).pack("BARS".encode("utf-8").ljust(4, b'\0'), header.size_, header.endian, header.reserved, header.count)
	)

	# Offset an amount of header.size
	pos += header.size

	trk_info = TRKInfo(bom, header.count)
	trk_info.data(f, pos)
	output_buffer[pos:pos + trk_info.size] = TRKInfo(dest_bom, header.count).pack(*trk_info.unknown)
	pos += trk_info.size

	# Create a structure for the track structure
	track_struct = TRKStruct(bom, header.count)
	track_struct.data(f, pos)

	# Write the track structure data to the output buffer
	output_buffer[pos:pos + track_struct.size] = TRKStruct(dest_bom, header.count).pack(*track_struct.offsets)

	track_names = []
	
	for t in range(header.count):
		amta_offset = track_struct.offsets[t * 2] # Get AMTA offset from list
		# Position ourselves at the AMTA offset
		pos = amta_offset

		# Create a structure for the AMTA header
		amta = AMTAHeader(bom)
		amta.data(f, pos)

		if amta.magic != AMTA_HEADER:
			raise RuntimeError(f"Track {t+1} has an invalid AMTA header")

		# Write the AMTA data to the output buffer
		output_buffer[pos:pos + amta.size] = bytes(
			AMTAHeader(dest_bom).pack(amta.magic, amta.endian, amta.reserved, amta.length, amta.data_offset, amta.mark_offset, amta.ext_offset, amta.strg_offset)
		)

		# Offset an amount of amta.size
		pos += amta.size

		for i in MAGICS:

			data = AMTASubHeader(bom)
			data.data(f, pos)
				
			output_buffer[pos:pos + data.size] = bytes(AMTASubHeader(dest_bom).pack(data.magic, data.length))
			pos += data.size

			data.data_ = f[pos:pos + data.length]
			start = 0
			if i == b'DATA':
				output_buffer[pos:pos + 12] = f[pos:pos + 12]
				start = 12
			
			if bom != dest_bom and i != b'STRG':
				for j in range(start, len(data.data_), 4):
					output_buffer[pos + j:pos + j + 4] = bytes([
						data.data_[j+3], data.data_[j+2], data.data_[j+1], data.data_[j] # Do an endian swap
					])
			elif bom != dest_bom:
				output_buffer[pos:pos + data.length] = data.data_

			pos += data.length
		
			if data.magic != i:
				raise RuntimeError(f"Track {t+1} has an invalid {data.magic} header")
	# Print out the number of tracks found
	print(f"{header.count} track{plural_s(header.count)} found!")

	if pos == header.size_: # We have now reached the end of the file despite the file telling us that there would be stuff here
		raise RuntimeError(f"Reached EOF, this file probably doesn't actually contain any FWAVs despite containing the offsets for them")

	for t in range(header.count):
		bars_track_offset = track_struct.offsets[t * 2 + 1] # Get track offset from list
		if bars_track_offset >= header.size_: # The offset the file is telling us to jump to can't exist because the file's too small
			# print(f"{filename}: Track {t+1} probably doesn't exist, skipping it")
			continue

		pos = bars_track_offset

		fwav = FWAVHeader(bom)
		fwav.data(f, pos)

		fwav_converted = sound.convFile(f[pos:pos + fwav.size_], sound.bytes_to_string(fwav.magic), dest_bom)

		output_buffer[pos:pos + fwav.size_] = bytes(fwav_converted)

	output_buffer[4:8] = struct.pack(dest_bom + "I", len(output_buffer))

	return output_buffer
