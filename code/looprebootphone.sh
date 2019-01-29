#/usr/bin/env sh


#!/bin/sh

### BEGIN INIT INFO
# Provides:             Guoshun.WU
# Required-Start:       $remote_fs $syslog
# Required-Stop:        $remote_fs $syslog
# Default-Start:        2 3 4 5
# Default-Stop:         2 3 5
# Short-Description:    Mini EDS start script
### END INIT INFO

# description: Start/stop reboot coredump check.
#       Usage: 
#              Put this file into /etc/init.d
#       	   Create a init link such as: ln -sf $this_file /etc/rc.d/SXXloop
#

############
# CHANGELOG
#=================================
# 2018/09/06  Guoshun WU
#      -      Create the file
#=================================
DESC="Reboot crash check."
scriptname=RCC


main(){
    case "$1" in
    start)
        start
    ;;
    stop)
        : "Not used so far."
    ;;
    status)
        : "Not used so far."
    ;;
  *)
    echo "Usage: $0 {start|stop}" >&2
        exit 1
        ;;
    esac
}

start(){
    # reboot in loop
    
    COUNTER_FILE=/root/resetCounter.txt

    counter=1
    [[ -f ${COUNTER_FILE} ]] && counter=`cat ${COUNTER_FILE}`

    printf "The %d time reboot....\n" $counter
    
    ls /data/core.*
    hasCore=$?

    if [[ $hasCore -eq 0 ]]; then
        printf "Coredump found by after %d time reboot!!\n" counter
    else
        counter=$(( $counter + 1 ))
        echo "No error found, reboot in 10 seconds"
        echo "${counter}">"${COUNTER_FILE}"
        sleep 10
        reset
    fi
}

main $@