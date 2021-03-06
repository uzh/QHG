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

// ************************************************************************* //
//                             avtPiePlot.C                                 //
// ************************************************************************* //

#include <vtkPointData.h>
#include <avtPiePlot.h>

#include <avtPieFilter.h>
#include <avtUserDefinedMapper.h>
#include <DebugStream.h>


#include <avtDataObjectString.h>
#include <avtVertexNormalsFilter.h>
// ****************************************************************************
//  Method: avtPiePlot constructor
//
//  Programmer: jody -- generated by xml2avt
//  Creation:   Wed Mar 18 08:34:48 PDT 2015
//
// ****************************************************************************

avtPiePlot::avtPiePlot()
{
    debug1 << "[avtPiePlot::avtPiePlot()]" << endl;
    PieFilter = new avtPieFilter();
    renderer = avtPieRenderer::New();
    avtCustomRenderer_p cr;
    CopyTo(cr, renderer);
    myMapper = new avtUserDefinedMapper(cr);

 
}


// ****************************************************************************
//  Method: avtPiePlot destructor
//
//  Programmer: jody -- generated by xml2avt
//  Creation:   Wed Mar 18 08:34:48 PDT 2015
//
// ****************************************************************************

avtPiePlot::~avtPiePlot()
{
    debug1 << "[avtPiePlot::~avtPiePlot()]" << endl;

    if (myMapper != NULL)
    {
        delete myMapper;
        myMapper = NULL;
    }
    if (PieFilter != NULL)
    {
        delete PieFilter;
        PieFilter = NULL;
    }
}


// ****************************************************************************
//  Method:  avtPiePlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: jody -- generated by xml2avt
//  Creation:   Wed Mar 18 08:34:48 PDT 2015
//
// ****************************************************************************

avtPlot*
avtPiePlot::Create()
{
    debug1 << "[avtPiePlot::Create]" << endl;
    return new avtPiePlot;
}


// ****************************************************************************
//  Method: avtPiePlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: jody -- generated by xml2avt
//  Creation:   Wed Mar 18 08:34:48 PDT 2015
//
// ****************************************************************************

avtMapper *
avtPiePlot::GetMapper(void)
{
    debug1 << "[avtPiePlot::GetMapper()]" << endl;
    return myMapper;
}


// ****************************************************************************
//  Method: avtPiePlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a Pie plot.  
//      The output from this method is a query-able object.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Pie plot has been applied.
//
//  Programmer: jody -- generated by xml2avt
//  Creation:   Wed Mar 18 08:34:48 PDT 2015
//
// ****************************************************************************

avtDataObject_p
avtPiePlot::ApplyOperators(avtDataObject_p input)
{
    debug1 << "[avtPiePlot::ApplyOperators(avtDataObject_p input)]" << endl;


    return input;
}


// ****************************************************************************
//  Method: avtPiePlot::ApplyRenderingTransformation
//
//  Purpose:
//      Applies the rendering transformation associated with a Pie plot.  
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Pie plot has been applied.
//
//  Programmer: jody -- generated by xml2avt
//  Creation:   Wed Mar 18 08:34:48 PDT 2015
//
// ****************************************************************************

avtDataObject_p
avtPiePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    debug1 << "[avtPiePlot::ApplyRenderingTransformation(avtDataObject_p input)]" << endl <<input->GetType() << endl << std::flush;
    debug1 << "  sdim: " << input->GetInfo().GetAttributes().GetSpatialDimension() << endl;
    debug1 << "  var:  " << input->GetInfo().GetAttributes().GetVariableName() << endl;
    debug1 << "  mesh: " << input->GetInfo().GetAttributes().GetMeshname() << endl;
    avtVarType t  = input->GetInfo().GetAttributes().GetVariableType();
    debug1 << avtVarTypeToString(t) << endl;

    PieFilter->SetInput(input);
    debug1 << "new piefilter input set" << endl << std::flush;
    input = PieFilter->GetOutput();
    debug1 << "new piefilter output gotten" << endl << std::flush;
    
    return input;
}


// ****************************************************************************
//  Method: avtPiePlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a Pie plot.  This includes
//      behavior like shifting towards or away from the screen.
//
//  Programmer: jody -- generated by xml2avt
//  Creation:   Wed Mar 18 08:34:48 PDT 2015
//
// ****************************************************************************

void
avtPiePlot::CustomizeBehavior(void)
{
 debug1 << "[avtPiePlot::CustomizeBehavior()]" << endl;
    //behavior->SetShiftFactor(0.6);
}


// ****************************************************************************
//  Method: avtPiePlot::CustomizeMapper
//
//  Purpose:
//      A hook from the base class that allows the plot to change its mapper
//      based on the dataset input. 
//
//  Arguments:
//      doi     The data object information.
//
//  Programmer: jody -- generated by xml2avt
//  Creation:   Wed Mar 18 08:34:48 PDT 2015
//
// ****************************************************************************

void
avtPiePlot::CustomizeMapper(avtDataObjectInformation &doi)
{
    debug1 << "[avtPiePlot::CustomizeMapper(avtDataObjectInformation &doi)]" << endl;
/* Example of usage.
    int dim = doi.GetAttributes().GetCurrentSpatialDimension();
    if (dim == 2)
    {
    }
    else
    {
    }
 */
}


// ****************************************************************************
//  Method: avtPiePlot::SetAtts
//
//  Purpose:
//      Sets the atts for the Pie plot.
//
//  Arguments:
//      atts    The attributes for this Pie plot.
//
//  Programmer: jody -- generated by xml2avt
//  Creation:   Wed Mar 18 08:34:48 PDT 2015
//
// ****************************************************************************

void
avtPiePlot::SetAtts(const AttributeGroup *a)
{
    debug1 << "[avtPiePlot::SetAtts(const AttributeGroup *a)]" << endl;
    const PieAttributes *newAtts = (const PieAttributes *)a;

    //@@    BASED ON ATTRIBUTE VALUES, CHANGE PARAMETERS IN MAPPER AND FILTER.
    //    debug1 << "[avtPiePlot::SetAtts(const AttributeGroup *a)] typename " << a->TypeName() << endl;
    renderer->SetAtts(newAtts);
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;
}
