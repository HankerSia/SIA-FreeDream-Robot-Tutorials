#!/bin/bash

rostopic list | grep "/rosout"
while [ $? == 1 ]
do
	rostopic list | grep "/rosout"
done

echo "12123321132"
