#!/bin/bash

#gnome-terminal:open a new terminal; -x: the following string is commond needed be execute; bash -c: avoid terminal exit suddenly; read: when press enter terminal exit 

# 1 start roscore, 
gnome-terminal -x bash -c "./roscore.sh; read;"
sleep 3

# 2 start kinect
gnome-terminal -x bash -c "roslaunch kinect2_bridge kinect2_bridge.launch; read;"
gnome-terminal -x bash -c "rosrun darknet_ros darknet_ros_node; read;"

# 3 start jaco
gnome-terminal -x bash -c "rosrun jaco_moveit_control main.sh; read;"

# 4 start finger
gnome-terminal -x bash -c "roslaunch beginner_tutorials bhand_can_axis_control.launch; read;"

# 5 start ugv
gnome-terminal -x bash -c "rosrun navigation_task navigation-start.sh; read;"

# 6 wait keydown
echo "Wait keydown(s + enter): "
read skey
while [ "$skey" != "s" ]
do
	read skey
done

# 7 start main
gnome-terminal -x bash -c "rosrun beginner_tutorials ge_test; read;"

