import { Component, OnInit, EventEmitter } from '@angular/core';
import { ConditionView } from './condition-view';
import { RuleCondition } from '../rule';
import { RuleConstantCondition } from './rule-conditions';

@Component({
  selector: 'app-constant-condition',
  templateUrl: './constant-condition.component.html',
  styleUrls: ['./constant-condition.component.css']
})
export class ConstantConditionComponent implements OnInit, ConditionView {
  condition: RuleConstantCondition;

  constructor() { }

  ngOnInit() {
  }

  initialize(c: RuleCondition) {
    this.condition = c as RuleConstantCondition;
  }

  updateCondition(c: RuleCondition) {
    this.condition = c as RuleConstantCondition;
  }
}
