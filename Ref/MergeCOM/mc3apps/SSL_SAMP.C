/*************************************************************************
 *
 *       System: MergeCOM-3 - Advanced Tool Kit
 *
 *    $Workfile:$
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This module provides sample code providing callback
 *               functions that can be used for implementing a
 *               Secure Socket Layer with the MergeCOM-3 Tool Kit.
 *               This sample uses the publically available OpenSSL library.
 *               It assumes the Windows 32 environment.
 *
 *    NOTE WELL: This software is supplied to demonstrate how callbacks
 *               are used with the MergeCOM-3 Tool Kit to provide secure
 *               socket connections.  It is not intended that this code
 *               be used in a production environment.
 *
 *************************************************************************
 *
 *		(c) 2002 Merge Technologies  Incorporated (d/b/a Merge eFilm)
 *                     Milwaukee, Wisconsin  53214
 *
 *                     -- ALL RIGHTS RESERVED --
 *
 *  This software is furnished under license and may be used and copied
 *  only in accordance with the terms of such license and with the
 *  inclusion of the above copyright notice.  This software or any other
 *  copies thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of the software is hereby
 *  transferred.
 *
 ************************************************************************/

/*
 *  Run-time version number string
 */
static char id_string[] = "$Header$";

/* $NoKeywords: $ */

/*
 *  Header Files (Note: Win32 assumed here)
 */
#include "mergecom.h"

#include <stdio.h>
#include <openssl/rsa.h>       /* SSLeay stuff */
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct SSLcontext_struct {
    char*       certificateFile;
    char*       privateKeyFile;
    char*       passwrd;
    SSL_CTX*    ctx;
} SSLcontext;

static BIO *bio_err=NULL;

/*
 * Local Function Prototypes
 */
static SS_STATUS NOEXP_FUNC Sample_Session_Start_Callback
                                        (int           SocketToUse,
                                         CONN_TYPE     ConnectionType,
                                         void*         ApplicationContext,
                                         void**        SecurityContext);

static SS_STATUS NOEXP_FUNC Sample_Read_Callback
                                        (void*         SScontext,
                                         void*         ApplicationContext,
                                         char*         Buffer,
                                         unsigned int  BytesToRead,
                                         unsigned int* BytesRead,
                                         int           Timeout);
static SS_STATUS NOEXP_FUNC Sample_Write_Callback
                                        (void*         SScontext,
                                         void*         ApplicationContext,
                                         char*         Buffer,
                                         unsigned int  BytesToWrite,
                                         unsigned int* BytesWritten,
                                         int           Timeout);
static void NOEXP_FUNC Sample_Session_Shutdown_Callback
                                        (void*         SScontext,
                                        void*          ApplicationContext);


/*
 * Applications provide this structure when calling
 * MC_Open_Secure_Association or MC_Wait_For_Secure_Association
 */
SecureSocketFunctions sampleSSfunctions = {
    Sample_Session_Start_Callback,
    Sample_Read_Callback,
    Sample_Write_Callback,
    Sample_Session_Shutdown_Callback
};


/*
 * Return 0 if something is available to read within the timeout period
 *        1 if timed out
 *       -1 if error 
 */
static int waitForInput(SSL* ssl, SOCKET socket, struct timeval* tvPtr) {
    fd_set           readfds;
    int              i;

    for (;;) {
        /* Is there something to read yet? */
        if (SSL_pending(ssl)) 
            return 0;

        FD_ZERO(&readfds);
        FD_SET(socket, &readfds);
        i = select(socket+1, (void *)&readfds, NULL, NULL, tvPtr);
        if (i == 0)    /* Timeout or error */
            return 1;
        if (i < 0)
            return -1;
        if (FD_ISSET(socket, &readfds)) 
            return 0;
    }
}

/*************************************************************************
 *
 *  Function    :   passwordCallback
 *
 *  Parameters  :   buf        - buffer to write password into.
 *                  size       - Length of buffer in bytes.
 *                  rwflag     - nonzero if the password will be used as a 
 *                               new password, i.e. user should be asked to 
 *                               repeat the password
 *                  userdata   - arbitrary pointer that can be set with 
 *                               SSL_CTX_set_default_passwd_cb_userdata()
 *                  returns    - number of bytes written to password 
 *                               buffer, -1 upon error
 *
 *  Return value:   size of the password.
 *
 *  Description :   Callback function so the SSL library doesn't ask for user
 *                  input for a password.
 *
 *************************************************************************/
