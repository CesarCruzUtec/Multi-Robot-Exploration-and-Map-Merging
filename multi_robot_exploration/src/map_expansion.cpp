/// \file
/// \brief This node published on <robot_ns>/map topic in order to manipulate the map sizes to be the same
///
/// PUBLISHES:
///     /tb3_0/map (nav_msgs/OccupancyGrid): Publishes a new width, height and origin
///     /tb3_1/map (nav_msgs/OccupancyGrid): Publishes a new width, height and origin

#include "ros/ros.h"
#include <nav_msgs/OccupancyGrid.h>
// #include <string>
#include <vector>
// #include <math.h>
#include "std_msgs/Header.h"
#include "nav_msgs/MapMetaData.h"

// define global message 
nav_msgs::OccupancyGrid map0_msg, map1_msg, slam0_map, slam1_map;


/// / \brief Grabs the position of the robot from the pose subscriber and stores it
/// / \param msg - pose message
/// \returns nothing
void map0Callback(const nav_msgs::OccupancyGrid::ConstPtr & msg)
{
  map0_msg.header = msg->header;
  map0_msg.info = msg->info;
  map0_msg.data = msg->data;
  slam0_map = map0_msg;
}

/// / \brief Grabs the position of the robot from the pose subscriber and stores it
/// / \param msg - pose message
/// \returns nothing
void map1Callback(const nav_msgs::OccupancyGrid::ConstPtr & msg)
{
  map1_msg.header = msg->header;
  map1_msg.info = msg->info;
  map1_msg.data = msg->data;
  slam1_map = map1_msg;
}


