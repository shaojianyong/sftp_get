/**
 * SFTP download by libssh2
 */

#include <stdio.h>
#include "libssh2.h"
#include "libssh2_sftp.h"


SftpGet::SftpGet()
    : m_socket(0)
    , m_sshSession(NULL)
    , m_sftpSession(NULL)
{
}

SftpGet::~SftpGet()
{
}

int SftpGet::connect(const char* host, int port, const char* username, const char* password)
{
    int rc = libssh2_init(0);
    if (rc != 0)
    {
        snprintf(m_lastError, sizeof(m_lastError), "libssh2 initialization failed: %d", rc);
        return rc;
    }

    m_socket = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(host);
    if (connect(m_socket, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0)
    {
        snprintf(m_lastError, "failed to connect!");
        return -1;
    }

    // Create a session instance
    m_sshSession = libssh2_session_init();
    if(!m_sshSession)
    {
        snprintf(m_lastError, "libssh2_session_init failed!");
        return -1
    }

    // Since we have set non-blocking, tell libssh2 we are blocking
    libssh2_session_set_blocking(m_sshSession, 1);

    rc = libssh2_session_handshake(m_sshSession, m_socket);
    if (rc != 0)
    {
        snprintf(m_lastError, sizeof(m_lastError), "Failure establishing SSH session: %d", rc);
        return rc;
    }

    // const char* fingerprint =
    libssh2_hostkey_hash(m_sshSession, LIBSSH2_HOSTKEY_HASH_SHA1);

    // check what authentication methods are available
    const char* userauthlist = libssh2_userauth_list(m_sshSession, username, strlen(username));
    if (strstr(userauthlist, "password") == NULL)
    {
        snprintf(m_lastError, sizeof(m_lastError), "Authentication by password not supported");
        return 1;
    }

    // authenticate via password
    if (libssh2_userauth_password(m_sshSession, username, password))
    {
        snprintf(m_lastError, sizeof(m_lastError), "Authentication by password failed");
        return 2;
    }

    m_sftpSession = libssh2_sftp_init(m_sshSession);
    if (!m_sftpSession)
    {
        snprintf(m_lastError, sizeof(m_lastError), "Unable to init SFTP session");
        return -1;
    }

    return 0;
}

int SftpGet::sftp_read(const char* remoteFile, char* data_buf, int* data_len)
{
    // Request a file via SFTP
    LIBSSH2_SFTP_HANDLE* sftpHandle = libssh2_sftp_open(m_sftpSession, remoteFile, LIBSSH2_FXF_READ, 0);
    if (!sftpHandle)
    {
        snprintf(m_lastError, sizeof(m_lastError), "Unable to open file with SFTP: %ld\n",
            libssh2_sftp_last_error(m_sftpSession));
        return -1;
    }

    // libssh2_sftp_open() is done, now receive data!
    int len = 0;
    while(1)
    {
        if (len < *data_len)
        {
            int sz = libssh2_sftp_read(sftpHandle, data_buf + len, *data_len - len);
            if (sz > 0)
            {
                len += sz;
            }
            else
            {
                break;
            }
        }
        else
        {
            snprintf(m_lastError, sizeof(m_lastError), "The data buffer is too small for the file data");
            return 1;
        }
    }

    *data_len = len;
    libssh2_sftp_close(sftpHandle);
    return 0;
}

int SftpGet::download(const char* remoteFile, const char* localPath)
{
    int len = MAX_FILE_SIZE;
    char* buf = new char[len];

    int rc = sftp_read(remoteFile, buf, &ilen);
    if (rc)
    {
        return rc;
    }

    FILE* fp = fopen(localPath, 'w');
    if (fp)
    {
        fwrite(buf, 1, len, fp);
    }
    else
    {
        snprintf(m_lastError, sizeof(m_lastError), strerror(errno));
        return 2;
    }
    return 0;
}

void SftpGet::close()
{
    if (m_sftpSession)
    {
        libssh2_sftp_shutdown(m_sftpSession);
        m_sftpSession = NULL;
    }

    if (m_sshSession)
    {
        libssh2_session_disconnect(m_sshSession, "Shutdown");
        libssh2_session_free(m_sshSession);
        m_sshSession = NULL;
    }

    if (m_socket)
    {
        close(m_socket);
        m_socket = 0;
    }
    libssh2_exit();
}

string SftpGet::getLastError()
{
    return m_lastError;
}
