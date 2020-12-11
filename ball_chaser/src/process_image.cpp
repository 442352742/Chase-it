#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <iostream>     // std::cout
#include <algorithm> 
#include <std_msgs/Float64.h>
#include <vector>



// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    
    ROS_INFO_STREAM("Sending new command");
    
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    
    if (!client.call(srv)){
        ROS_ERROR("Failed to call service command_robot");
    }	
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    int left_counter = 0;
    int mid_counter = 0;
    int right_counter = 0;
    int loc = 0;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
     for (int i = 0; i < img.height * img.step; i += 3) {
        int position = i % (img.width * 3) / 3;
        if (img.data[i] == white_pixel && img.data[i + 1] == white_pixel && img.data[i + 2] == white_pixel ) {
            if(position <= 265) {
		left_counter += 1;                
            }
            if(position > 265 && position <= 533) {
		mid_counter += 1;               
            }
            if(position > 533) {
		right_counter += 1;                
            }
	}
    }
    
    // Locate the ball
    std::vector<int> pos_counter{left_counter, mid_counter, right_counter};
    loc = *std::max_element(pos_counter.begin(), pos_counter.end());
    
    //Run
    if (loc == 0){
        drive_robot(0.0, 0.2); // try to look around
    }
    else if (loc == left_counter) {
	drive_robot(0.0, 0.5);  
    }
    else if (loc == mid_counter) {
        drive_robot(0.5, 0.0);  
    }
    else if (loc == right_counter) {
        drive_robot(0.0, -0.5);
    }
    
    
    
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
