import { Component, OnInit, HostListener } from '@angular/core';
import { Device } from '../device';
import { DeviceService } from '../device.service';
import { Observable } from 'rxjs';
import { scan } from 'rxjs/operators';
import { Router } from '@angular/router';

@Component({
  selector: 'app-device-list',
  templateUrl: './device-list.component.html',
  styleUrls: ['./device-list.component.scss']
})
export class DeviceListComponent implements OnInit {
  devices$: Observable<Device[]>;

  selectedDevice: Device;

  constructor(private deviceService: DeviceService, private router: Router) { }

  ngOnInit() {
    this.getDevices();
  }

  getDevices(): void {
    // Transform to array, replace repeated devices
    this.devices$ = this.deviceService.getDevicesAsArray();
  }

  onDeviceKeyDown(event: KeyboardEvent, id: number) {
    if (event.key === 'Enter' || event.key === ' ') {
      this.router.navigate(['/device', id]);
    }
  }
}
