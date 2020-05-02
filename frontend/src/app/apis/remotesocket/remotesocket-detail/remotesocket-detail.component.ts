import {COMMA, ENTER} from '@angular/cdk/keycodes';
import {Component, OnInit} from '@angular/core';
import {FormBuilder, FormControl, FormGroup, Validators} from '@angular/forms';
import {MatChipInputEvent} from '@angular/material/chips';
import {MatDialog} from '@angular/material/dialog';

import {DeleteDialog} from '../../../delete-dialog/delete-dialog.component';
import {Device, PropertyChange} from '../../../devices/device';
import {DeviceDetail} from '../../../devices/device-detail/device-detail';
import {DeviceService} from '../../../devices/device.service';
import {RemoteSocketService} from '../remotesocket.service';

@Component({
  selector: 'app-remotesocket-detail',
  templateUrl: './remotesocket-detail.component.html',
  styleUrls: ['./remotesocket-detail.component.css']
})
export class RemoteSocketDetailComponent implements OnInit, DeviceDetail {
  remoteSocket: Device;
  toggle: FormControl;
  editForm: FormGroup;
  editEnabled = false;
  editGroups: string[];
  readonly separatorKeyCodes: number[] = [COMMA, ENTER];

  constructor(
      private deviceService: DeviceService,
      private remoteSocketService: RemoteSocketService, private fb: FormBuilder,
      private dialog: MatDialog) {}

  ngOnInit() {}

  setDevice(d: Device) {
    this.remoteSocket = d;
    this.buildForm();
  }

  onPropertyChange(propertyChange: PropertyChange): void {
    this.remoteSocket.properties.set(propertyChange.key, propertyChange.value);
  }

  buildForm() {
    this.toggle = this.fb.control(this.remoteSocket.properties.get('state'));
    this.toggle.valueChanges.subscribe((value) => this.onValueChange(value));
  }
  onValueChange(value: boolean) {
    this.deviceService.setDeviceProperty(this.remoteSocket.id, 'state', value);
  }

  delete() {
    const dialogRef = this.dialog.open(
        DeleteDialog, {data: {content: 'Dieses Gerät löschen?'}});
    dialogRef.afterClosed().subscribe(result => {
      if (result) {
        this.remoteSocketService.deleteRemoteSocket(this.remoteSocket.id);
      }
    });
  }
  edit() {
    this.editEnabled = true;
    this.editGroups = this.remoteSocket.groups.slice();
    this.editForm = this.fb.group({
      name: [this.remoteSocket.name, Validators.required],
      groups: [this.remoteSocket.groups],
      code: [
        this.remoteSocket.properties.get('code'),
        Validators.pattern('[01]{5}[1-4]')
      ]
    });
  }
  addGroup(event: MatChipInputEvent) {
    const input = event.input;
    const value = event.value;
    const trimmed = value.trim();
    if (trimmed.length != 0 && !this.editGroups.includes(trimmed)) {
      this.editGroups.push(trimmed);
    }

    // Reset input
    if (input) {
      input.value = '';
    }
  }
  removeGroup(group: string) {
    const index = this.editGroups.indexOf(group);

    if (index >= 0) {
      this.editGroups.splice(index, 1);
    }
  }

  finishEdit() {
    this.editEnabled = false;
    if (this.editForm.valid) {
      const {name, location, code} = this.editForm.value;
      if (name != this.remoteSocket.name) {
        this.remoteSocket.name = name;
        this.deviceService.setDeviceName(this.remoteSocket.id, name);
      }
      if (code != this.remoteSocket.properties.get('code')) {
        this.remoteSocket.properties.set('code', code);
        this.deviceService.setDeviceProperty(
            this.remoteSocket.id, 'code', code);
      }
      let changed = false;
      if (this.editGroups.length != this.remoteSocket.groups.length) {
        changed = true;
      } else {
        for (let i = 0; i < this.editGroups.length; ++i) {
          if (this.editGroups[i] != this.remoteSocket.groups[i]) {
            changed = true;
            break;
          }
        }
      }
      if (changed) {
        this.remoteSocket.groups = this.editGroups;
        this.deviceService.setDeviceGroups(
            this.remoteSocket.id, this.remoteSocket.groups);
      }
    }
  }
  cancelEdit() {
    this.editEnabled = false;
  }
}
