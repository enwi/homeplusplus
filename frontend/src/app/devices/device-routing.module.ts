import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';

// routing components
import { DeviceListComponent } from './device-list/device-list.component';
import { DeviceDetailComponent } from './device-detail/device-detail.component';
import { AuthGuard } from '../auth/auth-guard.service';
import { AddDeviceComponent } from './add-device/add-device.component';
import { NavigationComponent } from '../navigation/navigation.component';
import { AddRemotesocketComponent } from '../apis/remotesocket/add-remotesocket/add-remotesocket.component';

const routes: Routes = [
  {
    path: '',
    component: NavigationComponent,
    canActivate: [AuthGuard],
    canActivateChild: [AuthGuard],
    children: [
      { path: 'devices', component: DeviceListComponent },
      {
        path: 'device',
        children: [
          {
            path: 'add',
            children: [
              { path: '', component: AddDeviceComponent },
              { path: 'remoteSocket', component: AddRemotesocketComponent }
            ]
          },
          { path: ':id', component: DeviceDetailComponent }
        ]
      },
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DeviceRoutingModule { }
