#!/bin/bash
APP="../app/"
POPULATIONS="../populations/"
COMMON="../common/"
IDS_SRC="ids.h_for_configure"
FACTORY_SRC="PopulationFactory.cpp_for_configure"

if  [ X${SHORT} == X1 ]
then
    POPULATIONS="../pop_work/"
fi
echo "pop: [${POPULATIONS}]"

# arbitrary baseline for species numbering: 101
POPCOUNT=101

SPC_STRING=""
IDS_STRING=""
INCLUDE_STRING="\n"
FACTORY_STRING="\n"
SUMMARY_STRING=""

for popheader in `ls ${POPULATIONS}*Pop.h`; do

    # find name of population in header file
    BIGNAME=`sed -n '/#ifndef.*POP_H__/p' ${popheader} | awk '{print $2}'`
    BIGNAME=${BIGNAME%"POP_H__"}
    BIGNAME=${BIGNAME#"__"}

    CLASSNAME=`awk '{ if ($1 == "class") { print $2;} }' ${popheader}`
    CLASSNAME=${CLASSNAME%"public"} # remove trailing : just in case
    CLASSNAME=${CLASSNAME%":"} # remove trailing : just in case

    IDS_STRING=${IDS_STRING}'const spcid CLASS_'${BIGNAME}' = '$POPCOUNT';\n'
    SPC_STRING=${SPC_STRING}'    {"'${CLASSNAME}'", CLASS_'${BIGNAME}'},\n'

    popheader=${popheader#${KERNEL}}
    INCLUDE_STRING=${INCLUDE_STRING}'#include "'${popheader}'"\n'
    FACTORY_STRING=${FACTORY_STRING}'\t\tcase CLASS_'${BIGNAME}':\n'\
'\t\t\tif (bVerbose) { printf("PopulationFactory is creating '${CLASSNAME}'\\n"); }\n'\
'\t\t\tpPop = new '${CLASSNAME}'(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState,m_aiSeeds);\n'\
'\t\t\tbreak;\n'

    SUMMARY_STRING=${SUMMARY_STRING}'CLASS '${CLASSNAME}' : ClassID '${POPCOUNT}$'\n'

    POPCOUNT=`expr $POPCOUNT + 1`
done


# add lines to PopulationFactory.cpp

FACTORY_OUT=${FACTORY_SRC%"_for_configure"}
echo "updating "${POPULATIONS}${FACTORY_OUT}

sed -e "s@// CONFIGURE POPS: DO NOT DELETE THIS COMMENT LINE@${FACTORY_STRING}@" \
    -e "s@// CONFIGURE INCLUDE: DO NOT DELETE THIS COMMENT LINE@${INCLUDE_STRING}@" \
    ${POPULATIONS}${FACTORY_SRC} > ${POPULATIONS}${FACTORY_OUT}

# now add entries to ids.h file

#REV=`svnversion .. | gawk -F: '{ print $NF }' | sed -e "s/M//" -e "s/S//"`
REV=`svnversion ..`

IDS_OUT=${IDS_SRC%"_for_configure"}
echo "updating "${COMMON}${IDS_OUT}
sed -e "s@// CONFIGURE SPECIES: DO NOT DELETE THIS COMMENT LINE@${SPC_STRING}@" \
    -e "s@// CONFIGURE IDS: DO NOT DELETE THIS COMMENT LINE@${IDS_STRING}@" \
    -e "s/@@@REV@@@/${REV}/" \
    ${COMMON}${IDS_SRC} > ${COMMON}${IDS_OUT}


# we're done here

OUT=${APP}"AGENT_CLASS_SUMMARY.txt"
echo "### POPULATIONS SUMMARY ###" > ${OUT}
echo "${SUMMARY_STRING}" >> ${OUT}



