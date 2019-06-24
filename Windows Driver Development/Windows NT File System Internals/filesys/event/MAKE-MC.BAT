@echo off
REM The following explain the options used :
REM  v = verbose
REM  c = set Customer bit in all message ids.
REM  s = insert symbolic name as first line of each message.
REM  h <pathname> = pathname where (created) header file should be stored.
REM  U = output file should be in Unicode.

\mstools-351\bin\mc -vcs -h ..\inc -U errmsg.msg
