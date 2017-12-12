#include <iostream>
#include "sftp_get.h"

using namespace std;

int main(int argc, char** argv)
{
    SftpGet fg;
    int ret = fg.connect("127.0.0.1", 22, "root", "johnshao");
    if (ret)
    {
        string errmsg = fg.getLastError();
        cout << errmsg << endl;
        return -1;
    }

    int len = 1024 * 1024;
    char buf[len] = { 0 };
    ret = fg.sftp_read("interview/binary_find.cpp", buf, &len);
    if (ret)
    {
        string errmsg = fg.getLastError();
        cout << errmsg << endl;
        return -1;
    }
    cout << string(buf, len) << endl;

    ret = fg.download("interview/binary_find.cpp", "binary_find.cpp");
    if (ret)
    {
        string errmsg = fg.getLastError();
        cout << errmsg << endl;
        return -1;
    }

    return 0;
}
