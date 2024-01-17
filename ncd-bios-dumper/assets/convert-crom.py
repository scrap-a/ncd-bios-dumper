#!/usr/bin/env python3
#tiletool.pyのフォーマットを真似て書いていることを入れる

import os
import sys
import argparse

class converter(object):
    """Base converter functions"""
    def __init__(self, args):
        self.args = args
        self.cart_num = 0
        self.disc_num = 0

    def open_rom(self):
        """Init ROM I/O access"""
        pass

    def close_rom(self):
        """Close opened ROM"""
        self.fc1.close()
        self.fc2.close()
        self.fd.close()

    def validate(self):
        """Common input checks"""
        if self.cart_num != 2:
            sys.exit("error: expected two ROM files, given: %s" %
                     " ".join(self.args.input))
        if self.disc_num != 1:
            sys.exit("error: expected one ROM file, given: %s" %
                     " ".join(self.args.output))
        self.out = self.args.output[0]

    def convert(self):
        """Convert sprites"""
        pass

class cart2disc_converter(converter):
    """Specialization for convert sprite ROMs for MVS/AES to CD"""

    def __init__(self, args):
        self.args = args
        self.cart_num = 0
        self.disc_num = 0
        self.tile_size = 128
        self.tile_size_half = self.tile_size // 2

    def open_rom(self):
        """Init ROM I/O access"""
        self.fc1 = open(self.cart1 , 'rb')
        self.fc2 = open(self.cart2 , 'rb')
        self.fd  = open(self.disc  , 'wb')

    def validate(self):
        """Input checks"""
        self.cart_num = len(self.args.input)
        self.disc_num = len(self.args.output)

        super(cart2disc_converter, self).validate()

        self.cart1 = self.args.input[0]
        self.cart2 = self.args.input[1]
        self.disc  = self.args.output[0]

        if os.path.getsize(self.cart1) != os.path.getsize(self.cart2):
            sys.exit("error: both file sizes must be the same, given: %s" %
                     " ".join(self.args.input))

        if os.path.getsize(self.cart1) % self.tile_size_half != 0:
            sys.exit("error: file size must be a multiple of %d , given: %s" %
                     (self.tile_size_half," ".join(self.cart1)))

    def convert(self):
        """Convert Sprite ROMs, MVS/AES to CD"""
        self.open_rom()

        bin1=self.fc1.read(self.tile_size_half)
        bin2=self.fc2.read(self.tile_size_half)

        while bin1:
            bout = []
            for i in range(0,self.tile_size_half//2):
                bout.append(bin1[i*2+1])
                bout.append(bin1[i*2+0])
                bout.append(bin2[i*2+1])
                bout.append(bin2[i*2+0])
            self.fd.write(bytes(bout))
            bin1=self.fc1.read(self.tile_size_half)
            bin2=self.fc2.read(self.tile_size_half)

        self.close_rom()

class disc2cart_converter(converter):
    """Specialization for convert sprite ROMs for CD to MVS/AES"""

    def __init__(self, args):
        self.args = args
        self.cart_num = 0
        self.disc_num = 0
        self.tile_size = 128
        self.tile_size_half = self.tile_size // 2

    def open_rom(self):
        """Init ROM I/O access"""
        self.fd  = open(self.disc  , 'rb')
        self.fc1 = open(self.cart1 , 'wb')
        self.fc2 = open(self.cart2 , 'wb')

    def validate(self):
        """Input checks"""
        self.disc_num = len(self.args.input)
        self.cart_num = len(self.args.output)

        super(disc2cart_converter, self).validate()

        self.disc  = self.args.input[0]
        self.cart1 = self.args.output[0]
        self.cart2 = self.args.output[1]

        if os.path.getsize(self.disc) % self.tile_size != 0:
            sys.exit("error: file size must be a multiple of %d , given: %s" %
                     (self.tile_size," ".join(self.disc)))

    def convert(self):
        """Convert Sprite ROMs, CD to MVS/AES"""
        self.open_rom()

        bin=self.fd.read(self.tile_size)

        while bin:
            bout1 = []
            bout2 = []
            for i in range(0,self.tile_size_half//2):
                bout1.append(bin[i*4+1])
                bout1.append(bin[i*4+0])
                bout2.append(bin[i*4+3])
                bout2.append(bin[i*4+2])
            self.fc1.write(bytes(bout1))
            self.fc2.write(bytes(bout2))
            bin=self.fd.read(self.tile_size)

        self.close_rom()

def main():

    parser = argparse.ArgumentParser(
        description='Convert Sprite ROMs, MVS/AES to CD & CD to MVS/AES.')

    paction = parser.add_argument_group('action')
    pmode = paction.add_mutually_exclusive_group(required=True)
    pmode.add_argument('-d', '--disc', action='store_true', help='MVS/AES to CD')
    pmode.add_argument('-c', '--cart', action='store_true', help='CD to MVS/AES')
    
    parser.add_argument('-i', '--input',  nargs='+', help='name of input file')
    parser.add_argument('-o', '--output', nargs='+', help='name of output file')
    
    arguments = parser.parse_args()
    
    if arguments.disc:
        conv = cart2disc_converter(arguments)
    else:
        conv = disc2cart_converter(arguments)

    conv.validate()
    conv.convert()

if __name__ == '__main__':
    main()
