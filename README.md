# 通过代理访问SFTP服务器
sftp -v -o ProxyCommand='/usr/bin/nc -v -X connect  -x 192.168.6.156:9998 %h 22' qqstaging@32.64.205.89<br>
如果不想看到一大堆输出，两个-v选项都可以去掉。

# 把代理配置放在config文件中
创建~/.ssh/config文件，粘贴如下内容：
```
Host 32.64.205.89
    ProxyCommand        /usr/bin/nc -X connect -x 192.168.6.156:9998 %h 22
    ServerAliveInterval 10
```
现在命令行中就不需要指定代理选项，也可以访问SFTP服务器了：
sftp qqstaging@32.64.205.89

# 使用expect自动完成交互
## 记录交互过程（正常）
```
[justinshao@DevTJ-todo-1507091995 ~/tranglo/popen]$ sftp qqstaging@32.64.205.89
Connecting to 32.64.205.89...
qqstaging@32.64.205.89's password: 
sftp> cd TencentStaging
sftp> get DAILY_STATEMENT_ID_2017-12-30.csv
Fetching /TencentStaging/DAILY_STATEMENT_ID_2017-12-30.csv to DAILY_STATEMENT_ID_2017-12-30.csv
/TencentStaging/DAILY_STATEMENT_ID_2017-12-30.csv    100% 1422    1.4KB/s   00:00
sftp> quit
[justinshao@DevTJ-todo-1507091995 ~/tranglo/popen]$
```
## 记录交互过程（包含异常情况）
```
[justinshao@DevTJ-todo-1507091995 ~]$ sftp qqstaging@32.64.205.89
Connecting to 32.64.205.89...
qqstaging@32.64.205.89's password: 
Permission denied, please try again.
qqstaging@32.64.205.89's password: 
sftp> cd TencentStagin
Couldn't canonicalise: No such file or directory
sftp> cd TencentStaging
sftp> get DAILY_STATEMENT_ID_2018-12-30.csv 
Couldn't stat remote file: No such file or directory
File "/TencentStaging/DAILY_STATEMENT_ID_2018-12-30.csv" not found.
sftp> get DAILY_STATEMENT_ID_2017-12-30.csv 
Fetching /TencentStaging/DAILY_STATEMENT_ID_2017-12-30.csv to DAILY_STATEMENT_ID_2017-12-30.csv
/TencentStaging/DAILY_STATEMENT_ID_2017-12-30.csv    100% 1422  1.4KB/s  00:00
sftp> quit
[justinshao@DevTJ-todo-1507091995 ~]$
```
## 根据交互过程编写expect脚本
文件sftp_get.exp
```shell
#!/usr/bin/expect --

set timeout  600
set FILE_NAME  [lindex $argv 0]
# set FILE_NAME  [lindex $argv 1]

spawn sftp qqstaging@32.64.205.89
expect {
  "(yes/no)?" {send "yes\r"; expect_continue}
  "*password:" {send "alex123!@#\r"}
}

expect {
  "*please try again*" {exit 1}
  "sftp>" {send "cd TencentStaging\r"}
}

expect {
  "*No such file or directory*" {exit 2}
  "sftp>" {send "get $FILE_NAME\r"}
}

expect {
  "*No such file or directory" {exit 3}
  "sftp>" {send "quit\r"}
}

expect eof
exit 0
```
## 测试expect脚本
```
[justinshao@DevTJ-todo-1507091995 ~/tranglo/popen]$ ./sftp_get.exp DAILY_STATEMENT_ID_2017-12-30.csv
spawn sftp qqstaging@32.64.205.89
Connecting to 32.64.205.89...
qqstaging@32.64.205.89's password: 
sftp> cd TencentStaging
sftp> get DAILY_STATEMENT_ID_2017-12-30.csv
Fetching /TencentStaging/DAILY_STATEMENT_ID_2017-12-30.csv to DAILY_STATEMENT_ID_2017-12-30.csv
/TencentStaging/DAILY_STATEMENT_ID_2017-12-30.csv    100%  1422    1.4KB/s  00:00    
sftp> quit
[justinshao@DevTJ-todo-1507091995 ~/tranglo/popen]$ 
```

# 在C中调用expect脚本
```c++
#include <stdio.h>

int main(int argc, char** argv) {
  FILE* fp = popen("./sftp_get.exp DAILY_STATEMENT_ID_2017-12-30.csv", "r");
  if (!fp) {
    printf("popen failed!");
    return 1;
  }
  char line[10240];
  while (fgets(line, 10240, fp)) {
    printf(line);
  }
  pclose(fp);
  return 0;
}
```
