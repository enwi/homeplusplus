import { Component, OnInit, OnDestroy } from '@angular/core';

import { Observable } from 'rxjs';
import { StatisticsService } from '../../statistics/statistics.service';

import { NgxChartsModule } from '@swimlane/ngx-charts';
import { map } from 'rxjs/operators';

@Component({
  selector: 'app-memory-usage-card',
  templateUrl: './memory-usage-card.component.html',
  styleUrls: ['./memory-usage-card.component.css',
    '../../styles/card.css']
})

export class MemoryUsageCardComponent implements OnInit {
  colorGreenRed = {
    domain: ['#5AA454', '#A10A28']
  };

  data$: Observable<any[]>;

  constructor(private stat_service: StatisticsService) { }

  ngOnInit() {
    this.data$ = this.stat_service.getMemoryData().pipe(map(this.mapData));
  }


  mapData(data: any): any[] {
    return [
      {
        'name': 'Frei',
        'value': (1 - (data['total'] - data['available']) / data['total']) * 100
      },
      {
        'name': 'Genutzt',
        'value': ((data['total'] - data['available']) / data['total']) * 100
      }
    ];
  }

}
