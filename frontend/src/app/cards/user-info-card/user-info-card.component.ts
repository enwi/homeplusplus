import { Component, OnInit } from '@angular/core';

import { NgxChartsModule } from '@swimlane/ngx-charts';

@Component({
  selector: 'app-user-info-card',
  templateUrl: './user-info-card.component.html',
  styleUrls: [ './user-info-card.component.css',
    '../../styles/card.css' ]
})
export class UserInfoCardComponent implements OnInit {

  colorVivid = {
    domain: ['#647c8a', '#5AA454']
  };

  data: any[] = [
    {
      'name': 'Benutzer',
      'value': 3
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
