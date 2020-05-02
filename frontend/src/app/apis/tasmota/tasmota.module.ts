import {CommonModule} from '@angular/common';
import {NgModule, Type} from '@angular/core';
import {ReactiveFormsModule} from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatCardModule} from '@angular/material/card';
import {MatGridListModule} from '@angular/material/grid-list';
import {MatIconModule} from '@angular/material/icon';
import {MatInputModule} from '@angular/material/input';
import {MatProgressSpinnerModule} from '@angular/material/progress-spinner';
import {MatSlideToggleModule} from '@angular/material/slide-toggle';
import {MatSliderModule} from '@angular/material/slider';
import {NgxChartsModule} from '@swimlane/ngx-charts';

import {DoubleAxisChartComponent} from '../../chart/double-axis-chart/double-axis-chart.component';
import {ColorConvertModule} from '../../color-convert.module';
import {DeviceDetail} from '../../devices/device-detail/device-detail';
import {DeviceParser} from '../../devices/device-parser';
import {DEVICE_DETAILS, DEVICE_PARSERS} from '../../devices/device.config';
import {PropertyHistoryDialogComponent} from '../../dialogs/property-history-dialog/property-history-dialog.component';

import {PropertiesSlideToggleCardComponent} from './../../cards/properties-slide-toggle-card/properties-slide-toggle-card.component';
import {TasmotaDetailComponent} from './tasmota-detail/tasmota-detail.component';
import {TasmotaParseService} from './tasmota-parse.service';

@NgModule({
  imports: [
    CommonModule, ReactiveFormsModule, MatCardModule, MatButtonModule,
    ColorConvertModule, MatIconModule, MatSliderModule, MatInputModule,
    MatGridListModule, MatSlideToggleModule, NgxChartsModule,
    MatProgressSpinnerModule
  ],
  declarations: [
    TasmotaDetailComponent, PropertiesSlideToggleCardComponent,
    PropertyHistoryDialogComponent, DoubleAxisChartComponent
  ],
  providers: [
    {
      provide: DEVICE_PARSERS,
      deps: [TasmotaParseService],
      multi: true,
      useFactory: (tasmotaParser: TasmotaParseService):
          [string, DeviceParser] => ['tasmota', tasmotaParser]
    },
    {
      provide: DEVICE_DETAILS,
      multi: true,
      useFactory: ():
          [string, Type<DeviceDetail>] => ['tasmota', TasmotaDetailComponent]
    }
  ]
})
export class TasmotaModule {
}
