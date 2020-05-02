import { Component, OnInit, EventEmitter } from '@angular/core';
import { ConditionView } from './condition-view';
import { RuleCompareCondition } from './rule-conditions';
import { RuleCondition } from '../rule';

@Component({
  selector: 'app-compare-condition',
  templateUrl: './compare-condition.component.html',
  styleUrls: ['./compare-condition.component.css']
})
export class CompareConditionComponent implements OnInit, ConditionView {
  condition: RuleCompareCondition;

  constructor() { }

  ngOnInit() {
  }

  initialize(c: RuleCondition) {
    this.condition = c as RuleCompareCondition;
  }

  updateCondition(c: RuleCondition) {
    this.condition = c as RuleCompareCondition;
  }

  getCompareString(): string {
    switch (this.condition.compare) {
      case 0:
        // and
        return 'und';
      case 1:
        // or
        return 'oder';
      case 2:
        // nand
        return 'nicht und';
      case 3:
        // nor
        return 'nicht oder';
      case 4:
        // equal = xnor
        return 'ist gleich';
      case 5:
        // not equal = xor
        return 'ist nicht gleich';
    }
  }
}
