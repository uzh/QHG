/*============================================================================
| EventConsts
| 
|  Various constants for events and their parameters
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __EVENTCONSTS_H__
#define __EVENTCONSTS_H__

#define EVENT_ID_NONE    -1
#define EVENT_ID_WRITE    1

#define EVENT_ID_GEO      2
#define EVENT_ID_CLIMATE  3
#define EVENT_ID_VEG      4
#define EVENT_ID_NAV      5

#define EVENT_ID_ENV      6
#define EVENT_ID_ARR      7
#define EVENT_ID_POP      8
#define EVENT_ID_COMM     9
#define EVENT_ID_CHECK   10
#define EVENT_ID_DUMP    11

#define EVENT_ID_FLUSH    20
#define EVENT_ID_USER     21

#define EVENT_ID_USR_MIN 1000
#define EVENT_ID_USR_MAX 9999

#define EVENT_TYPE_WRITE          "write"
/*
#define EVENT_TYPE_GEO            "geo"
#define EVENT_TYPE_CLIMATE        "climate"
#define EVENT_TYPE_VEG            "veg"
#define EVENT_TYPE_NAV            "nav"
*/

#define EVENT_TYPE_ENV            "env"
#define EVENT_TYPE_ARR            "arr"
#define EVENT_TYPE_POP            "pop"
#define EVENT_TYPE_COMM           "comm"
#define EVENT_TYPE_FILE           "file"
#define EVENT_TYPE_CHECK          "check"
#define EVENT_TYPE_DUMP           "dump"
#define EVENT_TYPE_USER           "user"

#define EVENT_PARAM_NAME_GEO      "geo"
#define EVENT_PARAM_NAME_CLIMATE  "climate"
#define EVENT_PARAM_NAME_VEG      "veg"
#define EVENT_PARAM_NAME_NAV      "nav"
#define EVENT_PARAM_NAME_ALL      "all"

#define EVENT_PARAM_WRITE_GRID    "grid"
#define EVENT_PARAM_WRITE_GEO     "geo"
#define EVENT_PARAM_WRITE_CLIMATE "climate"
#define EVENT_PARAM_WRITE_VEG     "veg"
#define EVENT_PARAM_WRITE_POP     "pop"
#define EVENT_PARAM_WRITE_STATS   "stats"
#define EVENT_PARAM_WRITE_NAV     "nav"
#define EVENT_PARAM_WRITE_ENV     "env"

/*
#define EVENT_PARAM_GEO_SEA       "sea"
#define EVENT_PARAM_GEO_ALT       "alt"
#define EVENT_PARAM_GEO_ICE       "ice"
#define EVENT_PARAM_GEO_WATER     "water"
*/

#define EVENT_PARAM_GEO_QDF       "qdf"
#define EVENT_PARAM_CLIMATE_QDF   "qdf"
#define EVENT_PARAM_VEG_QDF       "qdf"
#define EVENT_PARAM_NAV_QDF       "qdf"

#define EVENT_PARAM_CHECK_LISTS   "lists"

#endif
