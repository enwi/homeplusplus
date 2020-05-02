import {LayoutModule} from '@angular/cdk/layout';
import {DatePipe} from '@angular/common';
import {HttpClientModule} from '@angular/common/http';
import {NgModule} from '@angular/core';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatCardModule} from '@angular/material/card';
import {MatCheckboxModule} from '@angular/material/checkbox';
import {MatDialogModule} from '@angular/material/dialog';
import {MatGridListModule} from '@angular/material/grid-list';
import {MatIconModule} from '@angular/material/icon';
import {MatInputModule} from '@angular/material/input';
import {MatListModule} from '@angular/material/list';
import {MatMenuModule} from '@angular/material/menu';
import {MatProgressSpinnerModule} from '@angular/material/progress-spinner';
import {MatSidenavModule} from '@angular/material/sidenav';
import {MatStepperModule} from '@angular/material/stepper';
import {MatToolbarModule} from '@angular/material/toolbar';
import {MatTooltipModule} from '@angular/material/tooltip';
import {BrowserAnimationsModule} from '@angular/platform-browser/animations';
import {NgxChartsModule} from '@swimlane/ngx-charts';

import {ActionsModule} from './actions/actions.module';
import {HueModule} from './apis/hue/hue.module';
import {RemoteSocketModule} from './apis/remotesocket/remotesocket.module';
import {TasmotaModule} from './apis/tasmota/tasmota.module';
import {AppRoutingModule} from './app-routing.module';
import {AppComponent} from './app.component';
import {AuthModule} from './auth/auth.module';
import {CpuTemperatureCardComponent} from './cards/cpu-temperature-card/cpu-temperature-card.component';
import {CpuUsageCardComponent} from './cards/cpu-usage-card/cpu-usage-card.component';
import {DeviceInfoCardComponent} from './cards/device-info-card/device-info-card.component';
import {MemoryUsageCardComponent} from './cards/memory-usage-card/memory-usage-card.component';
import {PropertyCurrentHistoryCardComponent} from './cards/property-current-history-card/property-current-history-card.component';
import {TextCardComponent} from './cards/text-card/text-card.component';
import {UserInfoCardComponent} from './cards/user-info-card/user-info-card.component';
import {DashboardCardComponent} from './dashboard/dashboard-card/dashboard-card.component';
import {DashboardComponent} from './dashboard/dashboard.component';
import {DevicesModule} from './devices/devices.module';
import {ImgCropperDialogComponent} from './dialogs/img-cropper-dialog/img-cropper-dialog.component';
import {LoadingDialogComponent} from './dialogs/loading-dialog/loading-dialog.component';
import {MatGridListResponsiveModule} from './lib/mat-grid-list-responsive/mat-grid-list-responsive.module';
import {NavUserComponent} from './navigation/nav-user/nav-user.component';
import {NavigationComponent} from './navigation/navigation.component';
import {NotificationsModule} from './notifications/notifications.module';
import {RulesModule} from './rules/rules.module';
import {StatisticsComponent} from './statistics/statistics.component';
import {UserComponent} from './user/user.component';
import {WebsocketCleanup} from './websocket/websocket-cleanup';

@NgModule({
  imports: [
    AppRoutingModule,  BrowserAnimationsModule,
    FormsModule,       ReactiveFormsModule,
    MatButtonModule,   MatCheckboxModule,
    LayoutModule,      MatToolbarModule,
    MatSidenavModule,  MatIconModule,
    MatListModule,     HttpClientModule,
    MatGridListModule, MatCardModule,
    MatMenuModule,     MatInputModule,
    MatStepperModule,  MatProgressSpinnerModule,
    MatTooltipModule,  MatDialogModule,
    DevicesModule,     ActionsModule,
    HueModule,         RemoteSocketModule,
    RulesModule,       NotificationsModule,
    AuthModule,        TasmotaModule,
    NgxChartsModule,   MatGridListResponsiveModule,
  ],
  declarations: [
    AppComponent, NavigationComponent, NavUserComponent, DashboardComponent,
    UserComponent, StatisticsComponent, TextCardComponent,
    CpuUsageCardComponent, MemoryUsageCardComponent,
    CpuTemperatureCardComponent, UserInfoCardComponent, DeviceInfoCardComponent,
    DashboardCardComponent, ImgCropperDialogComponent, LoadingDialogComponent,
    WebsocketCleanup, PropertyCurrentHistoryCardComponent
  ],
  exports: [MatButtonModule, MatCheckboxModule],
  providers: [DatePipe],
  bootstrap: [AppComponent]
})
export class AppModule {
}
