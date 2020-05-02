import {Injectable} from '@angular/core';

import {Device} from '../../devices/device';
import {DeviceParser} from '../../devices/device-parser';

@Injectable({providedIn: 'root'})
export class HueParseService implements DeviceParser {
  createDeviceInstance(type: string): Device|undefined {
    if (type === 'hueLight') {
      return new Device();
    }
    return undefined;
  }
  parse(json: any): Device|undefined {
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
