#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sensor_msgs/Joy.h>
#include <geometry_msgs/Twist.h>

class fMoterControl
{
public:
    fMoterControl();
private:
    void joyCallback(const sensor_msgs::Joy::ConstPtr& joy);
    void velocityCallback(const geometry_msgs::Twist::ConstPtr& vel);

    ros::Subscriber velocity_sub_;
    ros::WallTime last_command_time_;
    float lin_vel_;
    float ang_vel_;

    FILE *fp;

    ros::NodeHandle nh_;
    int  forward_, angular_;
    ros::Subscriber joy_sub_;
};


fMoterControl::fMoterControl():
    forward_(1),
    angular_(0)
{
    velocity_sub_ = nh_.subscribe("cmd_vel", 1, &fMoterControl::velocityCallback, this);
    nh_.param("axis_forward", forward_, forward_);
    nh_.param("axis_angular", angular_, angular_);
    joy_sub_ = nh_.subscribe<sensor_msgs::Joy>("joy", 10, &fMoterControl::joyCallback, this);
}

void fMoterControl::velocityCallback(const geometry_msgs::Twist::ConstPtr& vel)
{
  last_command_time_ = ros::WallTime::now();
  lin_vel_ = vel->linear.x;
  ang_vel_ = vel->angular.z;

  printf("lin_vel_ %3lf\n",lin_vel_);
  printf("ang_vel_ %3lf\n",ang_vel_);
  

  double v0 = 400;
  double v1 = 30;
  int v = 0;

  if( (fp = fopen("/dev/ttyACM0", "w")) == NULL ) {//LRF
         printf("File open failed.\n");
         exit(EXIT_FAILURE);
  }


  if((ang_vel_*ang_vel_) > 0.0){//roll left
      if(ang_vel_ > 0.0 ){
        v = 120.0;
        fprintf(fp, "%d,%d,%d,%d\n", -v, -v, v, v);
        printf("%d,%d,%d,%d\n", -v, -v, v, v);
      }
      else{
        v = - 120.0;
        fprintf(fp,"%d,%d,%d,%d\n", -v, -v, v, v);
        printf("%d,%d,%d,%d\n", -v, -v, v, v);
      }
  }else if((lin_vel_*lin_vel_) > 0.0){// foward
      if(lin_vel_ > 0){
        v = lin_vel_ * v1 + 80.0;
        fprintf(fp, "%d,%d,%d,%d\n", v, v, v, v);
        printf("%d,%d,%d,%d\n", v, v, v, v);
      }
      else{
        v = lin_vel_ * v1 - 80.0;
        fprintf(fp,"%d,%d,%d,%d\n", v, v, v, v);
        printf("%d,%d,%d,%d\n", v, v, v, v);
      }
  }else{
      fprintf(fp, "%d,%d,%d,%d\n", 0, 0, 0, 0);
      printf("%d,%d,%d,%d\n", 0, 0, 0, 0);
  }
 
  fclose(fp);


}

void fMoterControl::joyCallback(const sensor_msgs::Joy::ConstPtr& joy)
{

    if( (fp = fopen("/dev/ttyACM0", "w")) == NULL ) {//LRF
           printf("File open failed.\n");
           exit(EXIT_FAILURE);
    }

    static int v = 100;
    int r = 100;
    int f = 0;

    //accel
    if(joy->buttons[1] == 1) v = v + 10;//1:x
    if(joy->buttons[0] == 1) v = v - 10;//0:y
    if(joy->buttons[2] == 1) v = 100;//2:b

    if( (int)(joy->axes[angular_]) == 1){//roll left
        fprintf(fp, "%d,%d,%d,%d\n", -v, -v, v, v);
    }else if( (int)(joy->axes[angular_]) == -1){// roll right
        fprintf(fp, "%d,%d,%d,%d\n", v, v, -v, -v);
    }else if( (int)(joy->axes[forward_]) == 1){// foward
        fprintf(fp, "%d,%d,%d,%d\n", v, v, v, v);
    }else if( (int)(joy->axes[forward_]) == -1){//back
        fprintf(fp, "%d,%d,%d,%d\n", -v, -v, -v, -v);
    }else{
        fprintf(fp, "%d,%d,%d,%d\n", 0, 0, 0, 0);
    }
    printf("speed = %d\n",v);

    fclose(fp);

}

int main(int argc, char **argv){

    ros::init(argc, argv, "f_moter_control");//node_name

    fMoterControl fmc;

    ros::spin();

    return 0;

}

