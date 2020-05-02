import {Location} from '@angular/common';
import {Component, Inject, Input, OnDestroy, OnInit, Type, ViewChild, ViewContainerRef} from '@angular/core';
import {ComponentFactoryResolver} from '@angular/core';
import {ActivatedRoute} from '@angular/router';
import {Observable, Subscription} from 'rxjs';

import {Device} from '../device';
import {DEVICE_DETAILS} from '../device.config';
import {DeviceService} from '../device.service';

import {DeviceDetail} from './device-detail';
import {DeviceDetailDirective} from './device-detail.directive';

@Component({
  selector: 'app-device-detail',
  templateUrl: './device-detail.component.html',
  styleUrls: ['./device-detail.component.css']
})
export class DeviceDetailComponent implements OnInit, OnDestroy {
  deviceName = '';
  deviceNotFound = false;
  @ViewChild(DeviceDetailDirective, {static: false})
  deviceDetail: DeviceDetailDirective;

  private typeMap: Map<string, Type<DeviceDetail>>;
  private propertyChangeSubscription: Subscription;

  constructor(
      private deviceService: DeviceService, private route: ActivatedRoute,
      private componentFactoryResolver: ComponentFactoryResolver,
      @Inject(DEVICE_DETAILS) private types: [string, Type<DeviceDetail>][]) {
    this.typeMap = new Map(types);
  }

  ngOnInit() {
    this.getDevice();
  }

  ngOnDestroy() {
    if (this.propertyChangeSubscription) {
      this.propertyChangeSubscription.unsubscribe();
    }
  }

  getDevice() {
    const id: number = +this.route.snapshot.paramMap.get('id');
    const observer = this.deviceService.getDevice(id).subscribe(device => {
      observer.unsubscribe();
      this.loadComponent(
          device, device ? this.typeMap.get(device.type) : undefined);
    });
  }

  private loadComponent(device: Device, deviceType: Type<DeviceDetail>) {
    const viewContainerRef = this.deviceDetail.viewContainerRef;
    viewContainerRef.clear();
    if (deviceType && device) {
      this.deviceNotFound = false;
      const componentFactory =
          this.componentFactoryResolver.resolveComponentFactory(deviceType);

      const componentRef = viewContainerRef.createComponent(componentFactory);
      componentRef.instance.setDevice(device);
      this.propertyChangeSubscription =
          this.deviceService.getDevicePropertyChanges(device.id).subscribe(
              propertyChange =>
                  componentRef.instance.onPropertyChange(propertyChange))
    } else {
      this.deviceNotFound = true;
    }
  }
}
