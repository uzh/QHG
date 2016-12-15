#include <hdf5.h>

#include "SCell.h"
#include "QDFVisitUtils.h"

class QDFReaderUtils {

public:
    static int readCellData(hid_t, int, SCell*);
    static hid_t createCellDataType();

};

