include <iostream>
#include "sftp_get.h"

using namespace std;

int main(int argc, char** argv)
{
    SftpGet fg;
    int ret = fg.connect("127.0.0.1", 22, "johnshao", "johnshao");
    if (ret)
    {
        string errmsg = fg.getLastError();
        cout << errmsg << endl;
        return -1;
    }

    int len = 1024 * 1024;
    char buf[len] = { 0 };
    ret = fg.sftp_read("Downloads/bill.txt", buf, &len);
    if (ret)
    {
        string errmsg = fg.getLastError();
        cout << errmsg << endl;
        return -1;
    }
    cout << string(buf, len) << endl;

    ret = fg.download("Downloads/bill.txt", "bill.txt");
    if (ret)
    {
        string errmsg = fg.getLastError();
        cout << errmsg << endl;
        return -1;
    }

    return 0;
}
