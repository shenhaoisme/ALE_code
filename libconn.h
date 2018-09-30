/*************************************************************************
>          
>          File Name: libconn.h
>          
>          Author: Michel Sumyan
>          
>          Created Time: 2017/10/05
>          
*************************************************************************/
#ifndef __LIBCONN__H__
#define __LIBCONN__H__

#define   CONN_OK                          0
#define   CONN_ERR                        -1

#define   CONN_ERR_SERVER_SOCKET          -2
#define   CONN_ERR_SERVER_BIND            -3
#define   CONN_ERR_SERVER_LISTEN          -4
#define   CONN_ERR_SERVER_ACCEPT          -5

#define   CONN_ERR_CLIENT_CONNECT         -10
#define   CONN_ERR_CLIENT_BIND            -11
#define   CONN_ERR_CLIENT_SEND            -12
#define   CONN_ERR_CLIENT_SELECT          -13
#define   CONN_ERR_CLIENT_RECV            -14

/*
 ***************************************************************
 ***************************************************************
 ** SERVER PART
 ***************************************************************
 ***************************************************************
*/
/*
 * Callback function to process message
 * - cmd :    contains the received command (including null char for debug request)
 * - lenCmd : contains the received command length (including null char for debug request)
 * - out    : contains the pointer to store the response buffer address
 *            this buffer must be maloc by callback then freed by conn library
 * Return:
 * - 0 if OK and nothing to answer
 * - the output buffer len  if > 0
 * - errcode if < 0
 * Note:
 * - output buffer is freed only if not NULL and return > 0
 */

typedef int (*connProcessFunc)(const char *cmd, int lenCmd, char **out);

/*
 * Server loop request - No return
 * Manage requests from client
 * callback to manage requests must returns an alloc buffer into **out if not NULL
 * this buffer wll be free by lib
 * - sockName: server socket path (full path)
 * - func: callback function to be called when receiving command
 * - maxCmdLen: max length of received command to be processed by func
 * No return - infinite loop
 */
extern void conn_server_loop_request(const char *sockName, connProcessFunc func, int maxCmdLen);

/*
 ***************************************************************
 ***************************************************************
 ** CLIENT PART
 ***************************************************************
 ***************************************************************
*/
/*
 * Send cmd request to socket and get response id resp != NULL and maxResp != 0
 * Command & Response are uint8_t bytes
 * - sockName: server socket path (full path)
 * - sockId:   NULL or client socket prefix (ex: "led.", default is "conn.") 
 * - cmd:      input cmd
 * - lenCmd:   input cmd length
 * - resp:     output buffer
 * - maxResp:  max output buffer len
 * - timeout:  timeout for response
 * Return:
 *  if OK:  return  0 or response length
 *  if NOK: return errcode < 0
 */
extern int conn_client_request(const char *sockName, const char *sockId, const char *cmd, int lenCmd, char *resp, int maxResp, int timeout);

/*
 * Send debug request to socket and get response id resp != NULL and maxResp != 0
 * Command & Response are strings null terminated
 * - sockName: server socket path (full path)
 * - cmd:     input cmd (NULL terminated)
 * - lenCmd:  input cmd length
 * - resp:    output buffer
 * - maxResp: max output buffer len including ending NULL char
 * - timeout: timeout for response
 * Return:
 *  if OK:  return  0 or response length
 *  if NOK: return errcode < 0
 */
extern int conn_client_debug_request(const char *sockName, const char *sockId, const char *cmd, char *resp, int maxResp, int timeout);

#endif
