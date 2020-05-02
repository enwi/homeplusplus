import { Component, OnInit, EventEmitter } from '@angular/core';
import { SubActionView } from './sub-action-view';
import { PropertyToggle } from './sub-actions';
import { Observable } from 'rxjs';
import { Device } from '../../devices/device';
import { DeviceService } from '../../devices/device.service';
import { SubAction } from '../action';

@Component({
  selector: 'app-actor-toggle',
  templateUrl: './actor-toggle.component.html',
  styleUrls: ['./actor-toggle.component.css', './sub-action.css']
})
export class ActorToggleComponent implements OnInit, SubActionView {

  subAction: PropertyToggle;
  device$: Observable<Device>;
  edit: EventEmitter<void>;
  delete: EventEmitter<void>;

  constructor(private deviceService: DeviceService) { }

  ngOnInit() {
  }

  initialize(subAction: SubAction, editAction: EventEmitter<void>, deleteAction: EventEmitter<void>) {
    this.subAction = subAction as PropertyToggle;
    this.device$ = this.deviceService.getDevice(this.subAction.deviceId);
    this.edit = editAction;
    this.delete = deleteAction;
  }
}
