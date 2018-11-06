#!/bin/zsh
gcc -pthread tserver.c -o server
gcc -pthread tclient.c -o client

