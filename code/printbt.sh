#!/bin/bash

# Author: Guoshun WU
# Date: 2016-10-10

# description:
#   This script print the backtrace of a core dumped on a 8018 or 80x8ee phone
#

# include helper function

source `dirname $0`/helper.sh

# default parameter values
DEFAULT_range=8018
DEFAULT_appmode=sip
DEFAULT_project=sip3f

# entry function
main(){

    # exit hook
    trap exit_handler EXIT

    if [ $# -lt 1 ]; then
        usage $0
    fi

    # init global parameter variables
    init $@

    buildDebugSoLibs
    

    # personal
    # APP_PATH=$PERSONAL_BASE_DIR/application
    
    app_symbol_file_pre=${APP_PATH}
    if [[ "true" == "${isLocalLinkedBin}" ]]; then
        echo "Linked binary, use local symbol file."
        app_symbol_file_pre=${SDP_ROOTDIR_APPLICATION}
    fi

    app_symbol_file=${app_symbol_file_pre}/application/${module_name}/bin/${symbol_file}.$suffix
    
    if [ ! -f "${app_symbol_file}" ];then
        echo "${app_symbol_file} does not exists!"
        exit 1
    fi

    GDB=/usr/local/bin/arm-wrs-linux-gnueabi-gdb
    # user local compiled GDB(which support python) 
    [[ ! -x ${GDB} ]] && GDB=arm926ejs-wrswrap-linux-gnueabi-gdb

    solib_dirs=${TARGET_DIR}

    gdbcmd="$GDB -q -ex \"set sysroot ${SYSROOT_DIR}\" \
        -ex \"set solib-search-path ${solib_dirs}\" \
        --core ${core_file} ${app_symbol_file} \
        -ex \"bt\""


    if [ -n "${VERBOSE}" ];then
        echo $gdbcmd
    else
        gdbcmd="${gdbcmd} --batch"
    fi
    eval $gdbcmd
}

exit_handler(){
    #echo "Exit peacefully..."
    if [ -n "$core_compressed" ]; then
        gzip -1 $core_file
    fi

    # whether delete generated libs
    [ -d ${TARGET_DIR} ] && rm -rf $TARGET_DIR
}

usage(){
    echo "usage: $0 <corefile> [range] [appmode] [project]"
    echo "      range   default is ${DEFAULT_range}       "
    echo "      appmode default is ${DEFAULT_appmode}     "
    echo "      project default is ${DEFAULT_project}     "
    exit 1
}

# initialize global parameters
init(){

    core_file=${1}
    shift
    range=${1:-${DEFAULT_range}}
    shift
    appmode=${1:-${DEFAULT_appmode}}
	shift
    project=${1:-${DEFAULT_project}}

    envtype=target

    if [ ! -f ${core_file} ]; then
        echo "Core file ${core_file} not found!"
        exit 1
    fi

    core_compressed=`file ${core_file} | grep 'gzip compressed'`
    if [ -n "$core_compressed" ]; then
        # core file is compressed, uncompress it.
        gzip -d ${core_file}
        core_file=${core_file/.gz}
    fi

    app=${core_file##*.}
    
    corestring=`strings ${core_file}`
    versions=`echo "${corestring}" | grep '^Version.*=.*'`
    #printf "versions=\n${versions}\n===================================\n"
    eval `echo "${versions}" | grep '^VersionRelease'`
    eval `echo "${versions}" | grep '^VersionSoft'`
    eval `echo "${versions}" | grep '^VersionBuild'`
    eval `echo "${versions}" | grep '^VersionDate'`
    
    module_name=`echo "${corestring}" | grep '\-\-moduleName=' | head -1`
    module_name=${module_name%--moduleName=*}
    module_name=${module_name##*/}
    # trim
    module_name=`echo ${module_name} | xargs `
    symbol_file=${module_name}
    if [[ x"ictbtmgr" == x"${module_name}" ]]; then
        symbol_file="ictbtmgr"
        module_name="BT"
    fi
    # remove prefix for the linked module name
    isLocalLinkedBin="false"
    if [[ "h_" == ${module_name:0:2} ]];then
        module_name=${module_name#h_}
        [[ "BT" != module_name ]] && symbol_file=${module_name}
        isLocalLinkedBin="true"
    fi

    # echo "module_name=a${module_name}a"

    if [ -z $VersionRelease -o -z $VersionSoft ];then
        echo "Version extract from core file ${core_file} failed!"
        exit 1
    fi


    ID=${appmode}/${range}/${envtype}/${VersionRelease}/${VersionSoft}/${VersionBuild}

    BASE_DIR=/misc/${project}_dev/daily/production/${ID}
    # for personal
    PERSONAL_BASE_DIR=/localdisk/working/${USER}/${project}/tmp/${appmode}/${range}/${envtype}

    if [ ! -d ${BASE_DIR} ]; then
        echo "${BASE_DIR} do not exist!"
        exit 1
    fi

    SYSROOT_DIR=${BASE_DIR}/platform/sysroot/sysroots/dspg_dvf99-wrs-linux-gnueabi
    APP_PATH=${BASE_DIR}/application
}

#call main function
main $@