int main(int argc, char * argv[])
{
  ros::init(argc, argv, "map_metadata_node");
  ros::NodeHandle nh;

  // Create the initpose publisher and subscriber
  const auto new_tb3_0_map_pub = nh.advertise<nav_msgs::OccupancyGrid>("new_tb3_0_map", 100);
  const auto map_meta0_sub = nh.subscribe<nav_msgs::OccupancyGrid>("tb3_0/map", 100, map0Callback);

  const auto new_tb3_1_map_pub = nh.advertise<nav_msgs::OccupancyGrid>("new_tb3_1_map", 100);
  const auto map_meta1_sub = nh.subscribe<nav_msgs::OccupancyGrid>("tb3_1/map", 100, map1Callback);

  ros::Rate loop_rate(100);

  while (ros::ok())
  {
    if ( (map0_msg.data.size() != 0) || (map1_msg.data.size() != 0) || map0_msg.info.origin.position.x !=0)
    {
        nav_msgs::OccupancyGrid new_tb3_0_map, new_tb3_1_map;
        new_tb3_0_map.header.frame_id = "new_tb3_0_map";
        new_tb3_0_map.info.resolution = 0.05;
        new_tb3_0_map.info.origin.position.x =  -10.0;
        new_tb3_0_map.info.origin.position.y = -10.0;
        new_tb3_0_map.info.origin.position.z = 0.0;
        new_tb3_0_map.info.origin.orientation.w = 0.0;

        new_tb3_1_map.header.frame_id = "new_tb3_1_map";
        new_tb3_1_map.info.resolution = 0.05;
        new_tb3_1_map.info.origin.position.x =  new_tb3_0_map.info.origin.position.x;
        new_tb3_1_map.info.origin.position.y = new_tb3_0_map.info.origin.position.y;
        new_tb3_1_map.info.origin.position.z = new_tb3_0_map.info.origin.position.z;
        new_tb3_1_map.info.origin.orientation.w = new_tb3_0_map.info.origin.orientation.w;

        // Set the new map width and heights 
        const size_t width_ = 384;
        const size_t height_ = 384;
        new_tb3_0_map.info.width = width_;
        new_tb3_0_map.info.height = height_;
        new_tb3_1_map.info.width = width_;
        new_tb3_1_map.info.height = height_;

        // Determine how much space to fill in with unknown cells bewteen bottom of the new sized map and the original slam map
        const size_t bottom0_width_ = (map0_msg.info.origin.position.x - new_tb3_0_map.info.origin.position.x) / new_tb3_0_map.info.resolution;
        const size_t bottom1_width_ = (map1_msg.info.origin.position.x - new_tb3_1_map.info.origin.position.x) / new_tb3_1_map.info.resolution;

        const size_t bottom0_height_ = (map0_msg.info.origin.position.y - new_tb3_0_map.info.origin.position.y) / new_tb3_0_map.info.resolution;
        const size_t bottom1_height_ = (map1_msg.info.origin.position.y - new_tb3_1_map.info.origin.position.y) / new_tb3_1_map.info.resolution;

        /////////////////////////////////////////////////////////////////////////////
        // For the frist turtlebot (tb3_0)
        ////////////////////////////////////////////////////////////////////////////
        // Map starts loading in from origin in bottom right
        // From the origin, the row components corresponds with width (x-dir which is actually up)
        // Create empty space on rhs of map (total width and 122 pixels 'high' (to the left))

        int c0 = 0; // start a counter for map0

        // This fills in the right hand side of the new map to the slam map
        for (int i=0;  i < new_tb3_0_map.info.width * bottom0_height_; i++)
        {
          new_tb3_0_map.data.push_back(-1);
        }

        // For the new larger map, fill in the spaces next to the slam map with -1s
        for (int item_counter=0; item_counter < slam0_map.info.height; item_counter++)
        {

          for (int q=0; q < bottom0_width_; q++)
          {
            new_tb3_0_map.data.push_back(-1);
          }

          for (int a = 0; a < slam0_map.info.width; a++)
          {
            new_tb3_0_map.data.push_back(slam0_map.data[c0]);
            c0++;
          }

          for (int u=0; u < (new_tb3_0_map.info.width - slam0_map.info.width - bottom0_width_); u++)
          {
            new_tb3_0_map.data.push_back(-1);
          }
        } 

        // Fill in the left hand side of the new map
        for (int z=0;  z < ((height_ - slam0_map.info.height - bottom0_height_) * new_tb3_0_map.info.width); z++)
        {
          new_tb3_0_map.data.push_back(-1);
        }

        /////////////////////////////////////////////////////////////////////////////
        // For the second turtlebot (tb3_1)
        ////////////////////////////////////////////////////////////////////////////
        // Map starts loading in from origin in bottom right
        // From the origin, the row components corresponds with width (x-dir which is actually up)
        // Create empty space on rhs of map (total width and 122 pixels 'high' (to the left))

        int c1 = 0; // start a counter for map 2

        // Fill in the empty space on the right hand side of the map
        for (int i=0;  i < new_tb3_1_map.info.width *  bottom1_height_; i++)
        {
          new_tb3_1_map.data.push_back(-1);
        }

        // Fill in the area where the map1 is, also the area above and below that
        for (int item_counter=0; item_counter < slam1_map.info.height; item_counter++)
        {
          // Space below the maps width
          for (int q=0; q < bottom1_width_; q++)
          {
            new_tb3_1_map.data.push_back(-1);
          }

          // Fill in the map data for the current width row
          for (int a = 0; a < slam1_map.info.width; a++)
          {
            new_tb3_1_map.data.push_back(slam1_map.data[c1]);
            c1++;
          }

          // Fill in the space above the map
          for (int u=0; u < (new_tb3_1_map.info.width - slam1_map.info.width - bottom1_width_); u++)
          {
            new_tb3_1_map.data.push_back(-1);
          }
        } 

        // Fill in the area on the left hand side of the map
        for (int z=0;  z < ((height_ - slam1_map.info.height -  bottom1_height_) * new_tb3_1_map.info.width); z++)
        {
          new_tb3_1_map.data.push_back(-1);
        }

        new_tb3_0_map_pub.publish(new_tb3_0_map);
        new_tb3_1_map_pub.publish(new_tb3_1_map);
    }

      ros::spinOnce();
      loop_rate.sleep();
  }

  return 0;
}