import {Component} from '@angular/core';
import {Observable, of} from 'rxjs';

import {CpuTemperatureCardComponent} from '../cards/cpu-temperature-card/cpu-temperature-card.component';
import {DeviceInfoCardComponent} from '../cards/device-info-card/device-info-card.component';

import {MemoryUsageCardComponent} from './../cards/memory-usage-card/memory-usage-card.component';
import {PropertyCurrentHistoryCardComponent} from './../cards/property-current-history-card/property-current-history-card.component';
import {DashboardCard} from './dashboard-card';

@Component({
  selector: 'app-dashboard',
  templateUrl: './dashboard.component.html',
  styleUrls: ['./dashboard.component.css']
})
export class DashboardComponent {
  cards: Observable<DashboardCard[]> = of([
    {maxCols: 1, maxRows: 1, component: DeviceInfoCardComponent},
  ]);
  cols = 1;

  constructor() {}
}
