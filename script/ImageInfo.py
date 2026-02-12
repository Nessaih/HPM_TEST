import struct
import zlib
import argparse
import os
from typing import Optional
from intelhex import IntelHex

'''I'm Hex struct

struct program_config_t
{
    char     image_ver[64];
    u32      image_addr;
    u32      image_size;
    u32      image_crc32;
    u32      image_state;
    u32      reserved[8];
    u32      struct_num;
    u32      struct_addr;
    u32      struct_size;
    u32      struct_crc32;
};

program_config_t config @ 0x00000000;

'''


VERSION_OFFSET_IN_IMAGE  = 0x200  # Offset of version string in image file
VERSION_STRING_LENGTH    = 64   # Length of version string in bytes
IMAGE_STORE_ADDRESS      = 0x00098000
IMAGE_MAGIC_NEED_UPGRADE = 0x5A5A5A5A
RESERVED_VALUE           = 0x00000000
STRUCT_SIZE              = 128       

DEFAULT_STRUCT_STORT_ADDRESS = 0x00118000  # Default value for saddr field
DEFAULT_OUTPUT_FILENAME = "program_config.bin"  # Default output filename
DEFAULT_HEX_FILENAME = "program_config.hex"  # Default HEX output filename


def auto_int(x: str) -> int:
    """
    Convert a string to integer, supporting both decimal and hexadecimal (with 0x prefix).
    
    Args:
        x: Input string
        
    Returns:
        Integer value
    """
    return int(x, 0)


def convert_bin_to_hex(bin_file_path: str, hex_file_path: str, start_address: int) -> None:
    """
    Convert a binary file to Intel HEX format.
    
    Args:
        bin_file_path: Path to input binary file
        hex_file_path: Path to output HEX file
        start_address: Starting address for the HEX file content
    """
    if IntelHex is None:
        print("HEX conversion skipped: intelhex library not available")
        return
    
    # Create IntelHex object
    ih = IntelHex()
    
    # Read binary file and load into IntelHex at specified address
    with open(bin_file_path, 'rb') as f:
        binary_data = f.read()
    
    ih.frombytes(binary_data, offset=start_address)
    
    # Write to HEX file
    ih.write_hex_file(hex_file_path, write_start_addr=True)
    
    print(f"HEX file generated successfully!")
    print(f"HEX Output file: {hex_file_path}")
    print(f"HEX Start address: 0x{start_address:08X}")


def generate_program_config(input_file_path: str, snum: int, saddr: int = DEFAULT_STRUCT_STORT_ADDRESS, output_file_path: str = None, hex_output_path: Optional[str] = None) -> None:
    """
    Generate a program configuration binary file based on the input file.
    
    Args:
        input_file_path: Path to the input file
        snum: snum field value (uint32_t)
        saddr: saddr field value (uint32_t)
        output_file_path: Path to the output binary file
        hex_output_path: Path to the output HEX file (optional, defaults to same name as bin with .hex extension)
    """
    # Set default output path if not provided
    if output_file_path is None:
        input_dir = os.path.dirname(input_file_path)
        if not input_dir:
            input_dir = '.'
        output_file_path = os.path.join(input_dir, DEFAULT_OUTPUT_FILENAME)
    
    # Set default hex output path if not provided
    if hex_output_path is None:
        # Generate hex filename from bin filename
        bin_dir = os.path.dirname(output_file_path)
        bin_basename = os.path.basename(output_file_path)
        hex_basename = os.path.splitext(bin_basename)[0] + '.hex'
        hex_output_path = os.path.join(bin_dir, hex_basename)
    
    # Read input file data
    with open(input_file_path, 'rb') as f:
        file_data = f.read()
        file_size = len(file_data)
        
        # Read VERSION_STRING_LENGTH bytes from offset VERSION_OFFSET_IN_IMAGE, pad with zeros if necessary
        f.seek(VERSION_OFFSET_IN_IMAGE)
        iver_data = f.read(VERSION_STRING_LENGTH)
        iver_data = iver_data.ljust(VERSION_STRING_LENGTH, b'\x00')  # Ensure exactly VERSION_STRING_LENGTH bytes
    
    # Calculate CRC32 of the entire input file
    file_crc32 = zlib.crc32(file_data) & 0xFFFFFFFF  # Ensure unsigned 32-bit value
    
    # Pack the struct data (excluding scrc32 field)
    # Format: < (little-endian) + 64s (VERSION_STRING_LENGTH bytes for iver) + 15I (15 uint32_t fields)
    struct_part = struct.pack(
        '<64s15I',
        iver_data,              # iver: VERSION_STRING_LENGTH bytes from VERSION_OFFSET_IN_IMAGE
        IMAGE_STORE_ADDRESS,                  # iaddr: fixed at IMAGE_STORE_ADDRESS
        file_size,              # isize: file size in bytes
        file_crc32,             # icrc32: CRC32 of entire file
        IMAGE_MAGIC_NEED_UPGRADE,                 # istate: fixed at IMAGE_MAGIC_NEED_UPGRADE
        RESERVED_VALUE, RESERVED_VALUE, RESERVED_VALUE, RESERVED_VALUE, RESERVED_VALUE, RESERVED_VALUE, RESERVED_VALUE, RESERVED_VALUE,  # rev[8]: all zeros
        snum,                   # snum: user-provided parameter
        saddr,                  # saddr: user-provided parameter
        STRUCT_SIZE             # ssize: struct size in bytes
    )
    
    # Calculate CRC32 of the struct (excluding scrc32 field)
    struct_crc32 = zlib.crc32(struct_part) & 0xFFFFFFFF
    
    # Pack the full struct including scrc32 field
    full_struct = struct_part + struct.pack('<I', struct_crc32)
    
    # Write the full struct to output binary file
    with open(output_file_path, 'wb') as f:
        f.write(full_struct)
    
    # Print information
    print(f"Program configuration file generated successfully!")
    print(f"BIN Output file: {output_file_path}")
    print(f"Input file size: {file_size} bytes")
    print(f"Input file CRC32: 0x{file_crc32:08X}")
    print(f"Struct CRC32 (excluding scrc32): 0x{struct_crc32:08X}")
    print(f"Generated struct size: {len(full_struct)} bytes")
    print(f"snum: {snum} (0x{snum:08X})")
    print(f"saddr: {saddr} (0x{saddr:08X})")
    
    # Convert to HEX (always do it since it's default now)
    convert_bin_to_hex(output_file_path, hex_output_path, saddr)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate program configuration binary file and HEX file")
    parser.add_argument("-i", "--input", required=True, help="Path to the input file")
    parser.add_argument("-n", "--snum", required=True, type=auto_int, help="snum field value (uint32_t, supports decimal or hex with 0x prefix)")
    parser.add_argument("-a", "--saddr", required=False, type=auto_int, default=DEFAULT_STRUCT_STORT_ADDRESS, help=f"saddr field value (uint32_t, supports decimal or hex with 0x prefix, default: 0x{DEFAULT_STRUCT_STORT_ADDRESS:08X})")
    parser.add_argument("-o", "--output", default=None, help=f"Output BIN file path (default: same directory as input file with name {DEFAULT_OUTPUT_FILENAME})")
    parser.add_argument("--hex", default=None, help="Output HEX file path (optional, default: same name as BIN file with .hex extension)")
    
    args = parser.parse_args()
    print(f"Input file: {args.input}")
    generate_program_config(args.input, args.snum, args.saddr, args.output, args.hex)