static int passwordCallback(char *buf, int size, int rwflag, void *userdata)
{
  SSLcontext* pConnect = (SSLcontext *)userdata;
  int passwordSize;

  if (userdata == NULL) return -1;

  if (pConnect->passwrd == NULL) return -1;

  passwordSize = strlen(pConnect->passwrd);

  if (passwordSize > size) passwordSize = size;
  strncpy(buf, pConnect->passwrd, passwordSize);

  return passwordSize;
}


/*************************************************************************
 *
 *  Function    :   Sample_Session_Start_Callback
 *
 *  Parameters  :   SocketToUse       - Library provides this open socket
 *                  ConnectionType    - Library provides the session type:
 *                                       REQUESTER_CONNECTION, or
 *                                       ACCEPTOR_CONNECTION
 *                  ApplicationContext - Our application context
 *                                       SCP applications provide the
 *                                       path name of the certificate
 *                                       file.  Not used by SCU applications.
 *                  SecurityContext    - If SS_NORMAL_COMPLETION is returned,
 *                                       the context object pointer for the 
 *                                       secure connection must be placed
 *                                       here. 
 *  Return value:   SS_NORMAL_COMPLETION - if all went well
 *                  SS_ERROR - if fatal error occurred
 *
 *  Description :   Opens a secure connection on the socket provided
 *                  by the MergeCOM-3 library.  Returns the SSL
 *                  connection context.
 *
 *  Note:           The socket connection provided by MergeCOM-3 is non-blocking.
 *
 *************************************************************************/
