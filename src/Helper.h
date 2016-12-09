#ifndef MEKA_GUIDING_HELPER_H
#define MEKA_GUIDING_HELPER_H


#include "ros/ros.h"
#include "tf/transform_listener.h"
#include "trajectory_msgs/JointTrajectory.h"
#include <mutex>

class Helper {

public:
    /**     functions  **/
    void init(ros::NodeHandle nh);
    void setActZ(double act_z);
    void controlJoint();

private:
    /**     variables   **/
    ros::Publisher pub;
    tf::TransformListener listener;
    tf::StampedTransform transform;
    std::mutex act_z_mutex;
    tf::Vector3 init_pos, old_pos, new_pos;
    std::string topic_pub, input_joint, controlled_joint, tf_joint;
    bool sim;
    double act_z, new_z;

    /**     functions   **/
    void readParams(ros::NodeHandle nh);

};


#endif //MEKA_GUIDING_HELPER_H