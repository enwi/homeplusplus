import {CommonModule} from '@angular/common';
import {NgModule, Type} from '@angular/core';
import {ReactiveFormsModule} from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatCardModule} from '@angular/material/card';
import {MatChipsModule} from '@angular/material/chips';
import {MatDialogModule} from '@angular/material/dialog';
import {MatDividerModule} from '@angular/material/divider';
import {MatFormFieldModule} from '@angular/material/form-field';
import {MatIconModule} from '@angular/material/icon';
import {MatInputModule} from '@angular/material/input';
import {MatSlideToggleModule} from '@angular/material/slide-toggle';

import {DeleteDialogModule} from '../../delete-dialog/delete-dialog.module';
import {DeviceDetail} from '../../devices/device-detail/device-detail';
import {DeviceParser} from '../../devices/device-parser';
import {DEVICE_DETAILS, DEVICE_PARSERS} from '../../devices/device.config';

import {AddRemotesocketComponent} from './add-remotesocket/add-remotesocket.component';
import {RemoteSocketDetailComponent} from './remotesocket-detail/remotesocket-detail.component';
import {RemoteSocketParseService} from './remotesocket-parse.service';

@NgModule({
  imports: [
    CommonModule, ReactiveFormsModule, MatSlideToggleModule, MatCardModule,
    MatIconModule, MatDialogModule, MatDividerModule, MatFormFieldModule,
    MatInputModule, MatButtonModule, MatChipsModule, DeleteDialogModule
  ],
  declarations: [RemoteSocketDetailComponent, AddRemotesocketComponent],
  providers: [
    {
      provide: DEVICE_PARSERS,
      deps: [RemoteSocketParseService],
      multi: true,
      useFactory: (parser: RemoteSocketParseService):
          [string, DeviceParser] => ['remoteSocket', parser]
    },
    {
      provide: DEVICE_DETAILS,
      multi: true,
      useFactory: ():
          [string,
           Type<DeviceDetail>] => ['remoteSocket', RemoteSocketDetailComponent]
    }
  ]
})
export class RemoteSocketModule {
}
