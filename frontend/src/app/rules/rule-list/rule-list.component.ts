import { Component, OnInit } from '@angular/core';
import { Observable } from 'rxjs';
import { Rule } from '../rule';
import { RuleService } from '../rule.service';

@Component({
  selector: 'app-rule-list',
  templateUrl: './rule-list.component.html',
  styleUrls: ['./rule-list.component.scss']
})
export class RuleListComponent implements OnInit {

  rules$: Observable<Rule[]>;

  constructor(private ruleService: RuleService) { }

  ngOnInit() {
    this.getRules();
  }

  getRules(): void {
    this.rules$ = this.ruleService.getRulesAsArray();
  }

}