static SS_STATUS NOEXP_FUNC Sample_Session_Start_Callback
                                        (int        SocketToUse,
                                         CONN_TYPE  ConnectionType,
                                         void*      ApplicationContext,
                                         void**     SecurityContext) {

    SSLcontext* appCtx = (SSLcontext*)ApplicationContext;
    int         err;
    static SSL* ssl;
    X509*       server_cert;
    char*       str;
    char        buf [4096];
    SSL_METHOD *meth;
    int         status;
    unsigned long l;
    struct timeval tv;

    if (ApplicationContext== NULL) {
        fprintf(stderr,"Sample_Session_Start_Callback: NULL ApplicationContext\n");
        return SS_ERROR;
    }

    if (SecurityContext == NULL) {
        fprintf(stderr,"Sample_Session_Start_Callback: NULL SecurityContext ptr\n");
        return SS_ERROR;
    }

    /* We will wait 15 seconds to get connected */
    tv.tv_sec = 15;
    tv.tv_usec = 0;

    if (bio_err == NULL)
        bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);

    if (ConnectionType == REQUESTER_CONNECTION)
        meth = TLSv1_client_method();
    else
        meth = TLSv1_server_method();

    /* Register the available ciphers and digests. */
    /* must be called before any other action takes place */
    SSLeay_add_ssl_algorithms();    /* Synonym for SSL_library_init */
    SSL_load_error_strings();

    /* Create a context object as a framework to establish connections */
    appCtx->ctx = SSL_CTX_new(meth);
    if (appCtx->ctx == NULL) {
        fprintf(stderr,"SSL: Unable to create new context\n");
        return SS_ERROR;
    }

    /*
     * Set the cipher suite for the secure connection.
     */
    SSL_CTX_set_cipher_list(appCtx->ctx, "TLS_RSA_WITH_NULL_SHA");


    /* register callback that replaces console input */
    SSL_CTX_set_default_passwd_cb(appCtx->ctx, passwordCallback);
    SSL_CTX_set_default_passwd_cb_userdata(appCtx->ctx, appCtx);

    /*
     * In this environment, only SCP has a certificate
     */
    if (ConnectionType == ACCEPTOR_CONNECTION) {
        SSL_CTX_set_quiet_shutdown(appCtx->ctx, 1);
        SSL_CTX_sess_set_cache_size(appCtx->ctx, 128);
        SSL_CTX_set_verify(appCtx->ctx, SSL_VERIFY_NONE ,NULL);  /* No client certificate */
        
        if (appCtx->certificateFile == NULL) {
            fprintf(stderr,"SSL: No certificate file provided by SCP\n");
            SSL_CTX_free (appCtx->ctx);
            return SS_ERROR;
        }
        status = SSL_CTX_use_certificate_file(appCtx->ctx, appCtx->certificateFile, SSL_FILETYPE_PEM);
        if (status <= 0) {
            fprintf(stderr,"Unable to load certificate file: %s\n", appCtx->certificateFile);
            SSL_CTX_free (appCtx->ctx);
            return SS_ERROR;
        }

        /* If no private key file presented, assume keys are in certificate file */
        if (appCtx->privateKeyFile == NULL)
            appCtx->privateKeyFile = appCtx->certificateFile;

        if (SSL_CTX_use_PrivateKey_file(appCtx->ctx, appCtx->privateKeyFile, SSL_FILETYPE_PEM) <= 0) {
            fprintf(stderr,"Unable to load private key from file: %s\n", appCtx->privateKeyFile);
            SSL_CTX_free (appCtx->ctx);
            return SS_ERROR;
        }

        if (!SSL_CTX_check_private_key(appCtx->ctx)) {
            fprintf(stderr,"Private key does not match the certificate public key\n");
            SSL_CTX_free (appCtx->ctx);
            return SS_ERROR;
        }
    }

    /*
     * Aquire an SSL context object
     */
    ssl = SSL_new(appCtx->ctx);
    if (ssl == NULL) {
        fprintf(stderr,"SSL: Unable to create new SSL object\n");
        SSL_CTX_free (appCtx->ctx);
        return SS_ERROR;
    }
    SSL_clear(ssl);

    /*
     * Use the socket file descriptor provided by MergeCOM-3
     */
    SSL_set_fd(ssl, SocketToUse);


    if (ConnectionType == ACCEPTOR_CONNECTION) {    /* SCP */
        SSL_set_accept_state(ssl);

        /* Try to accept */
        for (;;) {
            long verify_error;

            /* Wait till we get something from the client */

            if ( waitForInput(ssl, (SOCKET)SocketToUse, &tv) )
                return SS_ERROR;
            
            status = SSL_accept(ssl);
            if (status > 0) 
                break;

            if (BIO_sock_should_retry(status))
                continue;

            verify_error = SSL_get_verify_result(ssl);
            if (verify_error != X509_V_OK) {
                    fprintf(stderr, "Verify error:%s\n",
                        X509_verify_cert_error_string(verify_error));
            } else {
                err = SSL_get_error(ssl, status);
                if (err == SSL_ERROR_SYSCALL) {
                    /* Probably an IO error, let's check further */
                    while ((l=ERR_get_error()))
                        fprintf(stderr,"ERROR:%s\n",
                            ERR_error_string(l,NULL));
                }
                if (err == SSL_ERROR_SSL)
                    ERR_print_errors(bio_err);
                fprintf(stderr,"SSL_accept failed: %d (%d)\n", status, err);
            }
            return SS_ERROR;
        }

        /* Display the cipher - opt */
        printf("\tSSL connection using %s\n", SSL_get_cipher(ssl));
    }
    else {  /* SCU */
        SSL_set_connect_state(ssl);
        for (;;) {
            status = SSL_connect (ssl);                     
            if (status == -1) {
                err = SSL_get_error(ssl, status);
                if (err == SSL_ERROR_SYSCALL) {
                    /* Probably an IO error, let's check further */
                    while ((l=ERR_get_error()))
                        fprintf(stderr,"ERROR:%s\n",
                            ERR_error_string(l,NULL));
                }

                if (err == SSL_ERROR_SSL)
                    ERR_print_errors(bio_err);

                else if (err== SSL_ERROR_WANT_READ) {
                    if ( !waitForInput(ssl, (SOCKET)SocketToUse, &tv) )
                        continue;
                }
                fprintf(stderr,"SSL_connect failed: %d\n", err);
                SSL_shutdown(ssl);
                SSL_free (ssl);
                SSL_CTX_free (appCtx->ctx);
                return SS_ERROR;
            }
            break;
        }

        /* Display the cipher - opt */
        printf("\tSSL connection using %s\n", SSL_get_cipher(ssl));

        /* Get server's certificate */
        server_cert = SSL_get_peer_certificate (ssl);       
        if (server_cert == NULL)
            printf ("\tServer does not have a certificate\n");
        else {
            printf ("\tServer certificate:\n");

            str = X509_NAME_oneline (X509_get_subject_name (server_cert), buf, sizeof(buf));
            if (str != NULL)
                printf ("\t  subject: %s\n", str);
            else
                printf ("\t  subject: <ERROR>\n");

            str = X509_NAME_oneline (X509_get_issuer_name  (server_cert), buf, sizeof(buf));
            if (str != NULL)
                printf ("\t  issuer: %s\n", str);
            else
                printf ("\t  issuer: <ERROR>\n");

            /* 
             *  Do any other certificate verification stuff here before
             *  deallocating the certificate. 
             */
            X509_free (server_cert);
        }
    }

    *SecurityContext = ssl;
    return SS_NORMAL_COMPLETION;
}


