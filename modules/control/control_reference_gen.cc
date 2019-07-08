#include <memory>

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>
#include "cyber/class_loader/class_loader.h"
#include "cyber/component/component.h"
#include "cyber/cyber.h"
#include "cyber/time/rate.h"
#include "modules/control/proto/chassis.pb.h"
#include "modules/control/proto/control.pb.h"
#include "modules/sensors/proto/sensors.pb.h"

struct Point {
  double x, z, y;
};

std::vector<Point> new_vector;
int trajectory_reader(void) {
  char buffer[100000];
  std::string file_name = "/home/raosiyue/pose.dat";

  std::ifstream input_file(file_name, std::ios::in | std::ios::binary);
  input_file.seekg(0, std::ios_base::end);
  auto size_v = input_file.tellg();
  input_file.seekg(0, std::ios_base::beg);
  Point temp_vec;

  // buffer = (char*)malloc(size_v);
  std::cout << "length is " << size_v << std::endl;
  input_file.read(buffer, size_v);
  // std::cout << "read result:" << result << std::endl;
  input_file.close();
  for (int i = 0; i < size_v; i += 24) {
    double test_double = 0;
    memcpy(&test_double, buffer, 8);
    if ((buffer + i) > 100000) {
      break;
    }
    std::memcpy(&temp_vec, buffer + i, 24);
    new_vector.push_back(temp_vec);
  }

  for (const auto& v : new_vector) {
    std::cout << v.x << ", " << v.z << " , " << v.y << std::endl;
  }
  return 0;
}

int near_pt_find(float x, float y) {
  int ret = -1;
  float min_dist = 100;
  for (int i = 0; i < new_vector.size(); i++) {
    float dist = abs(new_vector.at(i).x - x) + abs(new_vector.at(i).z - y);
    if ((dist < min_dist) && (dist < 1.5)) {
      min_dist = dist;
      ret = i;
    }
  }
  return ret;
}

using apollo::control::Chassis;
using apollo::control::Control_Command;
using apollo::control::Control_Reference;
using apollo::cyber::Rate;
using apollo::cyber::Time;
using apollo::sensors::Pose;

int main(int argc, char* argv[]) {
  Pose pose_;
  trajectory_reader();
  apollo::cyber::Init("control ref gen");
  // create talker node
  auto talker_node = apollo::cyber::CreateNode("control ref gen");
  // create talker
  // auto talker = talker_node->CreateWriter<Chatter>("channel/chatter");
  auto control_refs_writer_ =
      talker_node->CreateWriter<Control_Reference>("/control_reference");
  auto pose_reader_ = node_->CreateReader<Pose>(
      "/realsense/pose",
      [this](const std::shared_ptr<Pose>& Pose) { pose_.CopyFrom(*Pose); });
  Rate rate(20.0);
  double t = 0;
  auto cmd = std::make_shared<Control_Reference>();
  while (apollo::cyber::OK()) {
    cmd->set_angular_speed(static_cast<float>(sin(t / 2.0)));
    cmd->set_vehicle_speed(static_cast<float>(0.4));
    t += 0.05;
    control_refs_writer_->Write(cmd);
    rate.Sleep();
  }

  // async_action_ = cyber::Async(&ControlComponent::TestCommand, this);
  return 0;
}