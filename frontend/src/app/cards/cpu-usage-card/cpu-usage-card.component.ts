import { Component, OnInit, OnDestroy } from '@angular/core';

import { Observable } from 'rxjs';
import { StatisticsService } from '../../statistics/statistics.service';

import { NgxChartsModule } from '@swimlane/ngx-charts';
import { map } from 'rxjs/operators';

@Component({
  selector: 'app-cpu-usage-card',
  templateUrl: './cpu-usage-card.component.html',
  styleUrls: ['./cpu-usage-card.component.css',
    '../../styles/card.css']
})
export class CpuUsageCardComponent implements OnInit {
  colorGreenRed = {
    domain: ['#5AA454', '#A10A28']
  };

  data$: Observable<any[]>;

  constructor(private stat_service: StatisticsService) { }

  ngOnInit() {
    this.data$ = this.stat_service.getCPUUsage().pipe(map(data => [{ 'name': 'Auslastung', 'value': data * 100 }]));
  }

  yFormat(data) {
    return data + '%';
  }

}
