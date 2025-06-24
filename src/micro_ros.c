#include <mruby.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>

#include <uros_network_interfaces.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
#include <rmw_microros/rmw_microros.h>
#endif

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

// rcl_publisher_t publisher;
// std_msgs__msg__Int32 msg;

//


static mrb_value
mrb_esp32_microros_mod_init(mrb_state *mrb, mrb_value self)
{
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
	RCCHECK(rcl_init_options_init(&init_options, allocator));

#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
	rmw_init_options_t* rmw_options = rcl_init_options_get_rmw_init_options(&init_options);

	// Static Agent IP and port can be used instead of autodisvery.
	RCCHECK(rmw_uros_options_set_udp_address(CONFIG_MICRO_ROS_AGENT_IP, CONFIG_MICRO_ROS_AGENT_PORT, rmw_options));
	//RCCHECK(rmw_uros_discover_agent(rmw_options));
#endif

	// create init_options
	RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));
}

void
mrb_mruby_esp32_microros_gem_init(mrb_state* mrb)
{
  struct RClass *esp32, *uros;

  esp32 = mrb_define_module(mrb, "ESP32");

  // Micro ROS module
  // init
  // register_data_type  ????
  // spin 引数 MicroRosNode
  uros_mod = mrb_define_module_under(mrb, esp32, "MicroROS");

  mrb_define_method(mrb, uros_mod, "init", mrb_esp32_microros_mod_init, MRB_ARGS_REQ(1));

  // Micro ROS Node class
  //class MicroRosNode
  //method
  // init
  //   
  // create_publisher
  // create_subscription
  uros_node = mrb_define_class_under(mrb, uros_mod, "MicroROSNode", mrb->object_class);

  mrb_define_method(mrb, uros_node, "_init", mrb_esp32_microros_node_init, MRB_ARGS_REQ(1));

  // mrb_define_method(mrb, i2c, "deinit", mrb_esp32_i2c_deinit, MRB_ARGS_NONE());
  // mrb_define_method(mrb, i2c, "send", mrb_esp32_i2c_send, MRB_ARGS_REQ(2));

}

void
mrb_mruby_esp32_microros_gem_final(mrb_state* mrb)
{
}






