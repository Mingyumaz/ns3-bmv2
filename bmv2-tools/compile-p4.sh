#!/bin/bash
# I do not think this is a good way to compile the p4 file.
PROG_NAME=${1}

#sudo docker run -w /p4c/build -v ${PWD}:/tmp p4c p4c-bm2-ss -I/tmp --p4v 16 -o /tmp/${PROG_NAME}.json /tmp/${PROG_NAME}.p4
sudo p4c-bm2-ss --p4v 16 --p4runtime-files ${PROG_NAME}.p4.p4info.txt -I ${PWD} -o ${PROG_NAME}.json ${PROG_NAME}.p4
sudo chown ${USER}:${USER} ${PROG_NAME}.json