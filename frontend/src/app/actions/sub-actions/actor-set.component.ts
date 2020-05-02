import { Component, OnInit, EventEmitter } from '@angular/core';
import { PropertySet } from './sub-actions';
import { Observable } from 'rxjs';
import { DeviceService } from '../../devices/device.service';
import { Device } from '../../devices/device';
import { SubActionView } from './sub-action-view';
import { SubAction } from '../action';
import { scan, filter } from 'rxjs/operators';

@Component({
  selector: 'app-actor-set',
  templateUrl: './actor-set.component.html',
  styleUrls: ['./actor-set.component.css', './sub-action.css']
})
export class ActorSetComponent implements OnInit, SubActionView {

  subAction: PropertySet;
  device$: Observable<Device>;
  edit: EventEmitter<void>;
  delete: EventEmitter<void>;

  constructor(private deviceService: DeviceService) { }

  ngOnInit() {
  }

  initialize(subAction: SubAction, editAction: EventEmitter<void>, deleteAction: EventEmitter<void>) {
    this.subAction = subAction as PropertySet;
    this.device$ = this.deviceService.getDevice(this.subAction.deviceId);
    this.edit = editAction;
    this.delete = deleteAction;
  }
}
