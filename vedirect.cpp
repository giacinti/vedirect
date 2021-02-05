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
static const char *HEX_LABEL = "HEX";

static void static_log_e(const char* module, const char* error) {
  esp_log_printf_(ESPHOME_LOG_LEVEL_DEBUG, module, 0, "%s", error);
}

static void static_hex_cb(const char* buffer, int size, void* obj) {
  ((VEDirectComponent*)obj)->hexCallback(buffer, size);
}

VEDirectComponent::VEDirectComponent(UARTComponent* parent)
  : UARTDevice(parent),
    nb_sensors(0),
    hexSensorIndex(-1),
    hexAsyncMute(false)
{
  vedfh.setErrorHandler(&static_log_e);
}

void VEDirectComponent::_addSensor(const char* label, Sensor* ss, bool is_text) {
  if (strcmp(label, HEX_LABEL)==0) {
    hexSensorIndex=nb_sensors;
    vedfh.addHexCallback(&static_hex_cb, (void*)this);
    ESP_LOGD(TAG,"recorded callback for HEX sensor, index=%d",hexSensorIndex);
  }
  
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
  while (available()) {
    vedfh.rxData(read());
  }
  yield();
}

void VEDirectComponent::loop() {
  if (available())
    ReadVEData();
}

void VEDirectComponent::update() {
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
	  //ESP_LOGD(TAG,"published state %s for text sensor %s", vedfh.veValue[i], s->label());
	}
	else {
	  float val=atof(vedfh.veValue[i]);
	  ss->publish_state(val);
	  //ESP_LOGD(TAG,"published state %f for numeric sensor %s", val, s->label());
	}
	break;
      }
    }
  }
}

void VEDirectComponent::hexCallback(const char* buffer, int size) {
  if (hexSensorIndex!=-1 && (buffer[1]!='A' || hexAsyncMute==false)) {
    char tmpdata[size+1];
    memcpy(tmpdata,buffer,size);
    tmpdata[size]=0; // end of string
    ((TextSensor*)(ve_sensors[hexSensorIndex]->sensor()))->publish_state(tmpdata);
    ESP_LOGD(TAG,"hex callback received, published hex frame %s", tmpdata);
  }
}

void VEDirectComponent::sendHexCommand(const char* command) {
  ESP_LOGD(TAG,"send hex command %s", command);
  write_str(command);
}
