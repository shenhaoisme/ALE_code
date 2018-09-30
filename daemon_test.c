/*

Sep 30 14:59:36 izm5eavb11mttn4gjzqgm4z daemontest[13176]: program started.
Sep 30 15:00:01 izm5eavb11mttn4gjzqgm4z systemd[1]: Created slice User Slice of root.
Sep 30 15:00:01 izm5eavb11mttn4gjzqgm4z systemd[1]: Starting User Slice of root.
Sep 30 15:00:01 izm5eavb11mttn4gjzqgm4z systemd[1]: Started Session 1994 of user root.
Sep 30 15:00:01 izm5eavb11mttn4gjzqgm4z systemd[1]: Starting Session 1994 of user root.
Sep 30 15:00:01 izm5eavb11mttn4gjzqgm4z systemd[1]: Removed slice User Slice of root.
Sep 30 15:00:01 izm5eavb11mttn4gjzqgm4z systemd[1]: Stopping User Slice of root.
Sep 30 15:00:02 izm5eavb11mttn4gjzqgm4z syslog-ng[458]: Log statistics; processed='destination(d_spol)=0',
processed='src.internal(s_sys#2)=1596', stamp='src.internal(s_sys#2)=1538290202', 
processed='center(received)=1596',processed='destination(d_mesg)=13673',processed='destination(d_mail)=4',
processed='destination(d_auth)=3177', 
processed='destination(d_mlal)=0', processed='center(queued)=19453', processed='src.none()=0', stamp='src.none()=0',
processed='destination(d_cron)=2587', processed='global(payload_reallocs)=3208', processed='global(sdata_updates)=0',
processed='destination(d_boot)=0', processed='destination(d_kern)=12', processed='global(msg_clones)=0', processed='source(s_sys)=1596'
Sep 30 15:00:03 izm5eavb11mttn4gjzqgm4z daemontest[13176]: program terminated.
*/

#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <syslog.h>  
#include <signal.h>  
int daemon_init(void)   
{   
    pid_t pid;   
  if((pid = fork()) < 0)   
    return(-1);   
  else if(pid != 0)   
    exit(0); /* parent exit */   
/* child continues */   
  setsid(); /* become session leader */   
  chdir("/"); /* change working directory */   
  umask(0); /* clear file mode creation mask */   
  close(0); /* close stdin */   
  close(1); /* close stdout */   
  close(2); /* close stderr */   
  return(0);   
}  
 //等待关闭 
void sig_term(int signo)   
{   
    if(signo == SIGTERM)   
/* catched signal sent by kill(1) command */   
  {   
    syslog(LOG_INFO, "program terminated.");   
  closelog();   
  exit(0);   
  }   
}  
   
int main(void)   
{   
    if(daemon_init() == -1)   
  {   
    printf("can't fork self/n");   
    exit(0);   
  }   
  openlog("daemontest", LOG_PID, LOG_USER);   
  syslog(LOG_INFO, "program started.");   
  signal(SIGTERM, sig_term); /* arrange to catch the signal */   
  while(1)   
  {   
    sleep(1); /* put your main program here */   
  }   
  return(0);   
} 
