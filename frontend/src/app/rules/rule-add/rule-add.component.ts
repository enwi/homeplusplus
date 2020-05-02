import { Component, OnInit, Inject } from '@angular/core';
import { Rule, RuleCondition } from '../rule';
import { RuleConstantCondition } from '../conditions/rule-conditions';
import { SubAction } from '../../actions/action';
import { FormGroup, FormBuilder, Validators } from '@angular/forms';
import { RuleService } from '../rule.service';
import { Router } from '@angular/router';
import { hexToColor } from '../../color-convert.module';
import { RULE_ICONS } from '../rule-icons';

@Component({
  selector: 'app-rule-add',
  templateUrl: './rule-add.component.html',
  styleUrls: ['./rule-add.component.css', '../rule-details/rule-details.component.css']
})
export class RuleAddComponent implements OnInit {

  subActions: SubAction[] = [];
  condition: RuleCondition = <RuleConstantCondition>{ id: 0, type: 0, state: false };
  header: FormGroup;

  constructor(private fb: FormBuilder,
    private ruleService: RuleService,
    private router: Router,
    @Inject(RULE_ICONS) public icons: [string, string][]) { }

  ngOnInit() {
    this.header = this.fb.group({
      name: ['', Validators.required],
      color: ['#FFFFFF'],
      icon: [''],
      enabled: [true]
    });
  }

  editCondition(c: RuleCondition) {
    this.condition = c;
  }

  deleteCondition() {
    this.condition = <RuleConstantCondition>{ id: 0, type: 0, state: false };
  }

  cancelRule() {
    this.router.navigate(['/rules'], { replaceUrl: true });
  }

  finishRule() {
    if (this.header.valid) {
      const { name, color, enabled } = this.header.value;
      const rule: Rule = {
        id: 0,
        name: name,
        color: hexToColor(color),
        icon: 'default',
        enabled: enabled,
        condition: this.condition,
        effect: {
          id: 0,
          name: name + '_Action',
          color: hexToColor(color),
          icon: 'default',
          subActions: this.subActions,
          visible: false
        }
      };
      this.ruleService.addRule(rule);
      // Replace url so that navigating back does not accidentally create another rule
      this.router.navigate(['/rules'], { replaceUrl: true });
    }
  }

}
