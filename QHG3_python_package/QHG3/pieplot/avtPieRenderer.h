// avtXXXXRenderer.h
#ifndef PIE_RENDERER_H
#define PIE_RENDERER_H
#include <avtCustomRenderer.h>
#include <PieAttributes.h>

#include "PieManager.h"
 
class avtPieRenderer : public avtCustomRenderer
{
public:
                            avtPieRenderer();
    virtual                ~avtPieRenderer();
    static avtPieRenderer  *New(void);
 
    // Override this method to provide customized rendering
    // functionality.
    virtual void            Render(vtkDataSet *);
 
    // Override this method so your renderer can release its
    // graphical resources when instructed to do so.
    virtual void            ReleaseGraphicsResources();
 
    // Provide this method to set your renderer's attributes from
    // avtPiePlot::SetAtts so your renderer can use data from
    // the plot's state object.
    void                    SetAtts(const AttributeGroup *);


    void                    SetVariable(const char *);

private:
    // Store the plot attributes so the renderer can use them
    const PieAttributes          *pAtts;

    char                  *varname;

    PieManager *m_pPieMan;
};
 
typedef ref_ptr<avtPieRenderer> avtPieRenderer_p;
#endif
