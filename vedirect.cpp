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

#include <stdarg.h>
#include "vedirect.h"

static const char *TAG = "vedirect";


// VEDirectComponent::VEDirectComponent(UARTComponent* parent, int nbs, ...)
//   : UARTDevice(parent) {
//   int i=0;
//   nb_sensors=0;
//   va_list slist;
//   va_start(slist,nbs);
//   while (i<nbs) {
//     VESensor* s=new VESensor;
//     ve_sensors[nb_sensors++]=s;
//     s->_label=strdup(va_arg(slist,const char*));
//     s->_sensor=va_arg(slist,Sensor*);
//     s->_is_text=(typeid(s->_sensor)==typeid(TextSensor));
//     i+=2;
//     ESP_LOGD(TAG,"recorded %s sensor for label %s", s->is_text()?"text":"numeric", s->label());
//   }
//   va_end(slist);
// }

void VEDirectComponent::_addSensor(const char* label, Sensor* ss, bool is_text) {
  VESensor* s=new VESensor;
  s->_label=strdup(label);
  s->_sensor=ss;
  s->_is_text=is_text;
  ve_sensors[nb_sensors++]=s;
  ESP_LOGD(TAG,"recorded %s sensor for label %s", s->is_text()?"text":"numeric", s->label());
}

void VEDirectComponent::addSensor(const char* label, TextSensor* ss) {
  this->_addSensor(label,(Sensor*)ss,true);
}

void VEDirectComponent::addSensor(const char* label, Sensor* ss) {
  this->_addSensor(label,ss,false);
}

void VEDirectComponent::ReadVEData() {
  while (vedfh.veEnd<=0) {
    while (available()) {
      vedfh.rxData(read());
    }
  }
  yield();
}

void VEDirectComponent::update() {
  ReadVEData();
  int skip[max_sensors]={};
  for (int i=0; i<vedfh.veEnd; i++) {
    for (int j=0; j<nb_sensors; j++) {
      if (skip[j]==0 && strcmp(ve_sensors[j]->label(),vedfh.veName[i])==0) {
	skip[j]=1;
	VESensor* s=ve_sensors[j];
	Sensor* ss=s->sensor();
	if (s->is_text()) {
	  TextSensor* ts=(TextSensor*)ss;
	  ts->publish_state(vedfh.veValue[i]);
	  ESP_LOGD(TAG,"published state %s for text sensor %s", vedfh.veValue[i], s->label());
	}
	else {
	  float val=atof(vedfh.veValue[i]);
	  ss->publish_state(val);
	  ESP_LOGD(TAG,"published state %f for numeric sensor %s", val, s->label());
	}
	break;
      }
    }
  }
}


