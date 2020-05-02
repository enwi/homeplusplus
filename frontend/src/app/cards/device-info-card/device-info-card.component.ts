import { Component, OnInit } from '@angular/core';

import { NgxChartsModule } from '@swimlane/ngx-charts';

@Component({
  selector: 'app-device-info-card',
  templateUrl: './device-info-card.component.html',
  styleUrls: [ './device-info-card.component.css',
    '../../styles/card.css' ]
})
export class DeviceInfoCardComponent implements OnInit {

  colorVivid = {
    domain: ['#647c8a', '#5AA454']
  };

  data: any[] = [
    {
      'name': 'Geräte',
      'value': 4
    },
    {
      'name': 'Aktiv',
      'value': 1
    }
  ];

  constructor() { }

  ngOnInit() {
  }

}
