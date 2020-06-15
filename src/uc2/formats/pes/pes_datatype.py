# -*- coding: utf-8 -*-
#
#  Copyright (C) 2009-2019 by Maxim S. Barabash
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License
#  as published by the Free Software Foundation, either version 3
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

import struct
from uc2.formats.pes import pes_const

packer_b = struct.Struct("B")
packer_b2 = struct.Struct("BB")


def unpack_stitch(data):
    d1, d2 = packer_b2.unpack(data[:2])
    print(d1, d2)

    # x = decode_x(d1, d2, d3)
    # y = decode_y(d1, d2, d3)
    # cmd = decode_command(d3)
    x = 0
    y = 0
    cmd = 0
    return x, y, cmd


def unpack_uint24le(b):
    b = bytearray(b)
    return (b[0] & 0xFF) + ((b[1] & 0xFF) << 8) + ((b[2] & 0xFF) << 16)
