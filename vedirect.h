/* 
   esphome vedirect component 
   Copyright (C) 2021 philippe@giacinti.com
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef __VEDIRECT_H
#define __VEDIRECT_H

#include "esphome.h"
using namespace esphome;
using namespace sensor;
using namespace uart;
using namespace text_sensor;
#include "VeDirectFrameHandler.h"

class VEDirectComponent : public PollingComponent, public UARTDevice {
public:
 VEDirectComponent(UARTComponent* parent) : UARTDevice(parent) {}

  void addSensor(const char*, TextSensor*);
  void addSensor(const char*, Sensor*);

  void setup() override {}
  void loop() override {}
  void update() override;

protected:

  struct VESensor {
    char* _label;
    Sensor* _sensor;
    bool _is_text;

  VESensor() : _label(0), _sensor(0), _is_text(false) {}
    ~VESensor();

    inline const char* label() {return _label;}
    inline Sensor* sensor() {return _sensor;}
    inline const bool is_text() {return _is_text;}
  };
  
  static const int max_sensors=60;
  VESensor* ve_sensors[max_sensors] = {NULL};
  unsigned int nb_sensors;
  void _addSensor(const char*, Sensor*, bool);

  VeDirectFrameHandler vedfh;
  void ReadVEData();
};

#endif
