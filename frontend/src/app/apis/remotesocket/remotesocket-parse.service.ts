import { Injectable } from '@angular/core';
import { DeviceParser } from '../../devices/device-parser';
import { Device } from '../../devices/device';

@Injectable({
  providedIn: 'root'
})
export class RemoteSocketParseService implements DeviceParser {

  createDeviceInstance(type: string): Device | undefined {
    if (type === 'remoteSocket') {
      return new Device();
    }
    return undefined;
  }
  parse(json: any): Device | undefined {
    if (json) {
      const d = this.createDeviceInstance(json.type);
      if (d) {
        d.parse(json);
        return d;
      }
    }
    return undefined;
  }
}
