#!/bin/sh

g++ sftp_get.cpp sftp_test.cpp -o sft_test -I./include -L./lib -lssh2

