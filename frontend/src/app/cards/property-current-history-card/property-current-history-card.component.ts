import {Component, Input, OnDestroy, OnInit} from '@angular/core';
import {colorSets} from '@swimlane/ngx-charts';
import * as shape from 'd3-shape';
import {Subscription} from 'rxjs';
import {PropertyHistory} from '../../devices/device';

import {DeviceService} from '../../devices/device.service';

@Component({
  selector: 'app-property-current-history-card',
  templateUrl: './property-current-history-card.component.html',
  styleUrls:
      ['./property-current-history-card.component.css', '../../styles/card.css']
})
export class PropertyCurrentHistoryCardComponent implements OnInit, OnDestroy {
  @Input() deviceId: number;
  @Input() propertyId: string;
  @Input() hours: number;

  deviceName: string;
  propertyName: string;

  chartData: {name: string, series: {name: Date, value: number}[],
              extra?: any}[];
  colorScheme: any;
  curve: shape.CurveFactory = shape.curveStepAfter;

  private propertySub: Subscription;

  constructor(private deviceService: DeviceService) {}

  ngOnInit(): void {
    this.colorScheme = colorSets.find(s => s.name === 'picnic');

    // TODO For now only use propertyId as name
    this.propertyName = this.propertyId;

    const subscriptionStr: {[deviceId: number]: string[]} = {};
    subscriptionStr[this.deviceId] = [this.propertyId];

    const deviceSub =
        this.deviceService.getDevice(this.deviceId).subscribe(device => {
          deviceSub.unsubscribe();
          this.deviceName = device.name;
        });

    const time = new Date();
    // time.setHours(time.getHours() - 24);
    time.setHours(time.getHours() - this.hours);
    const subscription =
        this.deviceService.getPropertyHistory(subscriptionStr, time)
            .subscribe(history => {
              subscription.unsubscribe();
              this.updateGraph(history);
            });

    this.propertySub =
        this.deviceService.getPropertyChanges().subscribe(propertyChange => {
          if (propertyChange.deviceId === this.deviceId &&
              propertyChange.key === this.propertyId) {
            this.chartData[0].series.push(
                {'name': new Date(), 'value': propertyChange.value});
            // TODO remove oldest element
            // Force chart update
            this.chartData = [...this.chartData];
          }
        });
  }

  ngOnDestroy() {
    if (this.propertySub) {
      this.propertySub.unsubscribe();
    }
  }

  updateGraph(history: PropertyHistory) {
    if (!this.chartData) {
      this.chartData = [];
      for (const deviceId in history.history) {
        if (history.history.hasOwnProperty(deviceId)) {
          const properties = history.history[deviceId];

          for (const propertyKey in properties) {
            if (properties.hasOwnProperty(propertyKey)) {
              const property = properties[propertyKey];

              const series: {name: Date, value: number}[] = [];
              for (const time in property) {
                if (property.hasOwnProperty(time)) {
                  const element = property[time];
                  series.push({'name': new Date(time), 'value': element});
                }
              }

              const chartSeries = {'name': propertyKey, 'series': series};
              this.chartData.push(chartSeries);
            }
          }
        }
      }
    }
  }
}
