/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <PyPieAttributes.h>
#include <ObserverToCallback.h>
#include <stdio.h>
#include <snprintf.h>

// ****************************************************************************
// Module: PyPieAttributes
//
// Purpose: 
//   Attributes for the PiePlot plot.
//
// Note:       Autogenerated by xml2python. Do not modify by hand!
//
// Programmer: xml2python
// Creation:   omitted
//
// ****************************************************************************

//
// This struct contains the Python type information and a PieAttributes.
//
struct PieAttributesObject
{
    PyObject_HEAD
    PieAttributes *data;
    bool        owns;
    PyObject   *parent;
};

//
// Internal prototypes
//
static PyObject *NewPieAttributes(int);

std::string
PyPieAttributes_ToString(const PieAttributes *atts, const char *prefix)
{
    std::string str; 
    char tmpStr[1000]; 

    const char *iGlyphStyle_names = "STYLE_PIE, STYLE_STAR, STYLE_BARS";
    switch (atts->GetIGlyphStyle())
    {
      case PieAttributes::STYLE_PIE:
          SNPRINTF(tmpStr, 1000, "%siGlyphStyle = %sSTYLE_PIE  # %s\n", prefix, prefix, iGlyphStyle_names);
          str += tmpStr;
          break;
      case PieAttributes::STYLE_STAR:
          SNPRINTF(tmpStr, 1000, "%siGlyphStyle = %sSTYLE_STAR  # %s\n", prefix, prefix, iGlyphStyle_names);
          str += tmpStr;
          break;
      case PieAttributes::STYLE_BARS:
          SNPRINTF(tmpStr, 1000, "%siGlyphStyle = %sSTYLE_BARS  # %s\n", prefix, prefix, iGlyphStyle_names);
          str += tmpStr;
          break;
      default:
          break;
    }

    SNPRINTF(tmpStr, 1000, "%sfScale1 = %g\n", prefix, atts->GetFScale1());
    str += tmpStr;
    SNPRINTF(tmpStr, 1000, "%sfScale2 = %g\n", prefix, atts->GetFScale2());
    str += tmpStr;
    if(atts->GetBFramed())
        SNPRINTF(tmpStr, 1000, "%sbFramed = 1\n", prefix);
    else
        SNPRINTF(tmpStr, 1000, "%sbFramed = 0\n", prefix);
    str += tmpStr;
    return str;
}

