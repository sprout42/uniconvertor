/* cms - small module which provides binding to LittleCMS library.
 *
 * Copyright (C) 2009-2012 by Ihor E.Novikov
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
#include <lcms.h>
#include "Imaging.h"

/* redefine the ImagingObject struct defined in _imagingmodule.c */
typedef struct {
    PyObject_HEAD
    Imaging image;
} ImagingObject;

DWORD
getLCMStype (char* mode) {

  if (strcmp(mode, "RGB") == 0) {
    return TYPE_RGBA_8;
  }
  else if (strcmp(mode, "RGBA") == 0) {
    return TYPE_RGBA_8;
  }
  else if (strcmp(mode, "RGBX") == 0) {
    return TYPE_RGBA_8;
  }
  else if (strcmp(mode, "RGBA;16B") == 0) {
    return TYPE_RGBA_16;
  }
  else if (strcmp(mode, "CMYK") == 0) {
    return TYPE_CMYK_8;
  }
  else if (strcmp(mode, "L") == 0) {
    return TYPE_GRAY_8;
  }
  else if (strcmp(mode, "L;16") == 0) {
    return TYPE_GRAY_16;
  }
  else if (strcmp(mode, "L;16B") == 0) {
    return TYPE_GRAY_16_SE;
  }
  else if (strcmp(mode, "YCCA") == 0) {
    return TYPE_YCbCr_8;
  }
  else if (strcmp(mode, "YCC") == 0) {
    return TYPE_YCbCr_8;
  }
  else if (strcmp(mode, "LAB") == 0) {
    return TYPE_Lab_8;
  }

  else {
    return TYPE_GRAY_8;
  }
}

static void pycms_FreeProfile(PyObject *obj) {
	cmsHPROFILE hProfile;

    hProfile = (cmsHPROFILE) PyCapsule_GetPointer(obj, "cmsHPROFILE");
    cmsCloseProfile(hProfile);
}

static void pycms_FreeTransform(PyObject *obj) {
	cmsHTRANSFORM hTransform;

    hTransform = (cmsHTRANSFORM) PyCapsule_GetPointer(obj, "cmsHTRANSFORM");
    cmsDeleteTransform(hTransform);
}

static void pycms_Free(PyObject *obj) {
	void *pixbuf;

    pixbuf = (cmsHTRANSFORM) PyCapsule_GetPointer(obj, "pixbuf");
    free(pixbuf);
}

