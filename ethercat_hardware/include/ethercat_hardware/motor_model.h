#ifndef ETHERCAT_HARDWARE_MOTOR_MODEL_H
#define ETHERCAT_HARDWARE_MOTOR_MODEL_H

#include <string>
#include <vector>

#include <realtime_tools/realtime_publisher.h>
#include <ethercat_hardware/MotorTraceSample.h>
#include <ethercat_hardware/MotorTrace.h>
#include <ethercat_hardware/ActuatorInfo.h>
#include <ethercat_hardware/BoardInfo.h>

#include <diagnostic_updater/DiagnosticStatusWrapper.h>

#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>

class MotorModel : private boost::noncopyable
{
public:
  MotorModel(unsigned trace_size);
  bool initialize(const ethercat_hardware::ActuatorInfo &actuator_info, 
                  const ethercat_hardware::BoardInfo &board_info);
  void publishTrace(const std::string &reason);
  void diagnostics(diagnostic_updater::DiagnosticStatusWrapper &d);
  void sample(const ethercat_hardware::MotorTraceSample &s);
  bool verify(std::string &reason) const;
  void reset();
protected:
  unsigned trace_size_;
  unsigned trace_index_; /* index of most recent element in trace vector */
  ethercat_hardware::ActuatorInfo actuator_info_;
  ethercat_hardware::BoardInfo board_info_;
  double backemf_constant_;
  bool previous_pwm_saturated_;
  std::vector<ethercat_hardware::MotorTraceSample> trace_buffer_;
  realtime_tools::RealtimePublisher<ethercat_hardware::MotorTrace> *publisher_;

  class SimpleFilter
  {
  public:
    SimpleFilter();
    void sample(double value, double filter_coefficient);
    void reset();
    double filter() const { return filtered_value_; }
  protected:
    double filtered_value_;
  };
  
  // Filters and keeps a record of max values
  class Filter : public SimpleFilter
  {
  public:
    Filter(double filter_coefficient);
    void sample(double value);
    void reset();
    double filter_max() const { return max_filtered_value_; }
  protected:
    double filter_coefficient_;
    double max_filtered_value_;
  };  

  // Lock around filter values, 
  // filter updates and diagnostic publishing are called from same different threads
  boost::mutex filter_mutex_; 
  Filter motor_voltage_error_;
  Filter abs_motor_voltage_error_;
  Filter measured_voltage_error_;
  Filter abs_measured_voltage_error_;
  Filter current_error_;
  Filter abs_current_error_;
  SimpleFilter motor_resistance_;

  // These filter is for indentifing source of errors not for calculations
  Filter abs_velocity_;  
  Filter abs_measured_current_;    
  Filter abs_board_voltage_;
  Filter abs_position_delta_;

};

#endif /* ETHERCAT_HARDWARE_MOTOR_MODEL_H */
