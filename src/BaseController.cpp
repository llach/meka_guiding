#include "BaseController.h"

BaseController::BaseController() {

    //waitin, otherwise the first tf would always fail
    ROS_INFO("waiting for transform for .5s");
    listener.waitForTransform(tf_src, tf_dst, ros::Time::now(), ros::Duration(0.5));

    //initialize velocity variables
    x_vel = 0.0;
    y_vel = 0.0;

    f = boost::bind(&BaseController::parameterCallback, this, _1, _2);
    server.setCallback(f);

    calibrate();

}

//gets transform and sets the new position
geometry_msgs::Twist BaseController::controlJoint() {
    geometry_msgs::Twist twist;

    try{
        listener.lookupTransform(tf_src, tf_dst, ros::Time(0), transform);
        ROS_DEBUG("Got Transform from %s to %s!", tf_src.c_str(), tf_dst.c_str());
    }
    catch (tf::TransformException ex){
        ROS_ERROR("%s",ex.what());
        ros::Duration(1.0).sleep();
    }

    new_translation = transform.getOrigin();

    calcVelocity();

    twist.linear.x = x_vel;
    twist.linear.y = y_vel;

    return twist;
}

//calculates new velocities to set depending on the deflections of the input joint
void BaseController::calcVelocity(){

    //deflection using the distance of the vectorelements
    double x_dist = new_translation.getX() - initial_translation.getX();
    double y_dist = new_translation.getY() - initial_translation.getY();

    //for precise determination of the deadzone
    double dist = initial_translation.distance(new_translation);

    //calculation of x_vel
    if((new_translation.getX() > (initial_translation.getX() * (1 + DEADLOCK_SIZE))) ||
       (new_translation.getX() < (initial_translation.getX() * (1 - DEADLOCK_SIZE)))){

        x_vel = x_dist * VELOCITY_FACTOR;
    } else {
        x_vel = 0.0;
    }

    //calculation of y_vel
    if((new_translation.getY() > (initial_translation.getY() * (1 + DEADLOCK_SIZE))) ||
       (new_translation.getY() < (initial_translation.getY() * (1 - DEADLOCK_SIZE)))){

        y_vel = y_dist * VELOCITY_FACTOR;
    } else {
        y_vel = 0.0;
    }

    ROS_DEBUG_STREAM( "VelocityX (dist, vel):[" << x_dist << ", " << x_vel << "]");
    ROS_DEBUG_STREAM( "VelocityY (dist, vel):[" << y_dist << ", " << y_vel << "]");
    ROS_DEBUG_STREAM( "Translation{initial}: [" << initial_translation.getX() << ", " << initial_translation.getY() << ", " << initial_translation.getZ() << "]");
    ROS_DEBUG_STREAM( "Translation{actual}:  [" << new_translation.getX() << ", " << new_translation.getY() << ", " << new_translation.getZ() << "]");

}

//calibration ... what else?
void BaseController::calibrate(){

    //give calibration additional tries to avoid first tf lookup error
    for(int i = 1; i <= MAX_CALIBRATION_TRIES; i++){
        ROS_INFO("calibration try %i", i);
        if(lookupInitialTransform()){
            ROS_INFO("successfully calibrated");
        } else if( i == MAX_CALIBRATION_TRIES){
            ROS_FATAL("failed to lookupInitialTransform after %i tries", MAX_CALIBRATION_TRIES);
            ros::shutdown();
        }
    }

}

//sets initial position
bool BaseController::lookupInitialTransform() {

    ROS_DEBUG("calibrating...");

    try{
        listener.lookupTransform(tf_src, tf_dst, ros::Time(0), transform);
        ROS_DEBUG("got transform!");
    }
    catch (tf::TransformException ex){
        ROS_ERROR("error while calibrating: %s", ex.what());
        ros::Duration(1.0).sleep();
        return false;
    }

    initial_translation = transform.getOrigin();

    return true;
}

void BaseController::parameterCallback(meka_guiding::GuidingConfig &config, uint32_t level) {
    LINEAR_VELOCITY_UPPER = config.speed_lim_v;
    ANGULAR_VELOCITY_UPPER = config.speed_lim_w;
    VELOCITY_FACTOR = config.velocity_factor;
    DEADLOCK_SIZE = config.deadlock_zone;

    tf_src = config.tf_src.c_str();
    tf_dst = config.tf_dst.c_str();

    ROS_INFO(
            "\n#######ParameterCallback\nLINEAR_VELOCITY_UPPER:     %f \nANGULAR_VELOCITY_UPPER:    %f \nVELOCITY_FACTOR:           %f \nDEADLOCK_SIZE:             %f \nSOURCE_FRAME:              %s \nTARGET_FRAME:              %s\n#######",
            LINEAR_VELOCITY_UPPER, ANGULAR_VELOCITY_UPPER, VELOCITY_FACTOR, DEADLOCK_SIZE, tf_src.c_str(),
            tf_dst.c_str());

}