/*************************************************************************
 *
 *  Function    :   Sample_Read_Callback
 *
 *  Parameters  :   SScontext          - SSL object that is our context
 *                  ApplicationContext - Application context
 *                                       Not used by the function
 *                  Buffer             - Buffer to receive data read
 *                  BytesToRead        - Number of bytes to read
 *                  BytesRead          - Number of bytes actually read
 *                                       is returned here (if not null)
 *                  Timeout            - Time to wait for read to complete
 *                                       Zero means try once
 *                                       < 0 means wait forever
 *
 *  Return value:   SS_NORMAL_COMPLETION - if all bytes were read
 *                  SS_TIMEOUT - if unable to read all bytes within Timeout
 *                  SS_ERROR - if fatal error occurred
 *                  SS_SESSION_CLOSED - the session closed
 *
 *  Description :   Reads bytes from the secure connection and returns
 *                  them unencrypted.
 *
 *************************************************************************/
static SS_STATUS NOEXP_FUNC Sample_Read_Callback
                                        (void*         SScontext,
                                         void*         ApplicationContext,
                                         char*         Buffer,
                                         unsigned int  BytesToRead,
                                         unsigned int* BytesRead,
                                         int           Timeout) {

    unsigned int      bytesRead, bytes2read;
    int               err;
    time_t            start_time;
    time_t            subsequentTimeToWait, timeToWait;
    struct timeval    timeout;
    struct timeval*   timeoutPtr;
    int               firstSelect;
    int               waitStatus;
    int               bytesReceived;

    if (SScontext== NULL) {
        fprintf(stderr,"SSL: NULL SScontext on Read\n");
        return SS_ERROR;
    }

    if (BytesRead)
        *BytesRead = 0L;
    bytesRead = 0;

    if (Timeout > 0)
        subsequentTimeToWait = Timeout;
    else
        subsequentTimeToWait = 30;

    start_time = time((time_t *)NULL);
    firstSelect = 1;
    
    for (;;) {
        if (firstSelect)
            timeToWait = (time_t)(long)Timeout;
        else
            if (Timeout < 0)
                timeToWait = (time_t)(long)Timeout;
            else
                timeToWait = subsequentTimeToWait;

        if (timeToWait == 0)
        {
            timeout.tv_sec  = 0;
            timeout.tv_usec = 1;
            timeoutPtr = &timeout;
        }       
        else if (timeToWait > 0)
        {
            timeout.tv_sec  = timeToWait;
            timeout.tv_usec = 0;
            timeoutPtr = &timeout;
        }       
        else
            timeoutPtr = NULL;

        waitStatus = waitForInput((SSL*)SScontext, 
                                 (SOCKET)SSL_get_fd((SSL*)SScontext), 
                                 timeoutPtr);
        if (waitStatus == 1)        /* i.e. timeout */
        {       
            if (firstSelect)
                return SS_TIMEOUT;
        }
        else if (waitStatus != 0)
            return SS_TIMEOUT;      /* select error */

        /* There's something to read or we timed out */
        firstSelect = 0;
        if (waitStatus == 0) {      /* something to read */

            bytes2read = BytesToRead - bytesRead;
            bytesReceived = SSL_read ((SSL*)SScontext, Buffer + bytesRead, (int)bytes2read);
            err = SSL_get_error((SSL*)SScontext, bytesReceived);
            if (err == SSL_ERROR_NONE) {
                bytesRead += bytesReceived;
                if (BytesRead)
                    *BytesRead = bytesRead;
                if (bytesRead >= (unsigned int)BytesToRead)
                    break;
            }
            else if (err == SSL_ERROR_ZERO_RETURN)
                return SS_SESSION_CLOSED;
            else if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE &&
                err != SSL_ERROR_WANT_X509_LOOKUP)
                return SS_SESSION_CLOSED;   /* some kind of problem with transport */
        }

        if (timeoutPtr)
        {
            subsequentTimeToWait -= (time((time_t *)NULL) - start_time);
            start_time = time ((time_t *)NULL);
            if (subsequentTimeToWait <= 0)
                return SS_TIMEOUT;
        }        
    }   /* Keep trying till we timeout or succeed */
    return SS_NORMAL_COMPLETION;
}


