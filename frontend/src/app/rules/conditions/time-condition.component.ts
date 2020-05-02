import { Component, OnInit, EventEmitter, Inject, LOCALE_ID } from '@angular/core';
import { ConditionView } from './condition-view';
import { RuleTimeCondition } from './rule-conditions';
import { RuleCondition } from '../rule';
import { getLocaleDayNames, FormStyle, TranslationWidth, getLocaleMonthNames, formatDate } from '@angular/common';

@Component({
  selector: 'app-time-condition',
  templateUrl: './time-condition.component.html',
  styleUrls: ['./time-condition.component.css']
})
export class TimeConditionComponent implements OnInit, ConditionView {
  condition: RuleTimeCondition;

  constructor(@Inject(LOCALE_ID) private locale) { }

  ngOnInit() {
  }

  initialize(c: RuleCondition) {
    this.condition = c as RuleTimeCondition;
  }
  updateCondition(c: RuleCondition) {
    this.condition = c as RuleTimeCondition;
  }

  timeToString(time: number): string {
    switch (this.condition.timeType) {
      case 0:
        // hour min sec
        const date = new Date(0, 0, 0, Math.floor(time / 3600),
          Math.floor((time % 3600) / 60), time % 60);
        return date.toLocaleTimeString();
      case 1:
        // hour
        return `${time} Uhr`;
      case 2:
        // dayWeek
        const dayNames = getLocaleDayNames(this.locale, FormStyle.Standalone, TranslationWidth.Wide);
        return dayNames[time];
      case 3:
        // dayMonth
        return `${time}. jeden Monat`;
      case 4:
        // dayYear
        return `${time}. Tag im Jahr`;
      case 5:
        // month
        const monthNames = getLocaleMonthNames(this.locale, FormStyle.Standalone, TranslationWidth.Wide);
        return monthNames[time];
      case 6:
        // year
        return `Jahr ${time}`;
      case 7:
        // absolute
        return formatDate(time, 'medium', this.locale);
    }
  }

  getTimeString(): string {
    // let time1Str: string;
    // let time2Str: string;

    switch (this.condition.compare) {
      case 0:
        // equals
        if (this.condition.time2 === 0) {
          return `Es ist ${this.timeToString(this.condition.time1)}`;
        } else {
          return `Es ist ${this.timeToString(this.condition.time1)} +/- ${this.condition.time2}`;
        }
      case 1:
        // not equals
        if (this.condition.time2 === 0) {
          return `Es ist nicht ${this.timeToString(this.condition.time1)}`;
        } else {
          return `Es ist nicht ${this.timeToString(this.condition.time1)} +/- ${this.condition.time2}`;
        }
      case 2:
        // greater
        return `Es ist nach ${this.timeToString(this.condition.time1)}`;
      case 3:
        // less
        return `Es ist vor ${this.timeToString(this.condition.time1)}`;
      case 4:
        // in range
        return `Es ist zwischen ${this.timeToString(this.condition.time1)} und ${this.timeToString(this.condition.time2)}`;
    }
    return 'unbekannt';
  }
}
