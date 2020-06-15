# -*- coding: utf-8 -*-
#
#       Copyright (C) 2012 by Ihor E. Novikov
#
#       This program is free software: you can redistribute it and/or modify
#       it under the terms of the GNU Affero General Public License
#       as published by the Free Software Foundation, either version 3
#       of the License, or (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU Affero General Public License
#       along with this program.  If not, see <https://www.gnu.org/licenses/>.

import unittest, os
from uc2.libimg import _libimg
try:
    from cStringIO import StringIO
except ModuleNotFoundError:
    from io import StringIO

_pkgdir = __path__[0]

class TestCoreLibImgFunctions(unittest.TestCase):

        def setUp(self):
                _libimg.init_magick()

        def tearDown(self):
                _libimg.terminate_magick()

        def test01_create_new_image(self):
                img = None
                img = _libimg.new_image()
                self.assertNotEqual(None, img)

        def test02_load_image(self):
                filepath = os.path.join(_pkgdir, 'img_data', 'white_100x100.png')
                wand = _libimg.new_image()
                self.assertEqual(1, _libimg.load_image(wand, filepath))

        def test03_load_nonexist_image(self):
                wrong_filepath = os.path.join(_pkgdir, 'img_data', 'white_100.png')
                wand = _libimg.new_image()
                self.assertEqual(0, _libimg.load_image(wand, wrong_filepath))

        def test04_load_wrong_image(self):
                filepath = os.path.join(_pkgdir, 'img_data', 'wrong_imape.png')
                wand = _libimg.new_image()
                self.assertEqual(0, _libimg.load_image(wand, filepath))

        def test05_write_image(self):
                filepath = os.path.join(_pkgdir, 'img_data', 'white_100x100.png')

                names = []
                names.append(os.path.join(_pkgdir, 'img_data', 'white.png'))
                names.append(os.path.join(_pkgdir, 'img_data', 'white.tif'))
                names.append(os.path.join(_pkgdir, 'img_data', 'white.jpg'))

                wand = _libimg.new_image()
                result = _libimg.load_image(wand, filepath)
                self.assertEqual(1, result)

                for name in names:
                        result = _libimg.write_image(wand, name)
                        self.assertEqual(1, result)
                        if os.path.isfile(name):
                                os.remove(name)
                        else:
                                self.fail()

        def test06_number_images(self):
                names = []
                names.append(os.path.join(_pkgdir, 'img_data', '1layer.gif'))
                names.append(os.path.join(_pkgdir, 'img_data', '2layer.gif'))
                names.append(os.path.join(_pkgdir, 'img_data', '3layer.gif'))

                for name in names:
                        wand = _libimg.new_image()
                        result = _libimg.load_image(wand, name)
                        self.assertEqual(1, result)
                        num = _libimg.get_number_images(wand)
                        self.assertEqual(names.index(name) + 1, num)

        def test07_image_type(self):
                names = [
                        ['type_bilevel.png', 'BilevelType'],
                        ['type_grayscale.png', 'GrayscaleType'],
                        ['type_palette.png', 'PaletteType'],
                        ['type_palettematte.png', 'PaletteMatteType'],
                        ['type_truecolor.png', 'TrueColorType'],
                        ['type_truecolormatte.png', 'TrueColorMatteType'],
                        ['type_cmyk.tif', 'ColorSeparationType'],
                        ['type_cmyka.tif', 'ColorSeparationMatteType'],
                ]

                wand = _libimg.new_image()
                for name in names:
                        print(name)
                        path = os.path.join(_pkgdir, 'img_data', name[0])
                        print(name[0])
                        result = _libimg.load_image(wand, path)
                        self.assertEqual(1, result)

                _libimg.reset_iterator(wand)
                for name in names:
                        print(name[0])
                        self.assertEqual(name[1], _libimg.get_image_type(wand))
                        _libimg.next_image(wand)

                #for name in names:
                #        wand = _libimg.new_image()
                #        path = os.path.join(_pkgdir, 'img_data', name[0])
                #        result = _libimg.load_image(wand, path)
                #        self.assertEqual(1, result)
                #        _libimg.reset_iterator(wand)
                #        _libimg.next_image(wand)
                #        self.assertEqual(name[1], _libimg.get_image_type(wand))

        def test08_blob_reading(self):
                path = os.path.join(_pkgdir, 'img_data', 'type_palettematte.png')
                blob = open(path, 'rb').read()
                wand = _libimg.new_image()
                result = _libimg.load_image_blob(wand, blob)
                self.assertEqual(1, result)
                blob=_libimg.get_image_blob(wand)
                self.assertEqual(False, blob=='')



