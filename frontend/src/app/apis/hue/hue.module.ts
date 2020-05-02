import {CommonModule} from '@angular/common';
import {NgModule, Type} from '@angular/core';
import {ReactiveFormsModule} from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatCardModule} from '@angular/material/card';
import {MatIconModule} from '@angular/material/icon';
import {MatInputModule} from '@angular/material/input';
import {MatSlideToggleModule} from '@angular/material/slide-toggle';
import {MatSliderModule} from '@angular/material/slider';

import {ColorConvertModule} from '../../color-convert.module';
import {DeviceDetail} from '../../devices/device-detail/device-detail';
import {DeviceParser} from '../../devices/device-parser';
import {DEVICE_DETAILS, DEVICE_PARSERS} from '../../devices/device.config';

import {HueDetailComponent} from './hue-detail/hue-detail.component';
import {HueParseService} from './hue-parse.service';

@NgModule({
  imports: [
    CommonModule, ReactiveFormsModule, MatCardModule, MatButtonModule,
    ColorConvertModule, MatIconModule, MatSliderModule, MatInputModule,
    MatSlideToggleModule
  ],
  declarations: [HueDetailComponent],
  providers: [
    {
      provide: DEVICE_PARSERS,
      deps: [HueParseService],
      multi: true,
      useFactory: (hueParser: HueParseService):
          [string, DeviceParser] => ['hueLight', hueParser]
    },
    {
      provide: DEVICE_DETAILS,
      multi: true,
      useFactory: ():
          [string, Type<DeviceDetail>] => ['hueLight', HueDetailComponent]
    }
  ]
})
export class HueModule {
}