/*************************************************************************
 *
 *  Function    :   Sample_Write_Callback
 *
 *  Parameters  :   SScontext          - SSL object that is our context
 *                  ApplicationContext - Application context
 *                                       (Not used by this function)
 *                  Buffer             - Buffer containing bytes to write
 *                  BytesToWrite       - Number of bytes to write
 *                  BytesWritten       - Number of bytes actually written
 *                                       is returned here (if not null)
 *                  Timeout            - # of seconds to wait
 *                                       (if <= 0, 15 will be used)
 *
 *  Return value:   SS_NORMAL_COMPLETION - if all bytes were written
 *                  SS_ERROR - if fatal error occurred
 *                  SS_TIMEOUT - if unable to write all bytes with Timeout
 *
 *  Description :   Writes bytes over the secure connection.
 *
 *************************************************************************/
static SS_STATUS NOEXP_FUNC Sample_Write_Callback
                                        (void*         SScontext,
                                         void*         ApplicationContext,
                                         char*         Buffer,
                                         unsigned int  BytesToWrite,
                                         unsigned int* BytesWritten,
                                         int           Timeout) {

    

    unsigned int       tcount;
    time_t             start;
    struct timeval     timeout;
    struct timeval*    timeoutPtr;
    time_t             timeToWait;
    fd_set             wfds;
    int                nfound;
    SOCKET             socket;
    int                bytes;
    int                err;
        
    if (SScontext== NULL) {
        fprintf(stderr,"SSL: NULL SScontext on Write\n");
        return SS_ERROR;
    }
    
    if (BytesWritten)
        *BytesWritten = 0;

    tcount = BytesToWrite;
    socket = (SOCKET)SSL_get_fd((SSL*)SScontext);

    (void)time(&start);
    
    /*
     * Setup the time to wait for the timeout to select,
     * note that the default is 15
     */    
    if (Timeout > 0)
        timeToWait = (time_t)(long)Timeout;
    else
        timeToWait = (time_t)(long)15;

    for (;;)
    {
        timeout.tv_sec  = timeToWait;
        timeout.tv_usec = 0;
        timeoutPtr = &timeout;

        FD_ZERO(&wfds);
        FD_SET(socket, &wfds);

        nfound = select(socket+1, NULL, &wfds, NULL, timeoutPtr);
        if (nfound == 0)        /* i.e. timeout */
            return SS_TIMEOUT;
        else if (nfound < 0)
        {
            fprintf(stderr,"SSL_write failed on select\n");
            return SS_ERROR;
        }
        else if (!FD_ISSET( socket, &wfds ))
        {
            fprintf(stderr,"SSL_write select failed\n");
            return SS_ERROR;
        }
    
        /*
         * If working on a secure connection, 
         * use callback to write the data
         */
        bytes = SSL_write ((SSL*)SScontext, Buffer, (int)tcount);
        err = SSL_get_error((SSL*)SScontext, bytes);
        if (err == SSL_ERROR_NONE) {
            Buffer += bytes;
            tcount -= bytes;
            if (BytesWritten)
                *BytesWritten += bytes;
            if (tcount <= 0)
                break;
        }       
        else if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE) {
            fprintf(stderr,"SSL_write failed\n");
            return SS_ERROR;
        }

        timeToWait -= (time((time_t *)NULL) - start);
        start = time ((time_t *)NULL);
        if (timeToWait <= 0)
            return SS_TIMEOUT;
    } 
    
    return SS_NORMAL_COMPLETION;
}


/*************************************************************************
 *
 *  Function    :   Sample_Session_Shutdown_Callback
 *
 *  Parameters  :   SScontext          - SSL object that is our context
 *                  ApplicationContext - Application context
 *                                       Not used by the function
 *
 *  Return value:   none
 *
 *  Description :   Closes the SSL session.  NOTE: The session's socket
 *                  connection remains open and is handled by the
 *                  MergeCOM-3 library.
 *
 *************************************************************************/
static void NOEXP_FUNC Sample_Session_Shutdown_Callback
                                        (void*      SScontext,
                                        void*       ApplicationContext) {

    SSLcontext* appCtx = (SSLcontext*)ApplicationContext;

    if (SScontext != NULL) {
        SSL_shutdown((SSL*)SScontext);
        SSL_free((SSL*)SScontext);
    }
    if (ApplicationContext && appCtx->ctx != NULL)
        SSL_CTX_free (appCtx->ctx);
}
