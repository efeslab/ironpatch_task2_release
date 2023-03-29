import argparse
import re
import logging

CAN_FRAME_SIZE = 16

class CANFrameExtractor(object):
    """
    @param can_open_prefix str The string prefix of the syscall that opens the CAN socket we are interested in
    """
    def __init__(self, can_open_prefix: str):
        self.can_open_prefix = can_open_prefix
        self.can_frame_prefix = None
        self.canFD = None
        self.canFrames = []
        logging.info("Looking for CAN socket FD")
    
    def parseLine(self, line: str):
        if self.canFD is None:
            if line.startswith(self.can_open_prefix):
                self.canFD = int(line.split('=')[-1])
                self.can_frame_prefix = f"read({self.canFD}"
                logging.info(f"Found CAN socket FD: {self.canFD}")
        else:
            if line.startswith(self.can_frame_prefix):
                # e.g.,
                # read(5, "\x00\x7f\xec\x98\x08\x00\x00\x00\x10\x06\xf9\xff\xff\xde\xad\xbe", 16) = 16
                split = re.split(r"(\(|\)|,|=)", line)
                can_frame_hexstr = split[4].strip()
                requested_size = int(split[6])
                return_size = int(split[10])
                assert(return_size == CAN_FRAME_SIZE)
                assert(requested_size == CAN_FRAME_SIZE)
                can_frame_bytes = bytearray()
                for b in re.findall(r'\\x[0-9a-z]{2}', can_frame_hexstr):
                    can_frame_bytes.append(int(b.replace('\\','0'), 16))
                self.canFrames.append(can_frame_bytes)
    
    def exportCHeader(self, header_path, msg = ""):
        with open(header_path, "w") as f:
            NR_ITERATIONS = len(self.canFrames)
            print( "#ifndef CAN_DUMP_HEADER_H", file=f)
            print( "#define CAN_DUMP_HEADER_H", file=f)
            print( "// Automatically generated from strace logs", file=f)
            print(f"// NOTE: {msg}", file=f)
            print( "#include <linux/can.h>", file=f)
            print(f"#define NR_ITERATIONS ({NR_ITERATIONS})", file=f)
            print( "extern int symbolic_ready[NR_ITERATIONS];", file=f)
            print( "extern struct can_frame *symboic_can_frame; // struct can_frame [NR_ITERATIONS]", file=f)
            print( "#endif // CAN_DUMP_HEADER_H", file=f)
    
    def exportCBody(self, body_path, header_path):
        with open(body_path, "w") as f:
            NR_ITERATIONS = len(self.canFrames)
            print(f"#include \"{header_path}\"", file=f)
            print(f"int symbolic_ready[NR_ITERATIONS] = {{{', '.join(['1' for i in range(NR_ITERATIONS)])}}};", file=f)
            print(f"unsigned char symboic_can_frame_data[NR_ITERATIONS][{CAN_FRAME_SIZE}] = {{", file=f)
            for i, can_frame in enumerate(self.canFrames):
                print(f"\t{{{', '.join([hex(x) for x in can_frame])}}}, // frame {i}", file=f)
            print("};", file=f)
            print( "struct can_frame *symboic_can_frame = (struct can_frame *)(symboic_can_frame_data);", file=f)
    
    def exportCFiles(self, prefix: str, msg = ""):
        header = prefix + '.h'
        body = prefix + '.c'
        self.exportCHeader(header, msg)
        self.exportCBody(body, header)


parser = argparse.ArgumentParser(description="Extract CAN frames from strace files")
parser.add_argument("file", type=str, help="The strace file")
parser.add_argument("-o", "--output", type=str, help="The prefix of the output header/body files")
args = parser.parse_args()

e = CANFrameExtractor("socket(AF_CAN, SOCK_RAW, CAN_RAW)")
with open(args.file) as f:
    for line in f.read().splitlines():
        e.parseLine(line)

e.exportCFiles(args.output, f"parsed from {args.file}")
