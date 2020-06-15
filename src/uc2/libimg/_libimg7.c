/* libimg - small module which provides binding to ImageMagick.
 *
 * Copyright (C) 2018 by Ihor E.Novikov
 *
 * 	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Affero General Public License
 *	as published by the Free Software Foundation, either version 3
 *	of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU Affero General Public License
 *	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Python.h>
#include <MagickWand/MagickWand.h>

static char* imgtype_to_str(ImageType img_type) {
	if (img_type == BilevelType){
		return "BilevelType";
	}
	else if (img_type == GrayscaleType){
		return "GrayscaleType";
	}
	else if (img_type == GrayscaleAlphaType){
		return "GrayscaleMatteType";
	}
	else if (img_type == PaletteType){
		return "PaletteType";
	}
	else if (img_type == PaletteAlphaType){
		return "PaletteMatteType";
	}
	else if (img_type == TrueColorType){
		return "TrueColorType";
	}
	else if (img_type == TrueColorAlphaType){
		return "TrueColorMatteType";
	}
	else if (img_type == ColorSeparationType){
		return "ColorSeparationType";
	}
	else if (img_type == ColorSeparationAlphaType){
		return "ColorSeparationMatteType";
	}
	else if (img_type == OptimizeType){
		return "OptimizeType";
	}
	else if (img_type == PaletteBilevelAlphaType){
		return "PaletteBilevelMatteType";
	}
	else {
		return "UndefinedType";
	}
}


static PyObject *
im_InitMagick(PyObject *self, PyObject *args) {

	MagickWandGenesis();

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
im_TerminateMagick(PyObject *self, PyObject *args) {

	MagickWandTerminus();

	Py_INCREF(Py_None);
	return Py_None;
}

static void im_FreeMagickWand(PyObject *obj) {
    MagickBooleanType status;
    MagickWand *magick_wand = (MagickWand*) PyCapsule_GetPointer(obj, "MagickWand");

    // In some cases python may uninitialize Magick before the wands are 
    // cleared, in that case don't destroy the wands, they are already gone.
    status = IsMagickWandInstantiated();

    if ((MagickTrue == status) && (NULL != magick_wand)) {
        ClearMagickWand(magick_wand);
        DestroyMagickWand(magick_wand);
    }
}

static PyObject *
im_NewImage(PyObject *self, PyObject *args) {

	MagickWand *magick_wand;
    PyObject *capsule;

	magick_wand = NewMagickWand();

    capsule = PyCapsule_New((void*) magick_wand, "MagickWand", im_FreeMagickWand);
    //capsule = PyCapsule_New((void*) magick_wand, "MagickWand", NULL);
	return capsule;
}

static PyObject *
im_LoadImage(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;
	char *filepath = NULL;
	MagickBooleanType status;

	if (!PyArg_ParseTuple(args, "Os:_libimg.load_image", &magick_pointer, &filepath)){
		return Py_BuildValue("i", 0);
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	status = MagickReadImage(magick_wand, filepath);

	if (status == MagickFalse){
		return Py_BuildValue("i", 0);
	}

	return Py_BuildValue("i", 1);
}

static PyObject *
im_LoadImageBlob(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;
	Py_ssize_t size;
	char *blob;
	MagickBooleanType status;

	if (!PyArg_ParseTuple(args, "Oy#:_libimg.load_image_blob", &magick_pointer, &blob, &size)){
		return Py_BuildValue("i", 0);
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");

	status = MagickReadImageBlob(magick_wand, blob, (size_t)size);

	if (status == MagickFalse){
		return Py_BuildValue("i", 0);
	}

	return Py_BuildValue("i", 1);
}


static PyObject *
im_MergeLayers(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand, *merged;
    PyObject *capsule;

	if (!PyArg_ParseTuple(args, "O:_libimg.merge_layers", &magick_pointer)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
    //MagickResetIterator(magick_wand);
    MagickSetFirstIterator(magick_wand);

    merged = MagickMergeImageLayers(magick_wand, MergeLayer);
    capsule = PyCapsule_New((void*) merged, "MagickWand", im_FreeMagickWand);
    //capsule = PyCapsule_New((void*) merged, "MagickWand", NULL);
	return capsule;
}

static PyObject *
im_WriteImage(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;
	char *filepath = NULL;
	MagickBooleanType status;

	if (!PyArg_ParseTuple(args, "Os:_libimg.write_image", &magick_pointer, &filepath)){
		return Py_BuildValue("i", 0);
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	status = MagickWriteImages(magick_wand, filepath, MagickTrue);

	if (status == MagickFalse){
		return Py_BuildValue("i", 0);
	}

	return Py_BuildValue("i", 1);
}

static PyObject *
im_GetImageBlob(PyObject *self, PyObject *args) {
//
	PyObject *magick_pointer;
	MagickWand *magick_wand;
	unsigned char *blob;
	size_t length;
	PyObject *ret;

	if (!PyArg_ParseTuple(args, "O:_libimg.get_image_blob", &magick_pointer)){
		return Py_BuildValue("i", 0);
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	blob = MagickGetImagesBlob(magick_wand, &length);
	ret = Py_BuildValue("y#", blob, length);
    //MagickRelinquishMemory(blob);

	return ret;
}

static PyObject *
im_GetNumberImages(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;

	if (!PyArg_ParseTuple(args, "O:_libimg.get_number_images", &magick_pointer)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");

	return Py_BuildValue("i", MagickGetNumberImages(magick_wand));
}

static PyObject *
im_ResetIterator(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;

	if (!PyArg_ParseTuple(args, "O:_libimg.reset_iterator", &magick_pointer)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
    //MagickResetIterator(magick_wand);
    MagickSetFirstIterator(magick_wand);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
im_NextImage(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;
	MagickBooleanType status;

	if (!PyArg_ParseTuple(args, "O:_libimg.next_image", &magick_pointer)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	status = MagickNextImage(magick_wand);

	if (status == MagickFalse){
		return Py_BuildValue("i", 0);
	}

	return Py_BuildValue("i", 1);
}


// Image types from MagickCore/image.h
//
//   UndefinedType,
//   BilevelType,
//   GrayscaleType,
//   GrayscaleAlphaType,
//   PaletteType,
//   PaletteAlphaType,
//   TrueColorType,
//   TrueColorAlphaType,
//   ColorSeparationType,
//   ColorSeparationAlphaType,
//   OptimizeType,
//   PaletteBilevelAlphaType

static PyObject *
im_GetImageType(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;
	ImageType img_type;

	if (!PyArg_ParseTuple(args, "O:_libimg.get_image_type", &magick_pointer)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	img_type = MagickGetImageType(magick_wand);
	return Py_BuildValue("s", imgtype_to_str(img_type));
}

// Colorspace types from magick/colorspace.h
//
// UndefinedColorspace
// CMYColorspace
// CMYKColorspace
// GRAYColorspace
// HCLColorspace
// HCLpColorspace
// HSBColorspace
// HSIColorspace
// HSLColorspace
// HSVColorspace
// HWBColorspace
// LabColorspace
// LCHColorspace
// LCHabColorspace
// LCHuvColorspace
// LogColorspace
// LMSColorspace
// LuvColorspace
// OHTAColorspace
// Rec601YCbCrColorspace
// Rec709YCbCrColorspace
// RGBColorspace
// scRGBColorspace
// sRGBColorspace
// TransparentColorspace
// xyYColorspace
// XYZColorspace
// YCbCrColorspace
// YCCColorspace
// YDbDrColorspace
// YIQColorspace
// YPbPrColorspace
// YUVColorspace
// LinearGRAYColorspace

static PyObject *
im_GetColorspace(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;
	ColorspaceType cs;

	if (!PyArg_ParseTuple(args, "O:_libimg.get_colorspace", &magick_pointer)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	cs = MagickGetImageColorspace(magick_wand);

	if (cs == RGBColorspace){
		return Py_BuildValue("s", "RGBColorspace");
	}
	else if (cs == GRAYColorspace){
		return Py_BuildValue("s", "GRAYColorspace");
	}
	else if (cs == TransparentColorspace){
		return Py_BuildValue("s", "TransparentColorspace");
	}
	else if (cs == OHTAColorspace){
		return Py_BuildValue("s", "OHTAColorspace");
	}
	else if (cs == LabColorspace){
		return Py_BuildValue("s", "LabColorspace");
	}
	else if (cs == XYZColorspace){
		return Py_BuildValue("s", "XYZColorspace");
	}
	else if (cs == YCbCrColorspace){
		return Py_BuildValue("s", "YCbCrColorspace");
	}
	else if (cs == YCCColorspace){
		return Py_BuildValue("s", "YCCColorspace");
	}
	else if (cs == YIQColorspace){
		return Py_BuildValue("s", "YIQColorspace");
	}
	else if (cs == YPbPrColorspace){
		return Py_BuildValue("s", "YPbPrColorspace");
	}
	else if (cs == YUVColorspace){
		return Py_BuildValue("s", "YUVColorspace");
	}
	else if (cs == CMYKColorspace){
		return Py_BuildValue("s", "CMYKColorspace");
	}
	else if (cs == sRGBColorspace){
		return Py_BuildValue("s", "sRGBColorspace");
	}
	else if (cs == HSBColorspace){
		return Py_BuildValue("s", "HSBColorspace");
	}
	else if (cs == HSLColorspace){
		return Py_BuildValue("s", "HSLColorspace");
	}
	else if (cs == HWBColorspace){
		return Py_BuildValue("s", "HWBColorspace");
	}
	else if (cs == Rec601YCbCrColorspace){
		return Py_BuildValue("s", "Rec601YCbCrColorspace");
	}
	else if (cs == Rec709YCbCrColorspace){
		return Py_BuildValue("s", "Rec709YCbCrColorspace");
	}
	else if (cs == LogColorspace){
		return Py_BuildValue("s", "LogColorspace");
	}
	else if (cs == CMYColorspace){
		return Py_BuildValue("s", "CMYColorspace");
	}
	else {
		return Py_BuildValue("s", "UndefinedColorspace");
	}
}

static PyObject *
im_CloneImage(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;
	MagickWand *wand_clone;
    PyObject *capsule;

	if (!PyArg_ParseTuple(args, "O:_libimg.clone_image", &magick_pointer)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	wand_clone = CloneMagickWand(magick_wand);

    capsule = PyCapsule_New((void*) wand_clone, "MagickWand", im_FreeMagickWand);
    //capsule = PyCapsule_New((void*) wand_clone, "MagickWand", NULL);
	return capsule;
}

static PyObject *
im_SetImageFormat(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;
	char *format = NULL;
	MagickBooleanType status;

	if (!PyArg_ParseTuple(args, "Os:_libimg.set_image_format", &magick_pointer, &format)){
		return Py_BuildValue("i", 0);
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	status = MagickSetImageFormat(magick_wand, format);

	if (status == MagickFalse){
		return Py_BuildValue("i", 0);
	}

	return Py_BuildValue("i", 1);
}


ImageType
get_image_type(char* mode) {

  if (strcmp(mode, "BilevelType") == 0) {
    return BilevelType;
  }
  else if (strcmp(mode, "GrayscaleType") == 0) {
    return GrayscaleType;
  }
  else if (strcmp(mode, "GrayscaleMatteType") == 0) {
    return GrayscaleAlphaType;
  }
  else if (strcmp(mode, "PaletteType") == 0) {
    return PaletteType;
  }
  else if (strcmp(mode, "PaletteMatteType") == 0) {
    return PaletteAlphaType;
  }
  else if (strcmp(mode, "TrueColorType") == 0) {
    return TrueColorType;
  }
  else if (strcmp(mode, "TrueColorMatteType") == 0) {
    return TrueColorAlphaType;
  }
  else if (strcmp(mode, "ColorSeparationType") == 0) {
    return ColorSeparationType;
  }
  else if (strcmp(mode, "ColorSeparationMatteType") == 0) {
    return ColorSeparationAlphaType;
  }
  else {
    return TrueColorType;
  }
}

static PyObject *
im_SetImageType(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;
	char *img_type = NULL;

	if (!PyArg_ParseTuple(args, "Os:_libimg.set_image_type", &magick_pointer, &img_type)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	MagickSetImageType(magick_wand, get_image_type(img_type));

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
im_RemoveAlpaChannel(PyObject *self, PyObject *args) {

	PyObject *magick_pointer;
	MagickWand *magick_wand;

	if (!PyArg_ParseTuple(args, "O:_libimg.remove_alpha_channel", &magick_pointer)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	magick_wand = (MagickWand *) PyCapsule_GetPointer(magick_pointer, "MagickWand");
	MagickSetImageAlphaChannel(magick_wand, DeactivateAlphaChannel);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
im_GetVersion(PyObject *self, PyObject *args) {

	size_t ver;
	PyObject *version_tuple;

	version_tuple = PyTuple_New(2);
	PyTuple_SetItem(version_tuple, 0, Py_BuildValue("s", GetMagickVersion(&ver)));
	PyTuple_SetItem(version_tuple, 1, Py_BuildValue("i", ver));
	return version_tuple;
}

static
PyMethodDef im_methods[] = {
		{"init_magick", im_InitMagick, METH_VARARGS},
		{"terminate_magick", im_TerminateMagick, METH_VARARGS},
		{"new_image", im_NewImage, METH_VARARGS},
		{"load_image", im_LoadImage, METH_VARARGS},
		{"load_image_blob", im_LoadImageBlob, METH_VARARGS},
		{"merge_layers", im_MergeLayers, METH_VARARGS},
		{"write_image", im_WriteImage, METH_VARARGS},
		{"get_image_blob", im_GetImageBlob, METH_VARARGS},
		{"get_number_images", im_GetNumberImages, METH_VARARGS},
		{"reset_iterator", im_ResetIterator, METH_VARARGS},
		{"next_image", im_NextImage, METH_VARARGS},
		{"get_image_type", im_GetImageType, METH_VARARGS},
		{"get_colorspace", im_GetColorspace, METH_VARARGS},
		{"clone_image", im_CloneImage, METH_VARARGS},
		{"set_image_format", im_SetImageFormat, METH_VARARGS},
		{"set_image_type", im_SetImageType, METH_VARARGS},
		{"remove_alpha_channel", im_RemoveAlpaChannel, METH_VARARGS},
		{"get_version", im_GetVersion, METH_VARARGS},

	{NULL, NULL}
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef imdef = {
    PyModuleDef_HEAD_INIT,
    "_libimg",           /* m_name */
    NULL,                /* m_doc */
    -1,                  /* m_size */
    im_methods,          /* m_methods */
};

PyMODINIT_FUNC
PyInit__libimg(void) {
    return PyModule_Create(&imdef);
}
#else
void init_libimg(void) {
    Py_InitModule("_libimg", im_methods);
}
#endif
