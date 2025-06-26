#include <mruby.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>

#include <uros_network_interfaces.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <sensor_msgs/msg/joy.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#define TAG "uros"

#ifndef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
#define CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
#endif

//#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
#include <rmw_microros/rmw_microros.h>
//#endif

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

rcl_allocator_t allocator;
rclc_support_t support;
rcl_init_options_t init_options;
rcl_node_t node;

rcl_publisher_t publisher;
rclc_executor_t executor;

std_msgs__msg__Int32 msg;
sensor_msgs__msg__Joy joy_msg;

static mrb_value
mrb_esp32_microros_mod_init(mrb_state *mrb, mrb_value self)
{
  ESP_LOGI(TAG, "mrb_esp32_microros_mod_init");
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

  ESP_LOGI(TAG, "rcl_get_zero_initialized_init_options");
	rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  ESP_LOGI(TAG, "rcl_init_options_init");
	RCCHECK(rcl_init_options_init(&init_options, allocator));

//#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
  ESP_LOGI(TAG, "rcl_init_options_get_rmw_init_options");
	rmw_init_options_t* rmw_options = rcl_init_options_get_rmw_init_options(&init_options);

	// Static Agent IP and port can be used instead of autodisvery.
  const char* ip = "192.168.10.91";
  const char* port = "8888";
  ESP_LOGI(TAG, "rmw_uros_options_set_udp_address = %s %s",ip,port);
	//RCCHECK(rmw_uros_options_set_udp_address(CONFIG_MICRO_ROS_AGENT_IP, CONFIG_MICRO_ROS_AGENT_PORT, rmw_options));
	RCCHECK(rmw_uros_options_set_udp_address("192.168.10.91", "8888", rmw_options));
	//RCCHECK(rmw_uros_discover_agent(rmw_options));
//#endif

	// create init_options
  ESP_LOGI(TAG, "rclc_support_init_with_options");
	RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

	// create node
  // input:node name
	RCCHECK(rclc_node_init_default(&node, "mruby_joy_publisher", "", &support));

	// create publisher
  // input:topic name
	RCCHECK(rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Joy),
		"joy"));

	// create timer,
	// rcl_timer_t timer;
	// const unsigned int timer_timeout = 1000;
	// RCCHECK(rclc_timer_init_default(
	// 	&timer,
	// 	&support,
	// 	RCL_MS_TO_NS(timer_timeout),
	// 	timer_callback));

	// create executor
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	//RCCHECK(rclc_executor_add_timer(&executor, &timer));

}


static mrb_value
mrb_esp32_microros_spin_some(mrb_state *mrb, mrb_value self)
{
  ESP_LOGI(TAG, "mrb_esp32_microros_spin_some");
  mrb_int time;
  mrb_get_args(mrb, "i", &time);

  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(time));
  return mrb_nil_value();
}

static mrb_value
mrb_esp32_microros_publish(mrb_state *mrb, mrb_value self)
{
  ESP_LOGI(TAG, "mrb_esp32_microros_publish");
  // ## joy_msg
  // # The timestamp is the time at which data is received from the joystick.
  // std_msgs/Header header
  // # The axes measurements from a joystick.
  // float32[] axes
  // # The buttons measurements from a joystick.
  // int32[] buttons
  mrb_float axis0;
  mrb_float axis1;
  mrb_get_args(mrb, "ff", &axis0, &axis1);

  sensor_msgs__msg__Joy__init(&joy_msg);
  joy_msg.header.stamp.sec = 0;
  joy_msg.header.stamp.nanosec = 0;
  joy_msg.header.frame_id.data = malloc(strlen("joy_frame") + 1);
  strcpy(joy_msg.header.frame_id.data, "joy_frame");
  joy_msg.header.frame_id.size = strlen("joy_frame");
  joy_msg.header.frame_id.capacity = strlen("joy_frame") + 1;

  joy_msg.buttons.size = 2;
  joy_msg.buttons.capacity = 2;
  joy_msg.buttons.data = malloc(sizeof(int32_t) * 2);
  joy_msg.buttons.data[0] = 0;
  joy_msg.buttons.data[1] = 0;

  joy_msg.axes.size = 2;
  joy_msg.axes.capacity = 2;
  joy_msg.axes.data = malloc(sizeof(float) * 2);
  joy_msg.axes.data[0] = axis0;
  joy_msg.axes.data[1] = axis1;

  RCSOFTCHECK(rcl_publish(&publisher, &joy_msg, NULL));
  
  sensor_msgs__msg__Joy__fini(&joy_msg);
  return mrb_nil_value();
}


void
mrb_mruby_esp32_microros_gem_init(mrb_state* mrb)
{
  struct RClass *esp32, *uros_mod;

  esp32 = mrb_define_module(mrb, "ESP32");

  // Micro ROS module
  // init
  // register_data_type  ????
  // spin 引数 MicroRosNode
  uros_mod = mrb_define_module_under(mrb, esp32, "MicroROS");

  mrb_define_module_function(mrb, uros_mod, "init", mrb_esp32_microros_mod_init, MRB_ARGS_REQ(0));
  //
  mrb_define_module_function(mrb, uros_mod, "spin_some", mrb_esp32_microros_spin_some, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, uros_mod, "publish", mrb_esp32_microros_publish, MRB_ARGS_REQ(1));

  // Micro ROS Node class
  //class MicroRosNode
  //method
  // init
  //   
  // create_publisher
  // create_subscription
  // uros_node = mrb_define_class_under(mrb, uros_mod, "MicroROSNode", mrb->object_class);

  // mrb_define_method(mrb, uros_node, "_init", mrb_esp32_microros_node_init, MRB_ARGS_REQ(1));

  // mrb_define_method(mrb, i2c, "deinit", mrb_esp32_i2c_deinit, MRB_ARGS_NONE());
  // mrb_define_method(mrb, i2c, "send", mrb_esp32_i2c_send, MRB_ARGS_REQ(2));

}

void
mrb_mruby_esp32_microros_gem_final(mrb_state* mrb)
{
  // RCCHECK(rcl_publisher_fini(&publisher, &node));
	// RCCHECK(rcl_node_fini(&node));
}






