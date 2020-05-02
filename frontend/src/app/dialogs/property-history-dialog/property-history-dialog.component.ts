import {Component, Inject, OnDestroy, OnInit} from '@angular/core';
import {MAT_DIALOG_DATA} from '@angular/material/dialog';
import {colorSets} from '@swimlane/ngx-charts';
import * as shape from 'd3-shape';

import {Subscription} from 'rxjs';

import {PropertyHistory} from '../../devices/device';
import {DeviceService} from '../../devices/device.service';

@Component({
  selector: 'app-property-history-dialog',
  templateUrl: './property-history-dialog.component.html',
  styleUrls: ['./property-history-dialog.component.scss']
})
export class PropertyHistoryDialogComponent implements OnInit, OnDestroy {
  chartData: {
    name: string,
    secondAxis: boolean,
    series: {name: Date, value: number}[],
    extra: any
  }[];
  hasSeries = false;
  colorScheme: any;
  propertyHasChart: {[key: string]: boolean} = {};
  curve: shape.CurveFactory = shape.curveStepAfter;  // default curveBasis

  private propertySub: Subscription;
  private subscriptionStr: {[deviceId: number]: string[]} = {};

  constructor(
      @Inject(MAT_DIALOG_DATA) public data: any,
      private deviceService: DeviceService) {}

  ngOnInit() {
    this.colorScheme = colorSets.find(s => s.name === 'picnic');

    for (const deviceId in this.data) {
      if (this.data.hasOwnProperty(deviceId)) {
        const properties = this.data[deviceId];
        this.subscriptionStr[deviceId] = Object.keys(properties);
        for (const propertyId in properties) {
          if (properties.hasOwnProperty(propertyId)) {
            this.propertyHasChart[propertyId] = properties[propertyId];
          }
        }
      }
    }

    // {<deviceid> : {property: boolean}}
    // {<deviceid> : [properties]}
    this.dayClick();
    this.propertySub =
        this.deviceService.getPropertyChanges().subscribe(propertyChange => {
          const series = this.chartData.find(
              s => parseInt(s.extra.deviceId, 10) === propertyChange.deviceId &&
                  s.name === propertyChange.key);
          if (series !== undefined) {
            this.hasSeries = true;
            series.series.push(
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

              if (series.length > 0) {
                this.hasSeries = true;
              }

              const chartSeries = {
                'name': propertyKey,
                'secondAxis': this.propertyHasChart[propertyKey],
                'series': series,
                'extra': {'deviceId': deviceId}
              };
              this.chartData.push(chartSeries);
            }
          }
        }
      }
    }

    // let tempLatest = 0;
    // data.forEach(entry => {
    //   const entryID = entry.id;
    //   if (this.latestEntryID < entryID) {
    //     if (entryID > tempLatest) {
    //       tempLatest = entryID;
    //     }
    //     // console.log(entry)
    //     const elem = {'name': new Date(entry.date), 'value': entry.value};

    //     this.chartData[0].series.push(elem);

    //     // if (this.chartData[0].series.length > 20) {
    //     //   this.chartData[0].series.shift();
    //     // }
    //   }
    // });
    // if (tempLatest > this.latestEntryID) {
    //   this.latestEntryID = tempLatest;
    //   if (this.chartData[0].series.length > 0) {
    //     this.chartData = [...this.chartData];
    //   } else {
    //     this.chartData = null;
    //   }
    // }
  }

  dayClick() {
    const time = new Date();
    time.setHours(time.getHours() - 24);
    this.chartData = null;
    const subscription =
        this.deviceService
            .getPropertyHistory(this.subscriptionStr, time, undefined, 60)
            .subscribe(history => {
              subscription.unsubscribe();
              this.updateGraph(history);
            });
  }
  weekClick() {
    const time = new Date();
    time.setDate(time.getDate() - 7);
    this.chartData = null;
    const subscription =
        this.deviceService
            .getPropertyHistory(this.subscriptionStr, time, undefined, 600)
            .subscribe(history => {
              subscription.unsubscribe();
              this.updateGraph(history);
            });
  }
  monthClick() {
    const time = new Date();
    time.setMonth(time.getMonth() - 1);
    this.chartData = null;
    const subscription =
        this.deviceService
            .getPropertyHistory(this.subscriptionStr, time, undefined, 1_800)
            .subscribe(history => {
              subscription.unsubscribe();
              this.updateGraph(history);
            });
  }
  yearClick() {
    const time = new Date();
    time.setMonth(time.getMonth() - 12);
    this.chartData = null;
    const subscription =
        this.deviceService
            .getPropertyHistory(this.subscriptionStr, time, undefined, 21_900)
            .subscribe(history => {
              subscription.unsubscribe();
              this.updateGraph(history);
            });
  }
}