static PyObject *
pycms_OpenProfile(PyObject *self, PyObject *args) {

	char *profile = NULL;
	cmsHPROFILE hProfile;
    PyObject *capsule;

	if (!PyArg_ParseTuple(args, "s:_cms.openProfile", &profile)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsErrorAction(LCMS_ERROR_IGNORE);

	hProfile = cmsOpenProfileFromFile(profile, "r");

	if(hProfile==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

    capsule = PyCapsule_New((void *) hProfile, "cmsHPROFILE", pycms_FreeProfile);
	return capsule;
}

static PyObject *
pycms_OpenProfileFromString(PyObject *self, PyObject *args) {

	long size;
	char *profile;
	cmsHPROFILE hProfile;
    PyObject *capsule;

	if (!PyArg_ParseTuple(args, "s#:_cms.openProfileFromString", &profile, &size)){
		Py_INCREF(Py_None);
		return Py_None;
	}

	hProfile = 	cmsOpenProfileFromMem(profile, size);

	if(hProfile==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

    capsule = PyCapsule_New((void *) hProfile, "cmsHPROFILE", pycms_FreeProfile);
	return capsule;
}

static PyObject *
pycms_CreateRGBProfile(PyObject *self, PyObject *args) {

	cmsHPROFILE hProfile;
    PyObject *capsule;

	cmsErrorAction(LCMS_ERROR_IGNORE);

	hProfile = cmsCreate_sRGBProfile();

	if(hProfile==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

    capsule = PyCapsule_New((void *) hProfile, "cmsHPROFILE", pycms_FreeProfile);
	return capsule;
}

static PyObject *
pycms_CreateLabProfile(PyObject *self, PyObject *args) {

	cmsHPROFILE hProfile;
    PyObject *capsule;

	cmsErrorAction(LCMS_ERROR_IGNORE);

	hProfile = cmsCreateLabProfile(NULL);

	if(hProfile==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

    capsule = PyCapsule_New((void *) hProfile, "cmsHPROFILE", pycms_FreeProfile);
	return capsule;
}

static PyObject *
pycms_CreateGrayProfile(PyObject *self, PyObject *args) {

	cmsHPROFILE hProfile;
	LPGAMMATABLE gamma;
    PyObject *capsule;

	cmsErrorAction(LCMS_ERROR_IGNORE);

	gamma = cmsBuildGamma(256, 2.2);
	hProfile = cmsCreateGrayProfile(cmsD50_xyY(), gamma);
	cmsFreeGamma(gamma);

	if(hProfile==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

    capsule = PyCapsule_New((void *) hProfile, "cmsHPROFILE", pycms_FreeProfile);
	return capsule;
}

static PyObject *
pycms_BuildTransform (PyObject *self, PyObject *args) {

	char *inMode;
	char *outMode;
	int renderingIntent;
	int inFlags;
	DWORD flags;
	PyObject *inputProfile;
	PyObject *outputProfile;
	cmsHPROFILE hInputProfile, hOutputProfile;
	cmsHTRANSFORM hTransform;
    PyObject *capsule;

	if (!PyArg_ParseTuple(args, "OsOsii:_cms.buildTransform",
                          &inputProfile, &inMode, &outputProfile,
                          &outMode, &renderingIntent, &inFlags)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsErrorAction(LCMS_ERROR_IGNORE);

	hInputProfile = (cmsHPROFILE) PyCapsule_GetPointer(inputProfile, "cmsHPROFILE");
	hOutputProfile = (cmsHPROFILE) PyCapsule_GetPointer(outputProfile, "cmsHPROFILE");
	flags = (DWORD) inFlags;

	hTransform = cmsCreateTransform(hInputProfile, getLCMStype(inMode),
			hOutputProfile, getLCMStype(outMode), renderingIntent, flags);

	if(hTransform==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

    capsule = PyCapsule_New((void *) hTransform, "cmsHTRANSFORM", pycms_FreeTransform);
	return capsule;
}

static PyObject *
pycms_BuildProofingTransform (PyObject *self, PyObject *args) {

	char *inMode;
	char *outMode;
	int renderingIntent;
	int proofingIntent;
	int inFlags;
	DWORD flags;
	PyObject *inputProfile;
	PyObject *outputProfile;
	PyObject *proofingProfile;
    PyObject *capsule;

	cmsHPROFILE hInputProfile, hOutputProfile, hProofingProfile;
	cmsHTRANSFORM hTransform;

	if (!PyArg_ParseTuple(args, "OsOsOiii:_cms.buildProofingTransform",
                          &inputProfile, &inMode, &outputProfile, &outMode,
			&proofingProfile, &renderingIntent, &proofingIntent, &inFlags)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsErrorAction(LCMS_ERROR_IGNORE);

	hInputProfile = (cmsHPROFILE) PyCapsule_GetPointer(inputProfile, "cmsHPROFILE");
	hOutputProfile = (cmsHPROFILE) PyCapsule_GetPointer(outputProfile, "cmsHPROFILE");
	hProofingProfile = (cmsHPROFILE) PyCapsule_GetPointer(proofingProfile, "cmsHPROFILE");
	flags = (DWORD) inFlags;

	hTransform = cmsCreateProofingTransform(hInputProfile, getLCMStype(inMode),
			hOutputProfile, getLCMStype(outMode), hProofingProfile, renderingIntent, proofingIntent, flags);

	if(hTransform==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

    capsule = PyCapsule_New((void *) hTransform, "cmsHTRANSFORM", pycms_FreeTransform);
	return capsule;
}

static PyObject *
pycms_SetAlarmCodes (PyObject *self, PyObject *args) {

	int red, green, blue;

	if (!PyArg_ParseTuple(args, "iii:_cms.setAlarmCodes", &red, &green, &blue)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsSetAlarmCodes(red, green, blue);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pycms_TransformPixel (PyObject *self, PyObject *args) {

	unsigned char *inbuf;
	int channel1,channel2,channel3,channel4;
	PyObject *transform;
	cmsHTRANSFORM hTransform;
	PyObject *result;

	if (!PyArg_ParseTuple(args, "Oiiii:_cms.transformPixel", &transform, &channel1, &channel2, &channel3, &channel4)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsErrorAction(LCMS_ERROR_IGNORE);

	inbuf=malloc(4);
	inbuf[0]=(unsigned char)channel1;
	inbuf[1]=(unsigned char)channel2;
	inbuf[2]=(unsigned char)channel3;
	inbuf[3]=(unsigned char)channel4;

	hTransform = (cmsHTRANSFORM) PyCapsule_GetPointer(transform, "cmsHTRANSFORM");

	cmsDoTransform(hTransform, inbuf, inbuf, 1);

	result = Py_BuildValue("[iiii]", inbuf[0], inbuf[1], inbuf[2], inbuf[3]);
	free(inbuf);
	return result;
}


static PyObject *
pycms_TransformPixel2 (PyObject *self, PyObject *args) {

	double channel1,channel2,channel3,channel4;
	unsigned char *inbuf;
	PyObject *transform;
	cmsHTRANSFORM hTransform;
	PyObject *result;

	if (!PyArg_ParseTuple(args, "Odddd:_cms.transformPixel2", &transform, &channel1, &channel2, &channel3, &channel4)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsErrorAction(LCMS_ERROR_IGNORE);

	inbuf=malloc(4);
	inbuf[0]=(unsigned char)(channel1*255);
	inbuf[1]=(unsigned char)(channel2*255);
	inbuf[2]=(unsigned char)(channel3*255);
	inbuf[3]=(unsigned char)(channel4*255);

	hTransform = (cmsHTRANSFORM) PyCapsule_GetPointer(transform, "cmsHTRANSFORM");

	cmsDoTransform(hTransform, inbuf, inbuf, 1);

	result = Py_BuildValue("(dddd)", (double)inbuf[0]/255, (double)inbuf[1]/255,
			(double)inbuf[2]/255, (double)inbuf[3]/255);

	free(inbuf);
	return result;
}

static PyObject *
pycms_TransformBitmap (PyObject *self, PyObject *args) {

	ImagingObject* inImage;
	ImagingObject* outImage;
	Imaging inImg, outImg;
	PyObject *transform;
	cmsHTRANSFORM hTransform;
	int width, height, i;

	if (!PyArg_ParseTuple(args, "OOOii:_cms.transformBitmap", &transform, &inImage, &outImage, &width, &height)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsErrorAction(LCMS_ERROR_IGNORE);

	inImg=inImage->image;
	outImg=outImage->image;

	hTransform = (cmsHTRANSFORM) PyCapsule_GetPointer(transform, "cmsHTRANSFORM");

	for (i = 0; i < height; i++) {
		cmsDoTransform(hTransform, inImg->image[i],	outImg->image[i], width);
	}

	Py_INCREF(Py_None);
	return Py_None;
}

#define BUFFER_SIZE 1000

static PyObject *
pycms_GetProfileName (PyObject *self, PyObject *args) {

	PyObject *profile;
	cmsHPROFILE hProfile;

	if (!PyArg_ParseTuple(args, "O:_cms.getProfileName", &profile)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsErrorAction(LCMS_ERROR_IGNORE);

	hProfile = (cmsHPROFILE) PyCapsule_GetPointer(profile, "cmsHPROFILE");

	return Py_BuildValue("s", cmsTakeProductDesc(hProfile));
}

static PyObject *
pycms_GetProfileInfo (PyObject *self, PyObject *args) {

	PyObject *profile;
	cmsHPROFILE hProfile;

	if (!PyArg_ParseTuple(args, "O:_cms.getProfileInfo", &profile)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsErrorAction(LCMS_ERROR_IGNORE);

	hProfile = (cmsHPROFILE) PyCapsule_GetPointer(profile, "cmsHPROFILE");

	return Py_BuildValue("s", cmsTakeProductName(hProfile));
}

static PyObject *
pycms_GetProfileInfoCopyright (PyObject *self, PyObject *args) {

	void *profile;
	cmsHPROFILE hProfile;
	char *buffer;
	PyObject *ret;

	if (!PyArg_ParseTuple(args, "O:_cms.getProfileInfoCopyright", &profile)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	ret=Py_BuildValue("s", "");
	return ret;
}

static PyObject *
pycms_GetPixelsFromImage (PyObject *self, PyObject *args) {

	int width, height, bytes_per_pixel, i;
	unsigned char *pixbuf;
	ImagingObject* inImage;
	Imaging inImg;
    PyObject *capsule;

	if (!PyArg_ParseTuple(args, "Oiii:_cms.getPixelsFromImage", &inImage, &width, &height, &bytes_per_pixel)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	pixbuf=malloc(width*height*bytes_per_pixel);
	inImg=inImage->image;

	for (i = 0; i < height; i++) {
		memcpy(&pixbuf[i*width*bytes_per_pixel], inImg->image[i], width*bytes_per_pixel);
	}

    capsule = PyCapsule_New((void *) pixbuf, "pixbuf", pycms_Free);
	return capsule;
}

static PyObject *
pycms_SetImagePixels (PyObject *self, PyObject *args) {

	int width, height, bytes_per_pixel, i;
	PyObject *pixels;
	unsigned char *pixbuf;
	ImagingObject* inImage;
	Imaging inImg;

	if (!PyArg_ParseTuple(args, "OOiii:_cms.setImagePixels", &pixels, &inImage, &width, &height, &bytes_per_pixel)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	pixbuf = (unsigned char *) PyCapsule_GetPointer(pixels, "pixbuf");
	inImg=inImage->image;

	for (i = 0; i < height; i++) {
		memcpy(inImg->image[i], &pixbuf[i*width*bytes_per_pixel], width*bytes_per_pixel);
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pycms_TransformPixels (PyObject *self, PyObject *args) {

	int width;
	unsigned char *pixbuf;
	unsigned char *result;
	PyObject *pixels;
	PyObject *transform;
	cmsHTRANSFORM hTransform;
    PyObject *capsule;

	if (!PyArg_ParseTuple(args, "OOi:_cms.transformPixels", &transform, &pixels, &width)) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	cmsErrorAction(LCMS_ERROR_IGNORE);

	hTransform = (cmsHTRANSFORM) PyCapsule_GetPointer(transform, "cmsHTRANSFORM");
	pixbuf = (unsigned char *) PyCapsule_GetPointer(pixels, "pixbuf");
	result=malloc(width*4);

	cmsDoTransform(hTransform, pixbuf, result, width);

    capsule = PyCapsule_New((void *) result, "pixbuf", pycms_Free);
	return capsule;
}

static PyObject *
pycms_GetVersion (PyObject *self, PyObject *args) {
	return Py_BuildValue("i:_cms.getVersion",  LCMS_VERSION);
}

static
PyMethodDef pycms_methods[] = {
	{"getVersion", pycms_GetVersion, METH_VARARGS},
	{"openProfile", pycms_OpenProfile, METH_VARARGS},
	{"openProfileFromString", pycms_OpenProfileFromString, METH_VARARGS},
	{"createRGBProfile", pycms_CreateRGBProfile, METH_VARARGS},
	{"createLabProfile", pycms_CreateLabProfile, METH_VARARGS},
	{"createGrayProfile", pycms_CreateGrayProfile, METH_VARARGS},
	{"buildTransform", pycms_BuildTransform, METH_VARARGS},
	{"buildProofingTransform", pycms_BuildProofingTransform, METH_VARARGS},
	{"setAlarmCodes", pycms_SetAlarmCodes, METH_VARARGS},
	{"transformPixel", pycms_TransformPixel, METH_VARARGS},
	{"transformPixel2", pycms_TransformPixel2, METH_VARARGS},
	{"transformBitmap", pycms_TransformBitmap, METH_VARARGS},
	{"getProfileName", pycms_GetProfileName, METH_VARARGS},
	{"getProfileInfo", pycms_GetProfileInfo, METH_VARARGS},
	{"getProfileInfoCopyright", pycms_GetProfileInfoCopyright, METH_VARARGS},
	{"getPixelsFromImage", pycms_GetPixelsFromImage, METH_VARARGS},
	{"setImagePixels", pycms_SetImagePixels, METH_VARARGS},
	{"transformPixels", pycms_TransformPixels, METH_VARARGS},
	{NULL, NULL}
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef cmsdef = {
    PyModuleDef_HEAD_INIT,
    "_cms",              /* m_name */
    NULL,                /* m_doc */
    -1,                  /* m_size */
    pycms_methods,       /* m_methods */
};

PyMODINIT_FUNC
PyInit__cms(void) {
    return PyModule_Create(&cmsdef);
}
#else
void
init_cms(void)
{
    Py_InitModule("_cms", pycms_methods);
}
#endif
