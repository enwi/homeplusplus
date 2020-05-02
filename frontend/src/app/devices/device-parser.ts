import { Injectable } from '@angular/core';
import { Device } from './device';

export interface DeviceParser {
  createDeviceInstance(type: string): Device | undefined;

  parse(json: any): Device | undefined;
}