static PyObject *
PieAttributes_Notify(PyObject *self, PyObject *args)
{
    PieAttributesObject *obj = (PieAttributesObject *)self;
    obj->data->Notify();
    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
PieAttributes_SetIGlyphStyle(PyObject *self, PyObject *args)
{
    PieAttributesObject *obj = (PieAttributesObject *)self;

    int ival;
    if(!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    // Set the iGlyphStyle in the object.
    if(ival >= 0 && ival < 3)
        obj->data->SetIGlyphStyle(PieAttributes::GlyphStyle(ival));
    else
    {
        fprintf(stderr, "An invalid iGlyphStyle value was given. "
                        "Valid values are in the range of [0,2]. "
                        "You can also use the following names: "
                        "STYLE_PIE, STYLE_STAR, STYLE_BARS.");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
PieAttributes_GetIGlyphStyle(PyObject *self, PyObject *args)
{
    PieAttributesObject *obj = (PieAttributesObject *)self;
    PyObject *retval = PyInt_FromLong(long(obj->data->GetIGlyphStyle()));
    return retval;
}

/*static*/ PyObject *
PieAttributes_SetFScale1(PyObject *self, PyObject *args)
{
    PieAttributesObject *obj = (PieAttributesObject *)self;

    float fval;
    if(!PyArg_ParseTuple(args, "f", &fval))
        return NULL;

    // Set the fScale1 in the object.
    obj->data->SetFScale1(fval);

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
PieAttributes_GetFScale1(PyObject *self, PyObject *args)
{
    PieAttributesObject *obj = (PieAttributesObject *)self;
    PyObject *retval = PyFloat_FromDouble(double(obj->data->GetFScale1()));
    return retval;
}

/*static*/ PyObject *
PieAttributes_SetFScale2(PyObject *self, PyObject *args)
{
    PieAttributesObject *obj = (PieAttributesObject *)self;

    float fval;
    if(!PyArg_ParseTuple(args, "f", &fval))
        return NULL;

    // Set the fScale2 in the object.
    obj->data->SetFScale2(fval);

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
PieAttributes_GetFScale2(PyObject *self, PyObject *args)
{
    PieAttributesObject *obj = (PieAttributesObject *)self;
    PyObject *retval = PyFloat_FromDouble(double(obj->data->GetFScale2()));
    return retval;
}

/*static*/ PyObject *
PieAttributes_SetBFramed(PyObject *self, PyObject *args)
{
    PieAttributesObject *obj = (PieAttributesObject *)self;

    int ival;
    if(!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    // Set the bFramed in the object.
    obj->data->SetBFramed(ival != 0);

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
PieAttributes_GetBFramed(PyObject *self, PyObject *args)
{
    PieAttributesObject *obj = (PieAttributesObject *)self;
    PyObject *retval = PyInt_FromLong(obj->data->GetBFramed()?1L:0L);
    return retval;
}



PyMethodDef PyPieAttributes_methods[PIEATTRIBUTES_NMETH] = {
    {"Notify", PieAttributes_Notify, METH_VARARGS},
    {"SetIGlyphStyle", PieAttributes_SetIGlyphStyle, METH_VARARGS},
    {"GetIGlyphStyle", PieAttributes_GetIGlyphStyle, METH_VARARGS},
    {"SetFScale1", PieAttributes_SetFScale1, METH_VARARGS},
    {"GetFScale1", PieAttributes_GetFScale1, METH_VARARGS},
    {"SetFScale2", PieAttributes_SetFScale2, METH_VARARGS},
    {"GetFScale2", PieAttributes_GetFScale2, METH_VARARGS},
    {"SetBFramed", PieAttributes_SetBFramed, METH_VARARGS},
    {"GetBFramed", PieAttributes_GetBFramed, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//

static void
PieAttributes_dealloc(PyObject *v)
{
   PieAttributesObject *obj = (PieAttributesObject *)v;
   if(obj->parent != 0)
       Py_DECREF(obj->parent);
   if(obj->owns)
       delete obj->data;
}

static int
PieAttributes_compare(PyObject *v, PyObject *w)
{
    PieAttributes *a = ((PieAttributesObject *)v)->data;
    PieAttributes *b = ((PieAttributesObject *)w)->data;
    return (*a == *b) ? 0 : -1;
}

PyObject *
PyPieAttributes_getattr(PyObject *self, char *name)
{
    if(strcmp(name, "iGlyphStyle") == 0)
        return PieAttributes_GetIGlyphStyle(self, NULL);
    if(strcmp(name, "STYLE_PIE") == 0)
        return PyInt_FromLong(long(PieAttributes::STYLE_PIE));
    if(strcmp(name, "STYLE_STAR") == 0)
        return PyInt_FromLong(long(PieAttributes::STYLE_STAR));
    if(strcmp(name, "STYLE_BARS") == 0)
        return PyInt_FromLong(long(PieAttributes::STYLE_BARS));

    if(strcmp(name, "fScale1") == 0)
        return PieAttributes_GetFScale1(self, NULL);
    if(strcmp(name, "fScale2") == 0)
        return PieAttributes_GetFScale2(self, NULL);
    if(strcmp(name, "bFramed") == 0)
        return PieAttributes_GetBFramed(self, NULL);

    return Py_FindMethod(PyPieAttributes_methods, self, name);
}

int
PyPieAttributes_setattr(PyObject *self, char *name, PyObject *args)
{
    // Create a tuple to contain the arguments since all of the Set
    // functions expect a tuple.
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, args);
    Py_INCREF(args);
    PyObject *obj = NULL;

    if(strcmp(name, "iGlyphStyle") == 0)
        obj = PieAttributes_SetIGlyphStyle(self, tuple);
    else if(strcmp(name, "fScale1") == 0)
        obj = PieAttributes_SetFScale1(self, tuple);
    else if(strcmp(name, "fScale2") == 0)
        obj = PieAttributes_SetFScale2(self, tuple);
    else if(strcmp(name, "bFramed") == 0)
        obj = PieAttributes_SetBFramed(self, tuple);

    if(obj != NULL)
        Py_DECREF(obj);

    Py_DECREF(tuple);
    if( obj == NULL)
        PyErr_Format(PyExc_RuntimeError, "Unable to set unknown attribute: '%s'", name);
    return (obj != NULL) ? 0 : -1;
}

static int
PieAttributes_print(PyObject *v, FILE *fp, int flags)
{
    PieAttributesObject *obj = (PieAttributesObject *)v;
    fprintf(fp, "%s", PyPieAttributes_ToString(obj->data, "").c_str());
    return 0;
}

PyObject *
PieAttributes_str(PyObject *v)
{
    PieAttributesObject *obj = (PieAttributesObject *)v;
    return PyString_FromString(PyPieAttributes_ToString(obj->data,"").c_str());
}

//
// The doc string for the class.
//
#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *PieAttributes_Purpose = "Attributes for the PiePlot plot.";
#else
static char *PieAttributes_Purpose = "Attributes for the PiePlot plot.";
#endif

//
// The type description structure
//
static PyTypeObject PieAttributesType =
{
    //
    // Type header
    //
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                   // ob_size
    "PieAttributes",                    // tp_name
    sizeof(PieAttributesObject),        // tp_basicsize
    0,                                   // tp_itemsize
    //
    // Standard methods
    //
    (destructor)PieAttributes_dealloc,  // tp_dealloc
    (printfunc)PieAttributes_print,     // tp_print
    (getattrfunc)PyPieAttributes_getattr, // tp_getattr
    (setattrfunc)PyPieAttributes_setattr, // tp_setattr
    (cmpfunc)PieAttributes_compare,     // tp_compare
    (reprfunc)0,                         // tp_repr
    //
    // Type categories
    //
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    //
    // More methods
    //
    0,                                   // tp_hash
    0,                                   // tp_call
    (reprfunc)PieAttributes_str,        // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_CHECKTYPES,               // tp_flags
    PieAttributes_Purpose,              // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0                                    // tp_weaklistoffset
};

//
// Helper functions for object allocation.
//

static PieAttributes *defaultAtts = 0;
static PieAttributes *currentAtts = 0;

static PyObject *
NewPieAttributes(int useCurrent)
{
    PieAttributesObject *newObject;
    newObject = PyObject_NEW(PieAttributesObject, &PieAttributesType);
    if(newObject == NULL)
        return NULL;
    if(useCurrent && currentAtts != 0)
        newObject->data = new PieAttributes(*currentAtts);
    else if(defaultAtts != 0)
        newObject->data = new PieAttributes(*defaultAtts);
    else
        newObject->data = new PieAttributes;
    newObject->owns = true;
    newObject->parent = 0;
    return (PyObject *)newObject;
}

static PyObject *
WrapPieAttributes(const PieAttributes *attr)
{
    PieAttributesObject *newObject;
    newObject = PyObject_NEW(PieAttributesObject, &PieAttributesType);
    if(newObject == NULL)
        return NULL;
    newObject->data = (PieAttributes *)attr;
    newObject->owns = false;
    newObject->parent = 0;
    return (PyObject *)newObject;
}

///////////////////////////////////////////////////////////////////////////////
//
// Interface that is exposed to the VisIt module.
//
///////////////////////////////////////////////////////////////////////////////

PyObject *
PieAttributes_new(PyObject *self, PyObject *args)
{
    int useCurrent = 0;
    if (!PyArg_ParseTuple(args, "i", &useCurrent))
    {
        if (!PyArg_ParseTuple(args, ""))
            return NULL;
        else
            PyErr_Clear();
    }

    return (PyObject *)NewPieAttributes(useCurrent);
}

//
// Plugin method table. These methods are added to the visitmodule's methods.
//
static PyMethodDef PieAttributesMethods[] = {
    {"PieAttributes", PieAttributes_new, METH_VARARGS},
    {NULL,      NULL}        /* Sentinel */
};

static Observer *PieAttributesObserver = 0;

std::string
PyPieAttributes_GetLogString()
{
    std::string s("PieAtts = PieAttributes()\n");
    if(currentAtts != 0)
        s += PyPieAttributes_ToString(currentAtts, "PieAtts.");
    return s;
}

static void
PyPieAttributes_CallLogRoutine(Subject *subj, void *data)
{
    PieAttributes *atts = (PieAttributes *)subj;
    typedef void (*logCallback)(const std::string &);
    logCallback cb = (logCallback)data;

    if(cb != 0)
    {
        std::string s("PieAtts = PieAttributes()\n");
        s += PyPieAttributes_ToString(currentAtts, "PieAtts.");
        cb(s);
    }
}

void
PyPieAttributes_StartUp(PieAttributes *subj, void *data)
{
    if(subj == 0)
        return;

    currentAtts = subj;
    PyPieAttributes_SetDefaults(subj);

    //
    // Create the observer that will be notified when the attributes change.
    //
    if(PieAttributesObserver == 0)
    {
        PieAttributesObserver = new ObserverToCallback(subj,
            PyPieAttributes_CallLogRoutine, (void *)data);
    }

}

void
PyPieAttributes_CloseDown()
{
    delete defaultAtts;
    defaultAtts = 0;
    delete PieAttributesObserver;
    PieAttributesObserver = 0;
}

PyMethodDef *
PyPieAttributes_GetMethodTable(int *nMethods)
{
    *nMethods = 1;
    return PieAttributesMethods;
}

bool
PyPieAttributes_Check(PyObject *obj)
{
    return (obj->ob_type == &PieAttributesType);
}

PieAttributes *
PyPieAttributes_FromPyObject(PyObject *obj)
{
    PieAttributesObject *obj2 = (PieAttributesObject *)obj;
    return obj2->data;
}

PyObject *
PyPieAttributes_New()
{
    return NewPieAttributes(0);
}

PyObject *
PyPieAttributes_Wrap(const PieAttributes *attr)
{
    return WrapPieAttributes(attr);
}

void
PyPieAttributes_SetParent(PyObject *obj, PyObject *parent)
{
    PieAttributesObject *obj2 = (PieAttributesObject *)obj;
    obj2->parent = parent;
}

void
PyPieAttributes_SetDefaults(const PieAttributes *atts)
{
    if(defaultAtts)
        delete defaultAtts;

    defaultAtts = new PieAttributes(*atts);
}

