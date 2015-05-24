#!/bin/bash 

> users.db;for ((i=1;i<=9;i++)); do for ((j=1;j<=40;j++)); do echo -n $i >> users.db ; done; done
