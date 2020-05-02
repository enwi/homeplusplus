import {CommonModule} from '@angular/common';
import {NgModule} from '@angular/core';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatCardModule} from '@angular/material/card';
import {MatRippleModule} from '@angular/material/core';
import {MatDialogModule} from '@angular/material/dialog';
import {MatDividerModule} from '@angular/material/divider';
import {MatExpansionModule} from '@angular/material/expansion';
import {MatFormFieldModule} from '@angular/material/form-field';
import {MatGridListModule} from '@angular/material/grid-list';
import {MatIconModule} from '@angular/material/icon';
import {MatInputModule} from '@angular/material/input';
import {MatSelectModule} from '@angular/material/select';

import {RemoteSocketModule} from '../apis/remotesocket/remotesocket.module';
import {DeleteDialogModule} from '../delete-dialog/delete-dialog.module';
import {FilterListModule} from '../filter-list/filter-list.module';

import {AddDeviceComponent} from './add-device/add-device.component';
import {DeviceDetailComponent} from './device-detail/device-detail.component';
import {DeviceDetailDirective} from './device-detail/device-detail.directive';
import {DeviceListComponent} from './device-list/device-list.component';
import {DeviceRoutingModule} from './device-routing.module';
import {DeviceService} from './device.service';

@NgModule({
  imports: [
    CommonModule, FormsModule, MatButtonModule, MatCardModule,
    MatGridListModule, MatFormFieldModule, MatInputModule, MatIconModule,
    MatCardModule, MatExpansionModule, MatDividerModule, MatRippleModule,
    MatDialogModule, MatSelectModule, ReactiveFormsModule, DeleteDialogModule,
    FilterListModule, DeviceRoutingModule, RemoteSocketModule
  ],
  exports: [RemoteSocketModule],
  declarations: [
    DeviceDetailComponent, DeviceListComponent, DeviceDetailDirective,
    AddDeviceComponent
  ]
})
export class DevicesModule {
}
