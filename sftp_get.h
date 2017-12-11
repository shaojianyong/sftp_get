/**
 * SFTP download by libssh2
 */
#ifndef __SFTP_GET__
#define __SFTP_GET__

#include <string>
using namespace std;

const int MAX_FILE_SIZE = 5 * 1024 * 1024;

class SftpGet {
public:
    SftpGet();
    virtual ~SftpGet();

public:
    int connect(const char* host, int port, const char* username, const char* password);

    int sftp_read(const char* remoteFile, char* data_buf, int** data_len);

    int download(const char* remoteFile, const char* localPath);

    void close();

    string getLastError();

private:
    int m_socket;
    char* m_lastError[1024];
    LIBSSH2_SESSION* m_sshSession;
    LIBSSH2_SFTP* m_sftpSession;
};

#endif
