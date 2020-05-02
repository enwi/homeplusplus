import {OnDestroy} from '@angular/core';
import {Subscription} from 'rxjs';
import {scan} from 'rxjs/operators';

import {Device} from '../../devices/device';
import {DeviceService} from '../../devices/device.service';

import {DeviceMeta} from './../../devices/device';

export interface PropertyListController {
  getSelectedDeviceId(): number;
  getSelectedProperty(): string;
  onPropertyChange(key: string): void;
  resetProperty(): void;
}

export class PropertyListContainer implements OnDestroy {
  devices: Device[];
  properties: Map<string, any>;
  meta: DeviceMeta;
  private subscription: Subscription;
  private metaSubscription: Subscription;

  constructor(
      private deviceService: DeviceService,
      public controller: PropertyListController) {}

  ngOnDestroy() {
    if (this.subscription) {
      this.subscription.unsubscribe();
    }
  }

  public initialize(): void {
    if (!this.subscription) {
      this.subscription =
          this.deviceService.getDevices()
              .pipe(scan(
                  (list: Device[], device: Device) => {
                    const index = list.findIndex(d => d.id === device.id);
                    if (index !== -1) {
                      if (device.properties.size > 0) {
                        list[index] = device;
                      } else {
                        list.slice(index, 1);
                      }
                    } else if (device.properties.size > 0) {
                      list.push(device);
                    }
                    return list;
                  },
                  []))
              .subscribe(devices => this.updateDevices(devices));
    }
  }

  updateDevices(devices: Device[]): void {
    this.devices = devices;
    const deviceId = this.controller.getSelectedDeviceId();
    if (deviceId !== undefined) {
      const device = this.findDevice(deviceId);
      if (device) {
        const obs =
            this.deviceService.getDeviceMeta(device.type).subscribe(meta => {
              this.updateProperties(device.properties, meta);
              obs.unsubscribe();
            });
        // .subscribe(meta => {
        //   if (meta.entries.has(this.property)) {
        //     this.valueInput = meta.entries.get(this.property).dataType;
        //   }
        // });
      }
    }
  }

  updateProperties(properties: Map<string, any>, meta: DeviceMeta): void {
    const filteredProps = new Map<string, any>();
    properties.forEach((value, key) => {
      if (meta.entries.has(key) && meta.entries.get(key).getActionWrite()) {
        filteredProps.set(key, value);
      }
    });
    this.properties = filteredProps;
    this.meta = meta;
    const property = this.controller.getSelectedProperty();
    if (property !== undefined) {
      if (!this.properties.has(property)) {
        this.controller.resetProperty();
      } else {
        this.controller.onPropertyChange(property);
      }
    }
  }

  findDevice(id: number): Device {
    if (!this.devices) {
      return undefined;
    }
    return this.devices.find(d => d.id === id);
  }
}
