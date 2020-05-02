import { Directive, ViewContainerRef } from '@angular/core';

@Directive({
  selector: '[appDeviceDetail]'
})
export class DeviceDetailDirective {

  constructor(public viewContainerRef: ViewContainerRef) { }

}
