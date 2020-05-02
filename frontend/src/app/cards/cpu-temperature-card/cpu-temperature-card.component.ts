import { Component, OnInit, OnDestroy } from '@angular/core';
import { DatePipe } from '@angular/common';

import { Subscription, Observable } from 'rxjs';
import { StatisticsService } from '../../statistics/statistics.service';

import { NgxChartsModule } from '@swimlane/ngx-charts';
import { map } from 'rxjs/operators';

@Component({
  selector: 'app-cpu-temperature-card',
  templateUrl: './cpu-temperature-card.component.html',
  styleUrls: ['./cpu-temperature-card.component.css',
    '../../styles/card.css']
})
export class CpuTemperatureCardComponent implements OnInit, OnDestroy {
  colorGreenRed = {
    domain: ['#5AA454', '#A10A28']
  };

  data: any[];

  subscription: Subscription;

  constructor(private stat_service: StatisticsService,
    private datepipe: DatePipe) { }

  ngOnInit() {
    this.subscription = this.stat_service.getCPUTemperatureData().subscribe(data => this.updateGraph(data));
  }

  ngOnDestroy() {
    this.subscription.unsubscribe();
  }

  updateGraph(data: any) {
    const elem = {
      'name': this.datepipe.transform(data['time'] * 1000, 'HH:mm:ss'),
      'value': data['temperature']
    };
    if (!this.data) {
      this.data = [
        {
          'name': 'Temperatur',
          'series': []
        }
      ];
    }
    this.data[0]['series'].push(elem);
    if (this.data[0]['series'].length > 20) {
      this.data[0]['series'].shift();
    }
    this.data = [...this.data];
  }

  yFormat(data) {
    return data + '°C';
  }

}
